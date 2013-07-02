#include "logoprism/logoprism.hpp"

#include "logoprism/utils/signals.hpp"
#include "logoprism/data/request_reader.hpp"
#include "logoprism/view/string_list.hpp"
#include "logoprism/video/encoder.hpp"
#include "logoprism/input/keys.hpp"

#include "logoprism/renderer/opengl.hpp"
#include "logoprism/renderer/cairo.hpp"

#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "GLFW/glfw3.h"

namespace logoprism {

  logoprism::logoprism() :
    application(),
    simulator(config::get("input.speed", 1.0), data::microseconds(static_cast< size_t >(config::get("input.keyframe-duration-us", 1000000)))),
    input_file(config::get("input.file")),
    keep_alive(config::get("input.keepalive")),
    display_size(config::get("display.width"), config::get("display.height")),
    request_views(glm::vec2(0.0, 0.0), this->display_size, this->keep_alive),
    info_view(this->display_size / 2.0f, this->display_size) {
    namespace bfs = boost::filesystem;

    if (this->offscreen || (config::get("display.renderer") == "cairo"))
      this->renderer.reset(new renderer::cairo(glm::ivec2(config::get("display.width"), config::get("display.height"))));
    else
      this->renderer.reset(new renderer::opengl(glm::ivec2(config::get("display.width"), config::get("display.height"))));

    if (!bfs::exists(this->input_file))
      throw std::runtime_error(this->input_file + " does not exist.");

    this->request_reader.reset(new data::request_reader(this->input_file, this->simulator, data::seconds(3600 * 24), data::seconds(10)));
    this->request_reader->start();

    if (config::get("output.video")) {
      double const framerate = config::get("output.framerate");

      this->encoder.reset(new video::encoder(*this->renderer, this->input_file, glm::ivec2(display_size.x, display_size.y), framerate, config::get("output.pipeline")));
      this->simulator.set_timelapse_duration(data::microseconds(1000000 / framerate + 1));
    }
  }

  void logoprism::tick() {
    this->timings = this->simulator.timings(this->timings);

    this->logic();
    this->draw();

    if (this->encoder)
      this->encoder->export_frame();

    this->pressed_keys_delay -= this->timings.timelapse;
    if (this->pressed_keys_delay < data::microseconds(0)) {
      this->on_key_press(this->timings);
      this->pressed_keys_delay += data::microseconds(300000);
    }
  }

  void logoprism::logic() {
    this->info_view.set_buffer_percentage(this->request_reader->buffering_percentage());

    if (this->timings.is_keyframe) {
      data::requests const& visible_requests = this->request_reader->visible_requests(this->timings);
      this->request_views.set_requests(visible_requests);

      if (visible_requests.empty())
        this->info_view.set_message("buffering...");
      else
        std::clog << "visible requests from " << visible_requests.begin()->start_time
                  << " to " << visible_requests.rbegin()->start_time << std::endl;

      if (visible_requests.empty() && this->request_reader->exhausted())
        utils::signals::kill();
      else
        std::clog << visible_requests.size() << " visible requests." << std::endl;
    }

    this->request_views.logic(this->timings);
    this->info_view.logic(this->timings);
  }

  void logoprism::draw() {
    this->renderer->erase();

    this->request_views.draw(*this->renderer, this->timings);
    this->info_view.draw(*this->renderer, this->timings);

    if (this->timings.is_keyframe)
      this->renderer->cleanup_cache();
  }

  void logoprism::on_key_press(data::timings const&) {
    if (this->pressed_keys.none())
      return;

    if (this->pressed_keys % input::keyset::enter
        && this->pressed_keys & (input::keyset::left_alt | input::keyset::right_alt)) {
      this->renderer->clear_cache();
      this->toggle_fullscreen();
      return;
    }

    double const speed_pow         = std::pow(10, std::floor(std::log(this->simulator.speed()) / std::log(10.0)) - 1);
    uint64_t     speed             = std::floor(this->simulator.speed() / speed_pow);
    size_t const left_token_limit  = this->request_views.items_left.token_limit();
    size_t const right_token_limit = this->request_views.items_right.token_limit();

    if (this->pressed_keys % input::keyset::space) {
      this->simulator.toggle_pause();
      if (this->simulator.speed() == 0.0)
        this->info_view.set_message("pause");
      else
        this->info_view.set_message("continue");
    }

    if (this->pressed_keys % input::keyset::right) {
      if ((this->pressed_keys & (input::keyset::left_control | input::keyset::right_control))
          && (this->pressed_keys & (input::keyset::left_alt | input::keyset::right_alt))) {
        this->info_view.set_message("1h →");
        this->timings.skip_timelapse = data::seconds(3600);
      } else if (this->pressed_keys & (input::keyset::left_alt | input::keyset::right_alt)) {
        this->info_view.set_message("1m →");
        this->timings.skip_timelapse = data::seconds(60);
      } else if (this->pressed_keys & (input::keyset::left_control | input::keyset::right_control)) {
        this->info_view.set_message("5s →");
        this->timings.skip_timelapse = data::seconds(5);
      } else {
        this->info_view.set_message("1s →");
        this->timings.skip_timelapse = data::seconds(1);
      }

      this->pressed_keys_delay = this->timings.skip_timelapse;
    }

    if ((this->pressed_keys % input::keyset::numpad_add)
        || (this->pressed_keys % (input::keyset::equal | input::keyset::right_shift))
        || (this->pressed_keys % (input::keyset::equal | input::keyset::left_shift)))
      if (this->simulator.speed() < 100.0) {
        speed = (speed == 10) ? 25 : speed * 2;
        this->simulator.set_speed(speed * speed_pow);
        this->info_view.set_speed(this->simulator.speed());
      }

    if ((this->pressed_keys % input::keyset::numpad_substract)
        || (this->pressed_keys % input::keyset::number_6))
      if (this->simulator.speed() > 0.01) {
        speed = (speed == 25) ? 10 : speed / 2;
        this->simulator.set_speed(speed * speed_pow);
        this->info_view.set_speed(this->simulator.speed());
      }

    if (this->pressed_keys % input::keyset::letter_o)
      if (left_token_limit < 4) {
        this->request_views.items_left.set_token_limit(left_token_limit + 1);
        this->info_view.set_message(boost::lexical_cast< std::string >(left_token_limit + 1) + " source tokens");
      }

    if (this->pressed_keys % input::keyset::letter_l)
      if (left_token_limit > 0) {
        this->request_views.items_left.set_token_limit(left_token_limit - 1);
        this->info_view.set_message(left_token_limit == 0 ? "unlimited source tokens" : boost::lexical_cast< std::string >(left_token_limit - 1) + " source tokens");
      }

    if (this->pressed_keys % input::keyset::letter_p) {
      this->request_views.items_right.set_token_limit(right_token_limit + 1);
      this->info_view.set_message(boost::lexical_cast< std::string >(right_token_limit + 1) + " target tokens");
    }

    if (this->pressed_keys % input::keyset::letter_m)
      if (right_token_limit > 0) {
        this->request_views.items_right.set_token_limit(right_token_limit - 1);
        this->info_view.set_message(right_token_limit == 0 ? "unlimited target tokens" : boost::lexical_cast< std::string >(right_token_limit - 1) + " target tokens");
      }

  } // on_key_press

}
