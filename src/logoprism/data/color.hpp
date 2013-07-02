#ifndef __LOGOPRISM_DATA_COLOR_HPP__
#define __LOGOPRISM_DATA_COLOR_HPP__

#include "logoprism/data/types.hpp"

namespace logoprism {
  namespace data {

    /**
     * Converts a string to a color, generated from the string's integer hash
     *
     * @param  string the input string to hash
     * @return        a color corresponding to the input string
     */
    static inline glm::vec4 string_color(std::string const& string) {
      size_t const    hash  = std::hash< std::string >()(string);
      glm::vec3 const color = glm::normalize(glm::vec3(static_cast< float >((hash / 7) % 255),
                                                       static_cast< float >((hash / 3) % 255),
                                                       static_cast< float >(hash % 255)));

      return glm::vec4(color.x, color.y, color.z, 1.0f);
    }

  }
}
#endif // ifndef __LOGOPRISM_DATA_COLOR_HPP__
