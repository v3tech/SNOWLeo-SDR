/* -*- c++ -*- */
/* 
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "sink_sync_impl.h"

typedef struct sockaddr SA;

#define TX_SAMPLES_NUM  (1024*8)
#define MIN_NUM_SAMPLES 1
#define MAX_NUM_SAMPLES 8196

namespace gr {
    namespace zingsdr {

        sink_sync::sptr
            sink_sync::make(const std::string &ipaddr, uint32_t port, 
                    uint32_t tx_freq, uint32_t tx_vga, uint32_t sample_rate, uint32_t dci, uint32_t dcq)
            {
                return gnuradio::get_initial_sptr
                    (new sink_sync_impl(ipaddr, port, tx_freq, tx_vga, sample_rate, dci, dcq));
            }

        /*
         * The private constructor
         */
        sink_sync_impl::sink_sync_impl(const std::string &ipaddr, uint32_t port, 
                uint32_t tx_freq, uint32_t tx_vga, uint32_t sample_rate, uint32_t dci, uint32_t dcq)
            : gr::sync_block("sink_sync",
                    gr::io_signature::make(1, 1, sizeof(gr_complex)),
                    gr::io_signature::make(0, 0, sizeof(gr_complex)))
        {
            _buf_used = 0;
            d_ipaddr = ipaddr;
            d_port = port;
            d_tx_vga = tx_vga;
            d_tx_freq = tx_freq;
            d_sample_rate = sample_rate;

            if(this->connect_server() < 0)
                throw std::runtime_error("zingsdr:connect server error!\n");
            this->set_dc_offset(dci, dcq);
            this->set_freq(d_tx_freq);
            this->set_gain(d_tx_vga);
            this->set_sample_rate(d_sample_rate);
            this->handshake();

            p = (struct ring_buffer *)malloc(sizeof(struct ring_buffer));
            p->front = p->rear = 0;
            int i = 0;
            for(i = 0; i< RING_BUFFER_MAXSIZE; i++)
                p->data[i] = (uint8_t *)malloc(TX_SAMPLES_NUM);

            _running = true;
            if(pthread_create(&thread_send, NULL, creat_send_thread, this) < 0)
                throw std::runtime_error("zingsdr:creat recv thread error!\n");

        }

        /*
         * Our virtual destructor.
         */
        sink_sync_impl::~sink_sync_impl()
        {
            int i = 0;

            _running = false;
            pthread_join(thread_send, NULL);

            for(i = 0; i< RING_BUFFER_MAXSIZE; i++)
                free(p->data[i]);
            free(p);

            //pthread_cancel(thread_send);

            if(this->disconnect_server() < 0)
                throw std::runtime_error("zingsdr:disconnect server error!\n");
        }

        int
            sink_sync_impl::work(int noutput_items,
                    gr_vector_const_void_star &input_items,
                    gr_vector_void_star &output_items)
            {
                const gr_complex *in = (const gr_complex *) input_items[0];
                gr_complex *out = (gr_complex *) output_items[0];

                // Do <+signal processing+>
                const float scaling = 2000.0f;
                //const float scaling = 1.0f;
                int i;
                char *buf = _buf + _buf_used;
                unsigned int prev_buf_used = _buf_used;

                unsigned int remaining = (TX_SAMPLES_NUM-_buf_used)/4; //complex

                unsigned int count = std::min((unsigned int)noutput_items,remaining);

                _buf_used += count * 4;
                for(i = 0; i < count*4; i+=4)
                {
                    (*(short *)(buf + i)) = (short)(scaling * in[i/4].real());
                    (*(short *)(buf + i + 2)) = (short)(scaling * in[i/4].imag());
                }

                int items_consumed = count;

                if((unsigned int)noutput_items >= remaining) {
                    {
                        if((p->rear + 1) % RING_BUFFER_MAXSIZE == p->front) {
                            _buf_used = prev_buf_used;
                            items_consumed = 0;
                            //std::cerr << "O" << std::flush;
                        }
                        else {
                            memcpy(p->data[p->rear], _buf, TX_SAMPLES_NUM);
                            p->rear = (p->rear + 1) % RING_BUFFER_MAXSIZE;
                            _buf_used = 0;
                        }
                    }
                }

                // Tell runtime system how many input items we consumed on
                // each input stream.
                consume_each(items_consumed);

                // Tell runtime system how many output items we produced.
                return 0;
                /*************************************/
            }
        void sink_sync_impl::set_dc_offset(uint32_t dci, uint32_t dcq)
        {
            uint32_t cmd_buf[2]={0,0};
            printf("dci = %d, dcq = %d\n", dci, dcq);
            //cmd_buf[0] = 0xF0210000;
            cmd_buf[0] = 0xF0210000|((dci&0xFF)<<8)|(dcq&0xFF);
            sendto(sock_cmd, cmd_buf, sizeof(cmd_buf), 0, (struct sockaddr *)&cmd_addr, sizeof(cmd_addr));

        }

        void sink_sync_impl::set_freq(uint32_t freq)
        {
            uint32_t cmd_buf[2]={0,0};
            cmd_buf[0] = 0xF0170000;
            cmd_buf[1] = freq; /*set tx freq*/
            sendto(sock_cmd, cmd_buf, sizeof(cmd_buf), 0, (struct sockaddr *)&cmd_addr, sizeof(cmd_addr));

        }
        void sink_sync_impl::set_gain(uint32_t gain)
        { 
            uint32_t cmd_buf[2]={0,0};
            gain = gain & 0x000000FF;
            cmd_buf[0] = 0xF0190000 | (gain); //set tx vga
            cmd_buf[1] = 0x0b000000;
            sendto(sock_cmd, cmd_buf, sizeof(cmd_buf), 0, (struct sockaddr *)&cmd_addr, sizeof(cmd_addr));
        }

        void sink_sync_impl::set_sample_rate(uint32_t sample_rate)
        { 
            uint32_t cmd_buf[2]={0,0};
            cmd_buf[0] = 0xF0240000;
            cmd_buf[1] = sample_rate;
            sendto(sock_cmd, cmd_buf, sizeof(cmd_buf), 0, (struct sockaddr *)&cmd_addr, sizeof(cmd_addr));
        }

        int sink_sync_impl::send_sample()
        {
            unsigned int cmd_buf[2]={0,0};
            int len = 0, i = 0, nbyte = 0;

            while(_running) {
                if(p->front == p->rear) {
                    //std::cerr << "U" << std::flush;
                    sleep(0);
                }
                else {
                    pthread_testcancel();
                    if(send(sock_sink, p->data[p->front], TX_SAMPLES_NUM, 0) < 0)
                        break;
                    pthread_testcancel();
                    p->front = (p->front + 1) % RING_BUFFER_MAXSIZE;
                }
            }
        }
        void *sink_sync_impl::creat_send_thread(void *param)
        {
            sink_sync_impl *pThis = (sink_sync_impl *)param;
            pThis->send_sample();
            return NULL;
        }

        int sink_sync_impl::connect_server()
        {
            if((sock_cmd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
            {
                perror("fail to socket!");
                return -1;
            }

            bzero(&cmd_addr, sizeof(cmd_addr));
            cmd_addr.sin_family = PF_INET;
            cmd_addr.sin_port = htons(5006);
            cmd_addr.sin_addr.s_addr = inet_addr(d_ipaddr.c_str());

            if((sock_sink = socket(PF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("fail to socket!");
                return -1;
            }

            bzero(&sink_addr, sizeof(sink_addr));
            sink_addr.sin_family = PF_INET;
            sink_addr.sin_port = htons(5005);
            sink_addr.sin_addr.s_addr = inet_addr(d_ipaddr.c_str());

            if(connect(sock_sink, (struct sockaddr *)&sink_addr, sizeof(struct sockaddr)) < 0){
                printf("connect (%d)\n",errno);
                exit(1);
            } else {
                printf("connect sucessful\n");
            }

            return 1;
        }
        int sink_sync_impl::disconnect_server()
        {
            close(sock_sink);
            return 0;
        }
        int sink_sync_impl::handshake()
        {
            uint32_t cmd_buf[2]={0,0};
            cmd_buf[0] = 0xF0160001;
            sendto(sock_cmd, cmd_buf, sizeof(cmd_buf), 0, (struct sockaddr *)&cmd_addr, sizeof(cmd_addr));
            
            sleep(1);

            return 1;
        }
    } /* namespace zingsdr */
} /* namespace gr */

