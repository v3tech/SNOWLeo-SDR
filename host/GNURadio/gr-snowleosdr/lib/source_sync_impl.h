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

#ifndef INCLUDED_ZINGSDR_SOURCE_SYNC_IMPL_H
#define INCLUDED_ZINGSDR_SOURCE_SYNC_IMPL_H

#include <zingsdr/source_sync.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <sys/socket.h>       /* for AF_INET */

#include <gnuradio/thread/thread.h>
#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#define RX_SAMPLES_NUM (1024*8)
#define RING_BUFFER_MAXSIZE 2048

namespace gr {
    namespace zingsdr {

        class source_sync_impl : public source_sync
        {
            private:
                // Nothing to declare in this block.

                std::string d_ipaddr;
                uint32_t d_port;
                uint32_t d_rx_freq;
                uint32_t d_rx_vga;
                uint32_t d_rx_lan;
                uint32_t d_sample_rate;
                int sock_cmd, sock_source;
                struct sockaddr_in cmd_addr;
                struct sockaddr_in source_addr, client_addr;
                socklen_t sock_len;

                pthread_t thread_recv;

                struct ring_buffer
                {
                    unsigned char *data[RING_BUFFER_MAXSIZE];
                    int front, rear;
                } *p;
                
                char _buf[RX_SAMPLES_NUM];

                gr::thread::thread _thread;
                boost::circular_buffer<gr_complex> *_fifo;
                boost::mutex _fifo_lock;
                boost::condition_variable _samp_avail;

                bool _running;
                static void _recv_samples(source_sync_impl *obj);
                void recv_samples();

                int connect_server();
                int disconnect_server();
                int handshake();

                static void *creat_recv_thread(void *);
                void *recv_sample(void *);
            protected:
                bool start();
                bool stop();
            public:
                void set_dc_offset(uint32_t dci, uint32_t dcq);
                void set_freq(uint32_t freq);
                void set_gain(uint32_t gain);
                void set_sample_rate(uint32_t sample_rate);

                source_sync_impl(const std::string &ipaddr, uint32_t port, 
                        uint32_t rx_freq, uint32_t rx_vga, uint32_t rx_lan, 
                        uint32_t sample_rate, uint32_t dci, uint32_t dcq);
                ~source_sync_impl();

                // Where all the action really happens
                int work(int noutput_items,
                        gr_vector_const_void_star &input_items,
                        gr_vector_void_star &output_items);
        };

    } // namespace zingsdr
} // namespace gr

#endif /* INCLUDED_ZINGSDR_SOURCE_SYNC_IMPL_H */

