#ifndef __LOGOPRISM_RENDERER_OPENGL_HPP__
#define __LOGOPRISM_RENDERER_OPENGL_HPP__

#include "logoprism/renderer/renderer.hpp"

#include <memory>

namespace logoprism {
  namespace renderer {

    struct opengl : renderer::base {
      opengl(glm::ivec2 const& source_dimension);

      void cleanup_cache();
      void clear_cache();

      void erase();

      glm::vec2 measure(std::string const& text, text::anchor const& anchor, std::string const& font);
      void      render(std::string const& text, glm::vec2 const& position, text::anchor const& anchor, std::string const& font, glm::vec4 const& color);

      void render(std::tuple< glm::vec2, glm::vec2, glm::vec2, glm::vec2 > const& points, glm::vec2 const& range, glm::vec2 const& fading, glm::vec4 const& color, double const width);

      void read(glm::ivec2 const& offset, glm::ivec2 const& dimension, uint8_t* const& target);

      void write(glm::ivec2 const& offset, glm::ivec2 const& dimension, uint8_t const* const& source);

      private:
        struct impl;

        std::unique_ptr< opengl::impl > impl_ptr;
    };

  }
}

#endif // ifndef __LOGOPRISM_RENDERER_OPENGL_HPP__
