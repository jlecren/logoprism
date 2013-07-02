#include "logoprism/video/encoder.hpp"
#include "logoprism/data/datetime.hpp"

#ifdef LOGOPRISM_ENABLE_GSTREAMER
# include <gst/app/gstappsrc.h>
#endif // ifdef LOGOPRISM_ENABLE_GSTREAMER

#include <sstream>
#include <iostream>
#include <boost/algorithm/string.hpp>

#include "GLFW/glfw3.h"

namespace logoprism {
  namespace video {

#ifdef LOGOPRISM_ENABLE_GSTREAMER

    /** callback whenever a gstreamer message is received */
    static gboolean bus_message(GstBus*, GstMessage* message, encoder* encoder) {
      switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
          GError* err      = NULL;
          gchar*  dbg_info = NULL;

          gst_message_parse_error(message, &err, &dbg_info);
          g_printerr("ERROR from element %s: %s\n", GST_OBJECT_NAME(message->src), err->message);
          g_printerr("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
          g_error_free(err);
          g_free(dbg_info);
          g_main_loop_quit(encoder->encoding_loop);
          break;
        }

        case GST_MESSAGE_EOS:
          g_main_loop_quit(encoder->encoding_loop);
          break;

        default:
          break;
      }
      return TRUE;
    }

    /** a separate thread to the glib main loop */
    struct encoding_thread {
      encoding_thread(GMainLoop* const encoding_loop) :
        encoding_loop(encoding_loop) {}

      void operator()() { g_main_loop_run(this->encoding_loop); }

      protected:
        GMainLoop*  encoding_loop;
        GstElement* pipeline;
        GstElement* appsrc;
    };

#endif // ifdef LOGOPRISM_ENABLE_GSTREAMER

    encoder::encoder(renderer::base& renderer, std::string const& filename, glm::ivec2 const& source_dimension, size_t const frames_per_second, std::string const& pipeline)
#ifdef LOGOPRISM_ENABLE_GSTREAMER
      :
      renderer(renderer),
      frames_per_second(frames_per_second),
      frame_duration(1000000000 / frames_per_second),
      frame_timestamp(0),
      source_dimension(source_dimension) {
      this->bytes_per_pixel   = 4;
      this->bits_per_pixel    = 8 * this->bytes_per_pixel;
      this->pixel_buffer_size = this->bytes_per_pixel * this->source_dimension.x * this->source_dimension.y;

      // initialize the pipeline with some input coming from our application, and video format converters
      std::stringstream pipeline_stream;
      pipeline_stream << "appsrc name=source block=true format=time ! ";
      pipeline_stream << "video/x-raw,"
                      << "format=BGRA,"
                      << "framerate=(fraction)" << this->frames_per_second << "/1,"
                      << "width=" << this->source_dimension.x << ","
                      << "height=" << this->source_dimension.y << ","
                      << "bpp=" << this->bits_per_pixel << " ! ";
      pipeline_stream << "queue ! videoflip method=vertical-flip ! tee name=video ";

      // add the default output pipeline or the pipeline coming from the configuration file, see gstreamer documentation
      if (pipeline == "") {
        pipeline_stream << "video. ! videoconvert ! video/x-raw,format=Y444 ! ";
        pipeline_stream << "queue name=enc-lq ! theoraenc quality=30 ! ";
        pipeline_stream << "queue name=mux-lq ! oggmux ! filesink location=" << filename << "-lq.ogv ";
      } else {
        pipeline_stream << boost::replace_all_copy(pipeline, "${filename}", filename);
      }

      std::string const& pipeline_string = pipeline_stream.str();
      std::clog << "I: encoding video with: " << pipeline_string << std::endl;

      // initialization and startup code for the gstreamer pipeline
      gst_init(NULL, NULL);

      this->pipeline = gst_parse_launch(pipeline_string.c_str(), NULL);
      this->bus      = gst_pipeline_get_bus(GST_PIPELINE(this->pipeline));
      gst_bus_add_watch(this->bus, (GstBusFunc) bus_message, this);

      this->source = gst_bin_get_by_name(GST_BIN(this->pipeline), "source");
      GstCaps* caps = gst_caps_new_simple("video/x-raw",
                                          "format", G_TYPE_STRING, "BGRA",
                                          "bpp", G_TYPE_INT, this->bits_per_pixel,
                                          "depth", G_TYPE_INT, this->bits_per_pixel,
                                          "width", G_TYPE_INT, this->source_dimension.x,
                                          "height", G_TYPE_INT, this->source_dimension.y,
                                          NULL);
      gst_app_src_set_caps(GST_APP_SRC(this->source), caps);
      gst_element_set_state(this->pipeline, GST_STATE_PLAYING);

      // start the encoding thread
      this->encoding_loop   = g_main_loop_new(NULL, TRUE);
      this->encoding_thread = boost::thread(video::encoding_thread(this->encoding_loop));
    }

#else // ifdef LOGOPRISM_ENABLE_GSTREAMER
    {}
#endif // ifdef LOGOPRISM_ENABLE_GSTREAMER

    encoder::~encoder() {
#ifdef LOGOPRISM_ENABLE_GSTREAMER

      // notify gstreamer that the stream has ended
      gst_app_src_end_of_stream(GST_APP_SRC(this->source));

      // wait for it to complete the encoding
      this->encoding_thread.join();

      gst_object_unref(this->bus);
      g_main_loop_unref(this->encoding_loop);
#endif // ifdef LOGOPRISM_ENABLE_GSTREAMER
    }

    void encoder::export_frame() {
#ifdef LOGOPRISM_ENABLE_GSTREAMER

      // create a gstreamer buffer and copy the current frame to it using the renderer
      GstBuffer* buffer = gst_buffer_new();
      GstMemory* memory = gst_allocator_alloc(NULL, this->pixel_buffer_size, NULL);
      gst_buffer_insert_memory(buffer, -1, memory);

      GstMapInfo map_info;
      gst_buffer_map(buffer, &map_info, GST_MAP_WRITE);
      this->renderer.read(glm::ivec2(0, 0), this->source_dimension, map_info.data);
      gst_buffer_unmap(buffer, &map_info);

      GST_BUFFER_DTS(buffer)      = this->frame_timestamp;
      GST_BUFFER_PTS(buffer)      = this->frame_timestamp;
      GST_BUFFER_DURATION(buffer) = this->frame_duration;
      this->frame_timestamp      += this->frame_duration;

      // push the frame to the appsrc input of the gstreamer pipeline
      gst_app_src_push_buffer(GST_APP_SRC(this->source), buffer);

#endif // ifdef LOGOPRISM_ENABLE_GSTREAMER
    }

  }
}
