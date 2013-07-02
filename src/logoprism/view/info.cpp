#include "logoprism/view/info.hpp"

#include "logoprism/config/config.hpp"

#include <boost/lexical_cast.hpp>

namespace logoprism {
  namespace view {

    info::info(glm::vec2 const& position, glm::vec2 const& dimension) :
      view::object(position, dimension),
      message(""),
      timing_message(""),
      buffering_message("") {
      this->set_speed(1.0);
      this->color = view::color::white;
    }

    void info::set_speed(double const info) {
      std::stringstream stream;

      stream.precision(3);
      stream << "×" << info;
      this->set_message(stream.str());
    }

    void info::set_message(std::string const& message) {
      this->message       = message;
      this->vitality_rate = 5.0;
    }

    void info::set_buffer_percentage(size_t const buffer_percentage) {
      std::stringstream stream;

      stream << buffer_percentage << "%";
      this->buffering_message = stream.str();
    }

    void info::logic(data::timings const& timings) {
      view::object::logic(timings);

      if (this->vitality >= 1.0)
        this->vitality_rate = -1.0;

      std::stringstream stream;
      stream << timings.simulation_time;
      this->timing_message = stream.str();
    }

    void info::draw(renderer::base& renderer, data::timings const& timings) {
      static std::string const message_font = config::get("display.fonts.info-message", "monospace 72");
      static std::string const time_font    = config::get("display.fonts.info-time", "monospace 12");

      object::draw(renderer, timings);

      if (!this->is_dead())
        renderer.render(this->message, this->position, text::anchor::CENTER_CENTER, message_font, this->get_color());
      renderer.render(this->timing_message, this->position + glm::vec2(0.0f, -this->dimension.y / 2.0f), text::anchor::TOP_CENTER, time_font, this->get_color(1.0));
    }

  }
}
