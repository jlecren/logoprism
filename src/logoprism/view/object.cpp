#include "logoprism/view/object.hpp"

#include <GL/gl.h>

namespace logoprism {
  namespace view {

    static double vitality_rate = 1.0;

    object::object(glm::vec2 const& position, glm::vec2 const& dimension) :
      position_fixed(false),
      position(position),
      dimension(dimension),
      color(view::color::white),
      focused(false),
      vitality(0.0),
      vitality_rate(+view::vitality_rate)
    {}

    void object::logic(data::timings const& timings) {
      this->vitality += this->vitality_rate * data::floating_seconds(timings.timelapse);
      this->vitality  = std::min(1.0, this->vitality);
      this->vitality  = std::max(0.0, this->vitality);
    }

    void object::draw(renderer::base&, data::timings const&) {}

    void object::kill() {
      this->vitality_rate = -std::abs(this->vitality_rate);
    }

    void object::resurrect() {
      this->vitality_rate = +std::abs(this->vitality_rate);
    }

    bool object::is_dying() {
      return this->vitality_rate < 0.0;
    }

    bool object::is_dead() {
      return this->is_dying() && this->vitality <= 0.0;
    }

    bool object::is_alive() {
      return !this->is_dying() && this->vitality >= 1.0;
    }

  }
}
