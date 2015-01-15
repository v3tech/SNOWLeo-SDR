/* -*- c++ -*- */
/* 
 * Copyright 2013 <+YOU OR YOUR COMPANY+>.
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
#include "source_sync_impl.h"
#include <boost/thread/thread.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct sockaddr SA;

//#define RX_SAMPLES_NUM (1024*8)
//#define RING_BUFFER_MAXSIZE 32

namespace gr {
    namespace zingsdr {

        source_sync::sptr
            source_sync::make(const std::string &ipaddr, uint32_t port, uint32_t rx_freq, 
                    uint32_t rx_vga, uint32_t rx_lan, uint32_t sample_rate, uint32_t dci, uint32_t dcq)
            {
                return gnuradio::get_initial_sptr
                    (new source_sync_impl(ipaddr, port, rx_freq, rx_vga, 
                                          rx_lan, sample_rate, dcq, dcq));
            }

        /*
         * The private constructor
         */
        source_sync_impl::source_sync_impl(const std::string &ipaddr, uint32_t port, 
                uint32_t rx_freq, uint32_t rx_vga, uint32_t rx_lan, 
                uint32_t sample_rate, uint32_t dci, uint32_t dcq)
            : gr::sync_block("source_sync",
                    gr::io_signature::make(0, 0, sizeof(gr_complex)),
                    gr::io_signature::make(1, 1, sizeof(gr_complex)))
        {
            d_ipaddr = ipaddr;
            d_port = port;
            d_rx_vga = rx_vga;
            d_rx_lan = rx_lan;
            d_rx_freq = rx_freq;
            d_sample_rate = sample_rate;
            sock_len = sizeof(source_addr);
            if(this->connect_server() < 0)
                throw std::runtime_error("zingsdr:connect server error!\n");
            this->set_freq(d_rx_freq);
            this->set_gain(d_rx_vga);
            this->set_sample_rate(d_sample_rate);
            this->set_dc_offset(dci, dcq);
            this->handshake();

            _fifo = new boost::circular_buffer<gr_complex>(RX_SAMPLES_NUM*1024);
            if (!_fifo) {
                throw std::runtime_error( std::string(__FUNCTION__) + " " +
                        "Failed to allocate a sample FIFO!" );
            }
#define BOOST_THREAD
#ifndef BOOST_THREAD
            _running = true;
            if(pthread_create(&thread_recv, NULL, creat_recv_thread, this) < 0)
               throw std::runtime_error("zingsdr:creat recv thread error!\n");
#else
            this->start();
#endif

        }

        /*
         * Our virtual destructor.
         */
        source_sync_impl::~source_sync_impl()
        {
            if(this->disconnect_server() < 0)
                throw std::runtime_error("zingsdr:disconnect server error!\n");

#ifndef BOOST_THREAD
            _running = true;
            pthread_join(thread_recv, NULL);
#else
            this->stop();
#endif
            int i = 0;
            close(sock_source);
            if (_fifo)
            {
                delete _fifo;
                _fifo = NULL;
            }

        }

        int
            source_sync_impl::work(int noutput_items,
                    gr_vector_const_void_star &input_items,
                    gr_vector_void_star &output_items)
            {
                //const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];
                gr_complex *out = (gr_complex *) output_items[0];

                boost::unique_lock<boost::mutex> lock(_fifo_lock);

                /* Wait until we have the requested number of samples */
                int n_samples_avail = _fifo->size();

                while (n_samples_avail < noutput_items) {
                    _samp_avail.wait(lock);
                    n_samples_avail = _fifo->size();
                }

                for(int i = 0; i < noutput_items; ++i) {
                    out[i] = _fifo->at(0);
                    _fifo->pop_front();
                }

                //std::cerr << "-" << std::flush;

                return noutput_items;

            }
        void source_sync_impl::set_dc_offset(uint32_t dci, uint32_t dcq)
        {
            uint32_t cmd_buf[2]={0,0};
            cmd_buf[0] = 0xF0230000|((dci&0xFF)<<8)|(dcq&0xFF);
            sendto(sock_cmd, cmd_buf, sizeof(cmd_buf), 0, (struct sockaddr *)&cmd_addr, sizeof(cmd_addr));
        }

        void source_sync_impl::set_freq(uint32_t freq)
        {
            uint32_t cmd_buf[2]={0,0};
            cmd_buf[0] = 0xF0180000;
            cmd_buf[1] = freq; /*set rx freq*/
            sendto(sock_cmd, cmd_buf, sizeof(cmd_buf), 0, (struct sockaddr *)&cmd_addr, sizeof(cmd_addr));
        }
        void source_sync_impl::set_gain(uint32_t gain)
        { 
            uint32_t cmd_buf[2]={0,0};
            //d_rx_vga = d_rx_vga & 0x000000FF;
            //d_rx_lan = d_rx_lan & 0x000000FF;
            cmd_buf[0] = 0xF020D000 | gain;
            cmd_buf[1] = 0x03000000; //set rx vga
            sendto(sock_cmd, cmd_buf, sizeof(cmd_buf), 0, (struct sockaddr *)&cmd_addr, sizeof(cmd_addr));
        }
        void source_sync_impl::set_sample_rate(uint32_t sample_rate)
        { 
            uint32_t cmd_buf[2]={0,0};
            cmd_buf[0] = 0xF0240000;
            cmd_buf[1] = sample_rate;
            sendto(sock_cmd, cmd_buf, sizeof(cmd_buf), 0, (struct sockaddr *)&cmd_addr, sizeof(cmd_addr));
        }

        void *source_sync_impl::recv_sample(void *)
        {
            int len = 0;
            while(_running) {
                pthread_testcancel();
                len = recv(sock_source, _buf, RX_SAMPLES_NUM, MSG_WAITALL);
                if(len == 0)
                    break;
                pthread_testcancel();
                size_t i, n_avail, to_copy, num_samples = len/4;
                short *sample = (short *)_buf;
                _fifo_lock.lock();

                n_avail = _fifo->capacity() - _fifo->size();
                to_copy = (n_avail < num_samples ? n_avail : num_samples);

                for (i = 0; i < to_copy; i++ )
                {
                    /* Push sample to the fifo */
                    _fifo->push_back( gr_complex( *sample, *(sample+1) ) );

                    /* offset to the next I+Q sample */
                    sample += 2;
                }

                _fifo_lock.unlock();

                /* We have made some new samples available to the consumer in work() */
                if (to_copy) {
                    //std::cerr << "+" << std::flush;
                    _samp_avail.notify_one();
                }

                /* Indicate overrun, if neccesary */
                if (to_copy < num_samples)
                    std::cerr << "O" << std::flush;
            }

        }
        void *source_sync_impl::creat_recv_thread(void *param)
        {
            //pthread_detach(pthread_self());
            source_sync_impl *pThis = (source_sync_impl *)param;
            pThis->recv_sample(NULL);
            return NULL;
        }
        bool source_sync_impl::start()
        {
            _running = true;
            _thread = gr::thread::thread(_recv_samples, this);

            return true;
        }
        bool source_sync_impl::stop()
        {
            _running = false;
            _thread.join();

            return true;
        }
        void source_sync_impl::_recv_samples(source_sync_impl *obj)
        {
            obj->recv_samples();
        }
        void source_sync_impl::recv_samples()
        {
            while(_running) {
                if(recv(sock_source, _buf, RX_SAMPLES_NUM, MSG_WAITALL) == 0)
                    break;
#if 1
                size_t i, n_avail, to_copy, num_samples = RX_SAMPLES_NUM/4;
                short *sample = (short *)_buf;
                _fifo_lock.lock();

                n_avail = _fifo->capacity() - _fifo->size();
                to_copy = (n_avail < num_samples ? n_avail : num_samples);

                for (i = 0; i < to_copy; i++ )
                {
                    /* Push sample to the fifo */
                    _fifo->push_back( gr_complex( *sample, *(sample+1) ) );

                    /* offset to the next I+Q sample */
                    sample += 2;
                }

                _fifo_lock.unlock();

                /* We have made some new samples available to the consumer in work() */
                if (to_copy) {
                    //std::cerr << "+" << std::flush;
                    _samp_avail.notify_one();
                }

                /* Indicate overrun, if neccesary */
                if (to_copy < num_samples)
                    std::cerr << "O" << std::flush;
#endif
            }
        }

        int source_sync_impl::connect_server()
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

            if((sock_source = socket(PF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("fail to socket!");
                return -1;
            }

            sock_len = sizeof(struct sockaddr_in);
            memset(&source_addr, 0, sock_len);
            source_addr.sin_family = AF_INET;
            source_addr.sin_port = htons(5004);
            if (inet_pton(AF_INET, d_ipaddr.c_str(), &source_addr.sin_addr) <= 0) {
                printf("Wrong dest IP address!\n");
            }

            if(connect(sock_source, (struct sockaddr *)&source_addr, sizeof(struct sockaddr)) < 0){
                perror("connect");
                printf("connect (%d)\n",errno);
            } else {
                printf("connect sucessful\n");
            }

            return 1;
        }
        int source_sync_impl::disconnect_server()
        {
            return 0;
        }
        int source_sync_impl::handshake()
        {
            uint32_t cmd_buf[2]={0,0};
            cmd_buf[0] = 0xF0160101;
            sendto(sock_cmd, cmd_buf, sizeof(cmd_buf), 0, (struct sockaddr *)&cmd_addr, sizeof(cmd_addr));
            
            sleep(1);

            return 1;
        }


    } /* namespace zingsdr */
} /* namespace gr */

