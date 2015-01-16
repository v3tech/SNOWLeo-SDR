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


#ifndef INCLUDED_ZINGSDR_SINK_SYNC_H
#define INCLUDED_ZINGSDR_SINK_SYNC_H

#include <zingsdr/api.h>
#include <gnuradio/sync_block.h>
#include <stdint.h>

namespace gr {
  namespace zingsdr {

    /*!
     * \brief <+description of block+>
     * \ingroup zingsdr
     *
     */
    class ZINGSDR_API sink_sync : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<sink_sync> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of zingsdr::sink_sync.
       *
       * To avoid accidental use of raw pointers, zingsdr::sink_sync's
       * constructor is in a private implementation
       * class. zingsdr::sink_sync::make is the public interface for
       * creating new instances.
       */
      static sptr make(const std::string &ipaddr, uint32_t port, 
              uint32_t tx_freq, uint32_t tx_vga1, uint32_t tx_vga2, uint32_t sample_rate, 
              uint32_t dci, uint32_t dcq);

      virtual void set_dc_offset(uint32_t dci, uint32_t dcq) = 0;
      virtual void set_freq(uint32_t freq) = 0;
      virtual void set_gain(uint32_t gain) = 0;
      virtual void set_sample_rate(uint32_t sample_rate) = 0;
    };

  } // namespace zingsdr
} // namespace gr

#endif /* INCLUDED_ZINGSDR_SINK_SYNC_H */

