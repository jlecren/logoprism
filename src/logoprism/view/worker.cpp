#include "logoprism/view/worker.hpp"
#include "logoprism/data/color.hpp"

#include <GL/gl.h>

namespace logoprism {
  namespace view {

    worker::worker(data::duration const& keepalive_duration, data::request const& data) :
      view::request(data),
      keepalive_duration(keepalive_duration),
      death_time(data.start_time + this->keepalive_duration) {
      this->color = glm::vec4(0.5, 0.5, 0.5, 1.0);
    }

    void worker::logic(data::timings const& timings) {
      object::logic(timings);

      double const offset_start = -0.1;
      double const offset_end   = 0.1;

      if (offset_end >= 0.0) {
        double curve_start = std::max(0.0, std::min(1.0, offset_start));
        double curve_end   = std::max(0.0, std::min(1.0, offset_end));

        glm::vec2 middle_point   = (this->position_left + this->position_center) / 2.0f;
        glm::vec2 control_point0 = this->position_left + glm::vec2(middle_point.x - this->position_left.x, 0.0);
        glm::vec2 control_point1 = this->position_center + glm::vec2(middle_point.x - this->position_center.x, 0.0);

        this->bezier_left = view::bezier(view::bezier::control_points_type(this->position_left, control_point0, control_point1, this->position_center),
                                         glm::vec2(curve_start, curve_end), glm::vec2(0.1, 0.0));
      }

      if (offset_start <= 0.0) {
        double curve_start = 1.0 + std::max(-1.0, std::min(0.0, offset_start));
        double curve_end   = 1.0 + std::max(-1.0, std::min(0.0, offset_end));

        glm::vec2 middle_point   = (this->position_center + this->position_right) / 2.0f;
        glm::vec2 control_point0 = this->position_center + glm::vec2(middle_point.x - this->position_center.x, 0.0);
        glm::vec2 control_point1 = this->position_right + glm::vec2(middle_point.x - this->position_right.x, 0.0);

        this->bezier_right = view::bezier(view::bezier::control_points_type(this->position_center, control_point0, control_point1, this->position_right),
                                          glm::vec2(curve_start, curve_end), glm::vec2(0.9, 1.0));
      }

      if ((timings.simulation_time >= this->death_time) && !this->is_dying())
        this->kill();

    } // logic

    void worker::draw(renderer::base& renderer, data::timings const& timings) {
      object::draw(renderer, timings);

      if (this->is_dead())
        return;

      renderer.render(this->bezier_left.control_points, this->bezier_left.range, this->bezier_left.fading, this->get_color(), 1.0);
      renderer.render(this->bezier_right.control_points, this->bezier_right.range, this->bezier_right.fading, this->get_color(), 1.0);
    }

  }
}
