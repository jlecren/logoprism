#include "logoprism/view/request_flow.hpp"
#include "logoprism/data/color.hpp"

namespace logoprism {
  namespace view {

    request_flow::request_flow(glm::vec2 const& position, glm::vec2 const& dimension, float keep_alive) :
      view::object(position, dimension),
      top_left(position.x, position.y),
      bottom_right(position.x + dimension.x, position.y + dimension.y),
      top_right(bottom_right.x, top_left.y),
      bottom_left(top_left.x, bottom_right.y),
      top_center((top_left.x + top_right.x) * 2.0 / 5.0f, (top_left.y + bottom_left.y) * 1.0 / 4.0),
      bottom_center((bottom_left.x + bottom_right.x) * 2.0 / 5.0f, (top_left.y + bottom_left.y) * 3.0 / 4.0),
      keep_alive(keep_alive),
      items_left(top_left, bottom_left, ".", view::alignment::left),
      items_center(top_center, bottom_center, "-", view::alignment::center),
      items_right(top_right, bottom_right, "/", view::alignment::right) {}

    void request_flow::set_requests(data::requests const& requests) {
      this->requests = requests;

      this->workers.clear();
      std::transform(std::begin(this->requests), std::end(this->requests), std::inserter(this->workers, std::end(this->workers)),
                     [](data::request const& r) { return r.worker; });
    }

    void request_flow::logic(data::timings const& timings) {
      object::logic(timings);

      if (this->is_dead())
        return;

      if (timings.is_keyframe) {
        for (auto& request : this->requests) {
          if (this->request_views.find(request) != this->request_views.end())
            continue;

          view::request view(request);
          if (!view.is_processing(timings))
            continue;

          this->request_views.insert(std::make_pair(request, std::move(view)));

          data::duration worker_litetime = data::microseconds(static_cast< uint64_t >(this->keep_alive * 1000000));
          if (!request.keep_alive)
            worker_litetime = request.duration;

          if (this->worker_views.find(request.worker) != this->worker_views.end())
            this->worker_views.erase(request.worker);

          this->worker_views.insert(std::make_pair(request.worker, view::worker(worker_litetime, request)));
        }

        for (auto it = std::begin(this->request_views), end = std::end(this->request_views); it != end;) {
          if (it->second.is_complete(timings) || (this->requests.find(it->first) == this->requests.end()))
            it->second.kill();

          if (it->second.is_dead())
            this->request_views.erase(it++);
          else
            ++it;
        }

        for (auto it = std::begin(this->worker_views), end = std::end(this->worker_views); it != end;) {
          if (this->workers.find(it->first) == this->workers.end())
            it->second.kill();

          if (it->second.is_dead())
            this->worker_views.erase(it++);
          else
            ++it;
        }

        std::set< std::string >& items_left   = this->items_left.get_items();
        std::set< std::string >& items_center = this->items_center.get_items();
        std::set< std::string >& items_right  = this->items_right.get_items();

        items_left.clear();
        items_center.clear();
        items_right.clear();

        std::transform(std::begin(this->request_views), std::end(this->request_views), std::inserter(items_left, std::end(items_left)),
                       [](std::pair< data::request, view::request > const& pair) { return pair.first.source; });

        std::transform(std::begin(this->request_views), std::end(this->request_views), std::inserter(items_center, std::end(items_center)),
                       [](std::pair< data::request, view::request > const& pair) { return pair.first.worker; });

        std::transform(std::begin(this->request_views), std::end(this->request_views), std::inserter(items_right, std::end(items_right)),
                       [](std::pair< data::request, view::request > const& pair) { return pair.first.target; });
      }

      for (auto& pair : this->worker_views) {
        pair.second.logic(timings);
      }

      for (auto& pair : this->request_views) {
        pair.second.logic(timings);
      }

      this->items_left.logic(timings);
      this->items_center.logic(timings);
      this->items_right.logic(timings);

      for (auto& pair : this->request_views) {
        pair.second.position_left   = this->items_left.get_position(pair.first.source);
        pair.second.position_center = this->items_center.get_position(pair.first.worker);
        pair.second.position_right  = this->items_right.get_position(pair.first.target);
      }

      for (auto& pair : this->worker_views) {
        pair.second.position_center = this->items_center.get_position(pair.first);
        pair.second.position_left   = pair.second.position_center - glm::vec2(100.0, 0.0);
        pair.second.position_right  = pair.second.position_center + glm::vec2(100.0, 0.0);
      }
    } // logic

    void request_flow::draw(renderer::base& renderer, data::timings const& timings) {
      object::draw(renderer, timings);

      if (this->is_dead())
        return;

      for (auto& pair : this->worker_views) {
        pair.second.draw(renderer, timings);
      }

      for (auto& pair : this->request_views) {
        pair.second.draw(renderer, timings);
      }

      for (auto& pair : this->request_views) {
        pair.second.draw_status(renderer, timings);
      }

      this->items_left.draw(renderer, timings);
      this->items_right.draw(renderer, timings);
    }

  }
}
