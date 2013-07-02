#ifndef __LOGOPRISM_VIEW_SPEED_HPP__
#define __LOGOPRISM_VIEW_SPEED_HPP__

#include "logoprism/view/object.hpp"

namespace logoprism {
  namespace view {

    struct info : public view::object {
      info(glm::vec2 const& position, glm::vec2 const& dimension);

      void set_speed(double const speed);
      void set_message(std::string const& message);

      void set_buffer_percentage(size_t const buffer_percentage);

      void logic(data::timings const& timings);
      void draw(renderer::base& renderer, data::timings const& timings);

      protected:
        std::string message;
        std::string timing_message;
        std::string buffering_message;
    };

  }
}

#endif // ifndef __LOGOPRISM_VIEW_SPEED_HPP__
