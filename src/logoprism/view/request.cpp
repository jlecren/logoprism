#include "logoprism/view/request.hpp"

#include "logoprism/config/config.hpp"
#include "logoprism/data/color.hpp"

template< typename T, typename TT >
static inline std::basic_ostream< T >& operator<<(std::basic_ostream< T >& stream, glm::detail::tvec2< TT > vec2) {
  return stream << vec2.x << "x" << vec2.y;
}

namespace logoprism {
  namespace view {

    request::request(data::request const& data) :
      data(data),
      width_factor((std::log(this->data.size_in_bytes) / std::log(10) - 2.0) * 3.0)
    {}

    data::datetime request::simulation_time(data::timings const& timings) const {
      return timings.simulation_time;
    }

    data::duration request::offset_start(data::timings const& timings) const {
      return this->data.start_time - timings.simulation_time;
    }

    data::duration request::offset_end(data::timings const& timings) const {
      return this->data.start_time + this->data.duration - timings.simulation_time;
    }

    void request::logic(data::timings const& timings) {
      object::logic(timings);

      if (data.source.size() > 1000) {
        std::clog << "data.source: " << &data << std::endl;
        std::clog << "data.source: " << data.source.size() << std::endl;
        throw std::runtime_error("");
      }

      this->color = data::string_color(data.source);

      double const offset_start = data::floating_seconds(this->offset_start(timings));
      double const offset_end   = data::floating_seconds(this->offset_end(timings));

      if (offset_end >= 0.0) {
        double curve_start = std::max(0.0, std::min(1.0, offset_start));
        double curve_end   = std::max(0.0, std::min(1.0, offset_end));

        glm::vec2 middle_point   = (this->position_left + this->position_center) / 2.0f;
        glm::vec2 control_point0 = this->position_left + glm::vec2(middle_point.x - this->position_left.x, 0.0);
        glm::vec2 control_point1 = this->position_center + glm::vec2(middle_point.x - this->position_center.x, 0.0);

        this->bezier_left = view::bezier(view::bezier::control_points_type(this->position_left, control_point0, control_point1, this->position_center),
                                         glm::vec2(curve_start, curve_end), glm::vec2(1.0, 0.75));
      }

      if (offset_start <= 0.0) {
        double curve_start = 1.0 + std::max(-1.0, std::min(0.0, offset_start));
        double curve_end   = 1.0 + std::max(-1.0, std::min(0.0, offset_end));

        glm::vec2 middle_point   = (this->position_center + this->position_right) / 2.0f;
        glm::vec2 control_point0 = this->position_center + glm::vec2(middle_point.x - this->position_center.x, 0.0);
        glm::vec2 control_point1 = this->position_right + glm::vec2(middle_point.x - this->position_right.x, 0.0);

        this->bezier_right = view::bezier(view::bezier::control_points_type(this->position_center, control_point0, control_point1, this->position_right),
                                          glm::vec2(curve_start, curve_end), glm::vec2(0.0, 0.25));
      }
    } // logic

    void request::draw(renderer::base& renderer, data::timings const& timings) {
      object::draw(renderer, timings);

      if (this->is_dead())
        return;

      double const offset_start = data::floating_seconds(this->offset_start(timings));
      double const offset_end   = data::floating_seconds(this->offset_end(timings));

      if (offset_end >= 0.0)
        renderer.render(this->bezier_left.control_points, this->bezier_left.range, this->bezier_left.fading, this->get_color(), this->width_factor);

      if (offset_start <= 0.0)
        renderer.render(this->bezier_right.control_points, this->bezier_right.range, this->bezier_right.fading, this->get_color(), this->width_factor);

    }

    void request::draw_status(renderer::base& renderer, data::timings const& timings) {
      double const offset_end = data::floating_seconds(this->offset_end(timings));

      if (offset_end <= 0.0) {
        float factor = glm::clamp(1.0 + offset_end, 0.0, 1.0);

        glm::vec2 position = this->position_center;

        glm::vec4 status_color;
        glm::vec2 direction;

        text::anchor anchor = text::anchor::CENTER_LEFT;

        if (data.status.size() > 1000) {
          std::clog << "data.status: " << &data << std::endl;
          std::clog << "data.status: " << data.status.size() << std::endl;
          throw std::runtime_error("");
        }

        if (data.status.empty())
          return;

        switch (data.status[0]) {
          case '2':
            direction    = glm::vec2(-10.0f, -20.0f);
            status_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
            anchor       = text::anchor::CENTER_RIGHT;
            factor      *= factor;
            factor      *= factor;
            break;

          case '3':
            direction    = glm::vec2(-10.0f, 50.0f);
            status_color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
            anchor       = text::anchor::CENTER_RIGHT;
            break;

          case '4':
            direction    = glm::vec2(30.0f, -30.0f);
            status_color = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
            factor       = sqrt(factor);
            factor       = sqrt(factor);
            break;

          default:
          case '5':
            direction    = glm::vec2(30.0f, -30.0f);
            status_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            factor       = sqrt(factor);
            factor       = sqrt(factor);
            break;
        }

        position      += direction * (1.0f - factor);
        status_color.w = factor;

        static std::string const status_font = config::get("display.fonts.request-status", "monospace 9");
        renderer.render(data.status, position, anchor, status_font, status_color);
      }
    } // draw

  }
}
