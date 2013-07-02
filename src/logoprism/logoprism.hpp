#ifndef __LOGOPRISM_LOGOPRISM_HPP__
#define __LOGOPRISM_LOGOPRISM_HPP__

#include "logoprism/config/config.hpp"
#include "logoprism/application.hpp"

#include "logoprism/data/request_reader.hpp"
#include "logoprism/data/simulator.hpp"
#include "logoprism/view/request.hpp"
#include "logoprism/view/request_flow.hpp"
#include "logoprism/view/info.hpp"
#include "logoprism/video/encoder.hpp"

#include "logoprism/renderer/renderer.hpp"

namespace logoprism {

  struct logoprism : application {
    public:
      logoprism();

      void tick();
      void stop() { this->request_reader->stop(); }

    private:
      void on_key_press(data::timings const& timings);

      void logic();
      void draw();

      data::simulator   simulator;
      std::string const input_file;
      double const      keep_alive;
      glm::vec2 const   display_size;

      std::unique_ptr< data::request_reader > request_reader;
      std::unique_ptr< renderer::base >       renderer;
      std::unique_ptr< video::encoder >       encoder;

      view::request_flow request_views;
      view::info         info_view;

      std::set< data::request > visible_requests;
      data::timings             timings;
  };

}

#endif // ifndef __LOGOPRISM_LOGOPRISM_HPP__
