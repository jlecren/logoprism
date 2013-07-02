#ifndef __LOGOPRISM_VIEW_BOND_HPP__
#define __LOGOPRISM_VIEW_BOND_HPP__

#include "logoprism/data/types.hpp"
#include "logoprism/data/datetime.hpp"

#include <iostream>

namespace logoprism {
  namespace view {

    struct bond {
      view::object* prev;
      view::object* next;
      glm::vec2     direction;

      bond(bond const& other) : prev(other.prev), next(other.next), direction(other.direction) {}
      bond(glm::vec2 direction) : prev(NULL), next(NULL), direction(direction) {}
      bond() : prev(NULL), next(NULL), direction(0.0f, 0.0f) {}

      void logic(data::timings const& timings) {
        // compute the move direction and variation, as something like square-square-root of the timelapse * speed...
        const glm::vec2 direction = next->position - prev->position;
        const glm::vec2 variation = (this->direction - direction) * static_cast< float >(std::min(1.0, 2.0f * sqrt(sqrt(data::floating_seconds(timings.timelapse * std::max(1.0, timings.simulation_speed))))));

        // if any of the positionable is fixed, update the other
        if (next->position_fixed) {
          if (!prev->position_fixed)
            prev->position -= variation;
        } else {
          next->position += variation;
        }

      }

    };

  }
}

#endif // ifndef __LOGOPRISM_VIEW_BOND_HPP__
