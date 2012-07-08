/* -*- c++ -*- */
/*
 * Copyright 2004,2010 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

/*
 * config.h is generated by configure.  It contains the results
 * of probing for features, options etc.  It should be the first
 * file included in your .cc file.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <howto_square2_ff.h>
#include <gr_io_signature.h>
#include <stdio.h>

/*
 * Create a new instance of howto_square2_ff and return
 * a boost shared_ptr.  This is effectively the public constructor.
 */
howto_square2_ff_sptr
howto_make_square2_ff ()
{
  return gnuradio::get_initial_sptr(new howto_square2_ff ());
}

/*
 * Specify constraints on number of input and output streams.
 * This info is used to construct the input and output signatures
 * (2nd & 3rd args to gr_block's constructor).  The input and
 * output signatures are used by the runtime system to
 * check that a valid number and type of inputs and outputs
 * are connected to this block.  In this case, we accept
 * only 1 input and 1 output.
 */
static const int MIN_IN = 1;	// mininum number of input streams
static const int MAX_IN = -1;	// maximum number of input streams
static const int MIN_OUT = 1;	// minimum number of output streams
static const int MAX_OUT = 1;	// maximum number of output streams

/*
 * The private constructor
 */
howto_square2_ff::howto_square2_ff ()
  : gr_sync_block ("square2_ff",
		   gr_make_io_signature (MIN_IN, MAX_IN, sizeof (float)),
		   gr_make_io_signature (MIN_OUT, MAX_OUT, sizeof (float)))
{
  // nothing else required in this example
}

/*
 * Our virtual destructor.
 */
howto_square2_ff::~howto_square2_ff ()
{
  // nothing else required in this example
}

int
howto_square2_ff::work (int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items)
{
  int num_inputs = input_items.size();

  if (num_inputs != zeros_so_far.size()) {
    zeros_so_far.resize(num_inputs, -1);
    current_channel_queue.resize(num_inputs, NULL);
  }

  float *out = (float *) output_items[0];

  for (int i = 0; i < noutput_items; i++) {
    for (int input = 0; input < num_inputs; input++) {
      float sample = ((const float *) input_items[input])[i];
      if (current_channel_queue.at(input) == NULL) {
        if (sample != 0) {
          // We've got a new audio segment.  Start queueing it.
          printf("Start queueing segment.\n");
          current_channel_queue.at(input) = new queue<float>;
          queued_audio_segments.push(current_channel_queue.at(input));
          zeros_so_far.at(input) = 0;
        }
      } else {
        if (sample == 0) {
          zeros_so_far.at(input)++;
          if (zeros_so_far.at(input) == 4000) {
            // This input has been silent for a while.  Stop queueing it.
            printf("Stop queueing segment.\n");
            current_channel_queue.at(input) = NULL;
          }
        } else {
          zeros_so_far.at(input) = 0;
        }
      }

      if (current_channel_queue.at(input) != NULL) {
        current_channel_queue.at(input)->push(sample);
      }
    }

    if (!queued_audio_segments.empty()) {
      queue<float> *current_audio_segment = queued_audio_segments.front();
      if (!current_audio_segment->empty()) {
        out[i] = current_audio_segment->front();
        current_audio_segment->pop();
      } else {
        // We reached the end of the current segment.  Throw it out.
        printf("End of segment.\n");
        delete current_audio_segment;
        queued_audio_segments.pop();
      }
    } else {
      out[i] = 0;
    }
  }

  // Tell runtime system how many output items we produced.
  return noutput_items;
}
