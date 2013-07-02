#ifndef __LOGOPRISM_VIEW_REQUEST_FLOW_HPP__
#define __LOGOPRISM_VIEW_REQUEST_FLOW_HPP__

#include "logoprism/view/request.hpp"
#include "logoprism/data/request.hpp"
#include "logoprism/view/worker.hpp"
#include "logoprism/view/object.hpp"
#include "logoprism/view/string_list.hpp"

namespace logoprism {
  namespace view {

    struct request_flow : public view::object {
      request_flow(glm::vec2 const& position, glm::vec2 const& dimension, float keep_alive);

      void set_requests(data::requests const& requests);

      void logic(data::timings const& timings);
      void draw(renderer::base& renderer, data::timings const& timings);

      protected:
        std::set< data::request > requests;
        std::set< std::string >   workers;

        std::map< data::request, view::request > request_views;
        std::map< std::string, view::worker >    worker_views;

        glm::vec2 top_left;
        glm::vec2 bottom_right;
        glm::vec2 top_right;
        glm::vec2 bottom_left;
        glm::vec2 top_center;
        glm::vec2 bottom_center;

        float keep_alive;

      public:
        view::string_list items_left;
        view::string_list items_center;
        view::string_list items_right;
    };

  }
}

#endif // ifndef __LOGOPRISM_VIEW_REQUEST_FLOW_HPP__
