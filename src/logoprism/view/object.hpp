#ifndef __LOGOPRISM_VIEW_OBJECT_HPP__
#define __LOGOPRISM_VIEW_OBJECT_HPP__

#include "logoprism/data/types.hpp"
#include "logoprism/data/datetime.hpp"
#include "logoprism/renderer/renderer.hpp"

#include <GLFW/glfw3.h>

namespace logoprism {
  namespace view {

    namespace color {
      static glm::vec4 const white = glm::vec4(1.0, 1.0, 1.0, 1.0);
    }

    struct object {
      object(glm::vec2 const& position=glm::vec2(), glm::vec2 const& dimension=glm::vec2());

      object const& operator=(object const& other) {
        this->position_fixed = other.position_fixed;
        this->position       = other.position;
        this->dimension      = other.dimension;

        return *this;
      }

      virtual void logic(data::timings const& timings);
      virtual void draw(renderer::base& renderer, data::timings const& timings);

      void kill();
      void resurrect();
      bool is_dying();
      bool is_dead();
      bool is_alive();

      glm::vec4 get_color() const { return glm::vec4(this->color.x, this->color.y, this->color.z, this->color.w * this->vitality); }
      glm::vec4 get_color(double const vitality) const { return glm::vec4(this->color.x, this->color.y, this->color.z, this->color.w * vitality); }

      bool      position_fixed;
      glm::vec2 position;
      glm::vec2 dimension;
      glm::vec4 color;

      bool   focused;
      double vitality;
      double vitality_rate;
    };

  }
}

#endif // ifndef __LOGOPRISM_VIEW_OBJECT_HPP__
