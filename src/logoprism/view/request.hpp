#ifndef __LOGOPRISM_VIEW_REQUEST_HPP__
#define __LOGOPRISM_VIEW_REQUEST_HPP__

#include "logoprism/data/request.hpp"
#include "logoprism/view/object.hpp"
#include "logoprism/view/string_list.hpp"

namespace logoprism {
  namespace view {

    struct bezier {
      typedef std::tuple< glm::vec2, glm::vec2, glm::vec2, glm::vec2 > control_points_type;

      bezier() : control_points(), range(), fading() {}
      bezier(control_points_type const& control_points, glm::vec2 const& range, glm::vec2 const& fading) :
        control_points(control_points),
        range(range),
        fading(fading)
      {}

      control_points_type control_points;
      glm::vec2           range;
      glm::vec2           fading;
    };

    struct request : public view::object {
      request(data::request const& request);

      data::datetime simulation_time(data::timings const& timings) const;
      data::duration offset_start(data::timings const& timings) const;
      data::duration offset_end(data::timings const& timings) const;

      bool is_processing(data::timings const& timings) {
        return data::floating_seconds(this->offset_start(timings)) <= 2.0 && data::floating_seconds(this->offset_end(timings)) >= -2.0;
      }

      bool is_complete(data::timings const& timings) {
        return data::floating_seconds(this->offset_end(timings)) < -2.0;
      }

      bool is_pending(data::timings const& timings) {
        return data::floating_seconds(this->offset_start(timings)) > 2.0;
      }

      bool operator<(view::request const& other) const {
        return this->data < other.data;
      }

      bool operator==(view::request const& other) const {
        return this->data == other.data;
      }

      void logic(data::timings const& timings);
      void draw(renderer::base& renderer, data::timings const& timings);
      void draw_status(renderer::base& renderer, data::timings const& timings);

      data::request const data;
      float const         width_factor;

      glm::vec2 position_left;
      glm::vec2 position_center;
      glm::vec2 position_right;

      view::bezier bezier_left;
      view::bezier bezier_right;
    };

  }
}

#endif // ifndef __LOGOPRISM_VIEW_REQUEST_HPP__
