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

#ifndef INCLUDED_ZINGSDR_SINK_SYNC_IMPL_H
#define INCLUDED_ZINGSDR_SINK_SYNC_IMPL_H

#include <zingsdr/sink_sync.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define RING_BUFFER_MAXSIZE (2048*4)
namespace gr {
    namespace zingsdr {

        class sink_sync_impl : public sink_sync
        {
            private:
                // Nothing to declare in this block.
                std::string d_ipaddr;
                uint16_t d_port;
                uint32_t d_tx_freq;
                uint32_t d_tx_vga;
                uint32_t d_sample_rate;
                int sock_cmd, sock_sink;
                struct sockaddr_in cmd_addr;
                struct sockaddr_in sink_addr;

                bool _running;
                pthread_t thread_send;
                char _buf[8192];
                uint32_t _buf_used;
                struct ring_buffer
                {
                    unsigned char *data[RING_BUFFER_MAXSIZE];
                    int front, rear;
                } *p;


                int send_sample();
                int connect_server();
                int disconnect_server();
                int handshake();

                static void *creat_send_thread(void *);

            public:
                void set_dc_offset(uint32_t dci, uint32_t dcq);
                void set_freq(uint32_t freq);
                void set_gain(uint32_t gain);
                void set_sample_rate(uint32_t sample_rate);

                sink_sync_impl(const std::string &ipaddr, uint32_t port, 
                        uint32_t tx_freq, uint32_t tx_vga, uint32_t sample_rate, 
                        uint32_t dci, uint32_t dcq);
                ~sink_sync_impl();

                // Where all the action really happens
                int work(int noutput_items,
                        gr_vector_const_void_star &input_items,
                        gr_vector_void_star &output_items);
        };

    } // namespace zingsdr
} // namespace gr

#endif /* INCLUDED_ZINGSDR_SINK_SYNC_IMPL_H */

