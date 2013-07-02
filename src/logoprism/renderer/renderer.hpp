#ifndef __LOGOPRISM_RENDERER_RENDERER_HPP__
#define __LOGOPRISM_RENDERER_RENDERER_HPP__

#include "logoprism/data/types.hpp"

namespace logoprism {

  namespace text {

    /**
     * Provides information about where the text should be anchored.
     *
     * @  : where the position of the text is
     * -> : to which direction the text area grows
     *
     *   TOP_LEFT:        TOP_CENTER:      TOP_RIGHT:
     *
     *   @------------->  <------@------>  <-------------@
     *   | TEXT   HERE      TEXT | HERE      TEXT   HERE |
     *   v                       v                       v
     *
     *
     *
     *   CENTER_LEFT:     CENTER_CENTER:   CENTER_RIGHT:
     *
     *   ^                       ^                       ^
     *   |                       |                       |
     *   @-TEXT---HERE->  <-TEXT-@-HERE->  <-TEXT---HERE-@
     *   |                       |                       |
     *   v                       v                       v
     *
     *
     *
     *   BOTTOM_LEFT:     BOTTOM_CENTER:   BOTTOM_RIGHT:
     *
     *   ^                       ^                       ^
     *   | TEXT   HERE      TEXT | HERE      TEXT   HERE |
     *   @------------->  <------@------>  <-------------@
     *
     */
    enum class anchor {
      TOP_LEFT,
      TOP_CENTER,
      TOP_RIGHT,
      CENTER_LEFT,
      CENTER_CENTER,
      CENTER_RIGHT,
      BOTTOM_LEFT,
      BOTTOM_CENTER,
      BOTTOM_RIGHT,
    };

  }

  namespace renderer {

    struct base {
      base(glm::ivec2 const& source_dimension);
      virtual ~base();

      /** cleanup the text cache, if there's any, discarding too old text data */
      virtual void cleanup_cache() = 0;

      /** clears the text cache, removing any cached text data from it */
      virtual void clear_cache() = 0;

      /** clears the screen */
      virtual void erase() = 0;

      /** @brief measures the number of pixels the text would take if rendered with given anchor and font. */
      virtual glm::vec2 measure(std::string const& text, text::anchor const& anchor, std::string const& font) = 0;

      /** @brief renders text at position, with given anchor, font, and color and adjusting the rendered area using overflow's dimension/ratio. */
      virtual void render(std::string const& text, glm::vec2 const& position, text::anchor const& anchor, std::string const& font, glm::vec4 const& color) = 0;

      /** @brief renders the lines, with given color and width. */
      virtual void render(std::tuple< glm::vec2, glm::vec2, glm::vec2, glm::vec2 > const& points, glm::vec2 const& range, glm::vec2 const& fading, glm::vec4 const& color, double const width) = 0;

      /** @brief copies 24bit RGB pixel data to target. */
      virtual void read(glm::ivec2 const& offset, glm::ivec2 const& dimension, uint8_t* const& target) = 0;

      /** @brief copies 24bit RGB pixel data from source. */
      virtual void write(glm::ivec2 const& offset, glm::ivec2 const& dimension, uint8_t const* const& source) = 0;

      glm::ivec2 const source_dimension;
    };

  }
}

#endif // ifndef __LOGOPRISM_RENDERER_RENDERER_HPP__
