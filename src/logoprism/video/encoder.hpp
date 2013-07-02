#ifndef __LOGOPRISM_VIDEO_ENCODER_HPP__
#define __LOGOPRISM_VIDEO_ENCODER_HPP__

#include "logoprism/data/types.hpp"
#include "logoprism/renderer/renderer.hpp"

#include <fstream>
#include <boost/thread.hpp>

#ifdef LOGOPRISM_ENABLE_GSTREAMER
# include <gst/gst.h>
#endif // ifdef LOGOPRISM_ENABLE_GSTREAMER

namespace logoprism {
  namespace video {

    struct encoder {
      public:
        encoder(renderer::base& renderer, std::string const& filename, glm::ivec2 const& source_dimension, size_t const frames_per_second, std::string const& pipeline);
        ~encoder();

        /** reads the current frame from the encoder and pushes it to the video pipeline */
        void export_frame();

#ifdef LOGOPRISM_ENABLE_GSTREAMER
        renderer::base& renderer;

        size_t frames_per_second;
        size_t frame_duration;
        size_t frame_timestamp;

        glm::ivec2 source_dimension;
        size_t     bytes_per_pixel;
        size_t     bits_per_pixel;

        GstElement* pipeline;
        GstElement* source;
        GstBus*     bus;

        GMainLoop* encoding_loop;

        size_t pixel_buffer_size;

        boost::thread encoding_thread;
#endif // ifdef LOGOPRISM_ENABLE_GSTREAMER
    };

  }
}

#endif // ifndef __LOGOPRISM_VIDEO_ENCODER_HPP__
