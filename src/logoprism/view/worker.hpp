#ifndef __LOGOPRISM_VIEW_WORKER_HPP__
#define __LOGOPRISM_VIEW_WORKER_HPP__

#include "logoprism/view/request.hpp"

namespace logoprism {
  namespace view {

    struct worker : public view::request {
      worker(data::duration const& keepalive_duration, data::request const& data);

      void logic(data::timings const& timings);
      void draw(renderer::base& renderer, data::timings const& timings);

      data::duration keepalive_duration;
      data::datetime death_time;
    };

  }
}

#endif // ifndef __LOGOPRISM_VIEW_WORKER_HPP__
