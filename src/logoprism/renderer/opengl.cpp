#include "logoprism/renderer/opengl.hpp"
#include "logoprism/view/object.hpp"

#include <GL/glext.h>

#include <cairo/cairo.h>
#include <pango/pango.h>
#include <pango/pango-layout.h>
#include <pango/pangocairo.h>

#include <unordered_map>

static PFNGLGENERATEMIPMAPPROC __glGenerateMipmap = 0;
static PFNGLWINDOWPOS2IVPROC   __glWindowPos2iv   = 0;

namespace logoprism {
  namespace renderer {

    namespace {

      template< size_t const EndOffset = 3, size_t const BeginOffset = 0 >
      struct bezier_impl {
        static inline glm::vec2 compute(float const position, std::vector< glm::vec2 > const& points) {
          return bezier_impl< EndOffset - 1, BeginOffset >::compute(position, points) * (position)
                 + bezier_impl< EndOffset, BeginOffset + 1 >::compute(position, points) * (1.0f - position);
        }

      };

      template< size_t const BeginOffset >
      struct bezier_impl< BeginOffset, BeginOffset > {
        static inline glm::vec2 compute(float const, std::vector< glm::vec2 > const& points) { return points[BeginOffset]; }
      };

#ifdef _WIN32
      static void cairo_image_surface_blur(cairo_surface_t* surface, double radius) {
        // Steve Hanov, 2009
        // Released into the public domain.

        // get width, height
        int            width   = cairo_image_surface_get_width(surface);
        int            height  = cairo_image_surface_get_height(surface);
        unsigned char* dst     = (unsigned char*) malloc(width * height * 4);
        unsigned*      precalc =
          (unsigned*) malloc(width * height * sizeof(unsigned));
        unsigned char* src = cairo_image_surface_get_data(surface);
        double         mul = 1.f / ((radius * 2) * (radius * 2));
        int            channel;

        // The number of times to perform the averaging. According to wikipedia,
        // three iterations is good enough to pass for a gaussian.
        const size_t MAX_ITERATIONS = 3;
        size_t       iteration;

        memcpy(dst, src, width * height * 4);

        for (iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
          for (channel = 0; channel < 4; channel++) {
            int x, y;

            // precomputation step.
            unsigned char* pix = src;
            unsigned*      pre = precalc;

            pix += channel;
            for (y = 0; y < height; y++) {
              for (x = 0; x < width; x++) {
                int tot = pix[0];
                if (x > 0)
                  tot += pre[-1];
                if (y > 0)
                  tot += pre[-width];
                if ((x > 0) && (y > 0))
                  tot -= pre[-width - 1];
                *pre++ = tot;
                pix   += 4;
              }
            }

            // blur step.
            pix = dst + (int) radius * width * 4 + (int) radius * 4 + channel;
            for (y = radius; y < height - radius; y++) {
              for (x = radius; x < width - radius; x++) {
                int l   = x < radius ? 0 : x - radius;
                int t   = y < radius ? 0 : y - radius;
                int r   = x + radius >= width ? width - 1 : x + radius;
                int b   = y + radius >= height ? height - 1 : y + radius;
                int tot = precalc[r + b * width] + precalc[l + t * width] -
                          precalc[l + b * width] - precalc[r + t * width];
                *pix = (unsigned char) (tot * mul);
                pix += 4;
              }
              pix += (int) radius * 2 * 4;
            }
          }
          memcpy(src, dst, width * height * 4);
        }

        free(dst);
        free(precalc);
      } // cairo_image_surface_blur

#endif // ifdef _WIN32

      static glm::vec2 alignment_ratio(text::anchor const& anchor) {
        switch (anchor) {
          default:
          case text::anchor::TOP_LEFT:
            return glm::vec2(0.0f, 0.0f);

          case text::anchor::CENTER_LEFT:
            return glm::vec2(0.0f, -0.5f);

          case text::anchor::BOTTOM_LEFT:
            return glm::vec2(0.0f, -1.0f);

          case text::anchor::TOP_CENTER:
            return glm::vec2(-0.5f, 0.0f);

          case text::anchor::CENTER_CENTER:
            return glm::vec2(-0.5f, -0.5f);

          case text::anchor::BOTTOM_CENTER:
            return glm::vec2(-0.5f, -1.0f);

          case text::anchor::TOP_RIGHT:
            return glm::vec2(-1.0f, 0.0f);

          case text::anchor::CENTER_RIGHT:
            return glm::vec2(-1.0f, -0.5f);

          case text::anchor::BOTTOM_RIGHT:
            return glm::vec2(-1.0f, -1.0f);
        }
      }

    }

    struct opengl::impl {
      struct texture_handle {
        texture_handle() { glGenTextures(1, &this->texture); }
        ~texture_handle() {
          if (this->texture)
            glDeleteTextures(1, &this->texture);
        }

        GLuint     texture;
        glm::ivec2 dimensions;
        float      super_sampling_factor;
        bool       used;
      };

      /**
       * Render some text to an OpenGL texture, and cache it, using the text data, font and anchor as the key.
       *
       * If the cache already contains the key, no need to render it again, just return the texture.
       */
      texture_handle const& make_texture(std::string const& text, text::anchor const& anchor, std::string const& font) {
        std::string const& key = text + " - " + font + " - " + boost::lexical_cast< std::string >(static_cast< size_t >(anchor));

        {
          auto const& it = this->texture_cache.find(key);
          if (it != this->texture_cache.end()) {
            it->second.used = true;
            return it->second;
          }
        }

        texture_handle& handle = this->texture_cache.insert(std::make_pair(key, texture_handle())).first->second;

        handle.super_sampling_factor = 1.0;

        static PangoFontMap* const pango_fontmap = pango_cairo_font_map_get_default();
        PangoContext* const        pango_context = pango_font_map_create_context(pango_fontmap);

        cairo_font_options_t* const font_options = cairo_font_options_create();
        cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_SUBPIXEL);
        cairo_font_options_set_hint_style(font_options, CAIRO_HINT_STYLE_FULL);
        cairo_font_options_set_hint_metrics(font_options, CAIRO_HINT_METRICS_ON);

        pango_cairo_context_set_font_options(pango_context, font_options);
        cairo_font_options_destroy(font_options);

        PangoFontDescription* const pango_font = pango_font_description_from_string(font.c_str());

        float const scaling_factor = std::max(1.0f, pango_font_description_get_size(pango_font) / static_cast< float >(40 * PANGO_SCALE));
        if (scaling_factor > 1.0)
          pango_font_description_set_size(pango_font, 40 * PANGO_SCALE);

#ifdef _MSC_VER
        if (scaling_factor > 1.0)
          handle.super_sampling_factor = 3.0;
#endif // ifdef _MSC_VER

        pango_context_set_font_description(pango_context, pango_font);

        PangoLayout* const pango_layout = pango_layout_new(pango_context);
        switch (anchor) {
          case text::anchor::TOP_LEFT:
          case text::anchor::CENTER_LEFT:
          case text::anchor::BOTTOM_LEFT:
            pango_layout_set_alignment(pango_layout, PANGO_ALIGN_LEFT);
            break;

          case text::anchor::TOP_CENTER:
          case text::anchor::CENTER_CENTER:
          case text::anchor::BOTTOM_CENTER:
            pango_layout_set_alignment(pango_layout, PANGO_ALIGN_CENTER);
            break;

          case text::anchor::TOP_RIGHT:
          case text::anchor::CENTER_RIGHT:
          case text::anchor::BOTTOM_RIGHT:
            pango_layout_set_alignment(pango_layout, PANGO_ALIGN_RIGHT);
            break;
        }
        pango_layout_set_markup(pango_layout, text.c_str(), -1);

        pango_layout_get_pixel_size(pango_layout, &handle.dimensions.x, &handle.dimensions.y);
        handle.dimensions = glm::ivec2(glm::vec2(handle.dimensions) * scaling_factor * handle.super_sampling_factor);

        cairo_surface_t* const cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, handle.dimensions.x, handle.dimensions.y);
        cairo_t* const         cairo_context = cairo_create(cairo_surface);

        pango_cairo_update_context(cairo_context, pango_context);
        cairo_set_source_rgb(cairo_context, 0.0, 0.0, 0.0);
        cairo_paint(cairo_context);
        cairo_set_antialias(cairo_context, CAIRO_ANTIALIAS_SUBPIXEL);
        cairo_scale(cairo_context, scaling_factor * handle.super_sampling_factor, scaling_factor * handle.super_sampling_factor);

#ifdef _WIN32
        cairo_move_to(cairo_context, -0.1, -0.1);
        cairo_set_source_rgb(cairo_context, 0.5, 0.5, 0.5);
        pango_cairo_update_layout(cairo_context, pango_layout);
        pango_cairo_show_layout(cairo_context, pango_layout);

        cairo_move_to(cairo_context, 0.1, -0.1);
        cairo_set_source_rgb(cairo_context, 0.5, 0.5, 0.5);
        pango_cairo_update_layout(cairo_context, pango_layout);
        pango_cairo_show_layout(cairo_context, pango_layout);

        cairo_move_to(cairo_context, -0.1, 0.1);
        cairo_set_source_rgb(cairo_context, 0.5, 0.5, 0.5);
        pango_cairo_update_layout(cairo_context, pango_layout);
        pango_cairo_show_layout(cairo_context, pango_layout);

        cairo_move_to(cairo_context, 0.1, 0.1);
        cairo_set_source_rgb(cairo_context, 0.5, 0.5, 0.5);
        pango_cairo_update_layout(cairo_context, pango_layout);
        pango_cairo_show_layout(cairo_context, pango_layout);

        cairo_move_to(cairo_context, 0.0, 0.0);
        cairo_set_source_rgb(cairo_context, 1.0, 1.0, 1.0);
        pango_cairo_update_layout(cairo_context, pango_layout);
        pango_cairo_show_layout(cairo_context, pango_layout);

        if (scaling_factor > 1.0)
          cairo_image_surface_blur(cairo_surface, 1.0);

#else // ifdef _WIN32
        cairo_move_to(cairo_context, 0.0, 0.0);
        cairo_set_source_rgb(cairo_context, 1.0, 1.0, 1.0);

        pango_cairo_layout_path(cairo_context, pango_layout);
        cairo_fill(cairo_context);

#endif // ifdef _WIN32

        uint32_t* const color_data = reinterpret_cast< uint32_t* >(cairo_image_surface_get_data(cairo_surface));
        for (int32_t i = 0; i < handle.dimensions.x * handle.dimensions.y; ++i) {
          uint8_t* const components = reinterpret_cast< uint8_t* >(color_data + i);
          components[3] = (components[0] + components[1] + components[2]) / 3;
        }

        glEnable(GL_TEXTURE_RECTANGLE);
        glBindTexture(GL_TEXTURE_RECTANGLE, handle.texture);
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA8, handle.dimensions.x, handle.dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, cairo_image_surface_get_data(cairo_surface));

        cairo_surface_finish(cairo_surface);
        cairo_surface_write_to_png(cairo_surface, (key + ".png").c_str());

        cairo_destroy(cairo_context);
        cairo_surface_destroy(cairo_surface);

        g_object_unref(pango_layout);
        pango_font_description_free(pango_font);
        g_object_unref(pango_context);

        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        /*if (!__glGenerateMipmap)
          __glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) glfwGetProcAddress("glGenerateMipmap");
        glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
        __glGenerateMipmap(GL_TEXTURE_RECTANGLE);*/

        handle.used = true;
        return handle;
      } // make_texture

      std::unordered_map< std::string, texture_handle > texture_cache;
    };

    opengl::opengl(glm::ivec2 const& source_dimension) :
      renderer::base(source_dimension),
      impl_ptr(new opengl::impl()) {
      glDisable(GL_FOG);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, this->source_dimension.x, this->source_dimension.y, 0, -1.0, 1.0);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
      glDisable(GL_LIGHTING);

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glHint(GL_POINT_SMOOTH, GL_NICEST);
      glHint(GL_LINE_SMOOTH, GL_NICEST);
      glHint(GL_POLYGON_SMOOTH, GL_NICEST);
      glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

      glEnable(GL_POINT_SMOOTH);
      glEnable(GL_LINE_SMOOTH);
      glEnable(GL_POLYGON_SMOOTH);
    }

    void opengl::cleanup_cache() {
      if (this->impl_ptr->texture_cache.size() < 50)
        return;

      std::vector< std::string > unused;

      for (auto& pair : this->impl_ptr->texture_cache) {
        if (pair.second.used)
          pair.second.used = false;
        else
          unused.push_back(pair.first);
      }

      for (auto const& key : unused) {
        this->impl_ptr->texture_cache.erase(key);
      }
    }

    void opengl::clear_cache() {
      this->impl_ptr->texture_cache.clear();
    }

    void opengl::erase() {
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    glm::vec2 opengl::measure(std::string const& text, text::anchor const& anchor, std::string const& font) {
      auto const& handle = this->impl_ptr->make_texture(text, anchor, font);

      return glm::vec2(handle.dimensions) / handle.super_sampling_factor;
    }

    void opengl::render(std::string const& text, glm::vec2 const& position, text::anchor const& anchor, std::string const& font, glm::vec4 const& color) {
      auto const& handle = this->impl_ptr->make_texture(text, anchor, font);

      glm::vec2 const&  dimensions   = glm::vec2(handle.dimensions);
      glm::ivec2 const& top_left     = glm::ivec2(position + alignment_ratio(anchor) * dimensions);
      glm::ivec2 const& bottom_right = top_left + glm::ivec2(dimensions);

      glColor4fv(glm::value_ptr(color));
      glEnable(GL_TEXTURE_RECTANGLE);
      glBindTexture(GL_TEXTURE_RECTANGLE, handle.texture);

      glBegin(GL_QUADS);
      glTexCoord2iv(glm::value_ptr(glm::ivec2(0, 0)));
      glVertex2iv(glm::value_ptr(top_left));

      glTexCoord2iv(glm::value_ptr(glm::ivec2(0, handle.dimensions.y)));
      glVertex2iv(glm::value_ptr(glm::ivec2(top_left.x, bottom_right.y)));

      glTexCoord2iv(glm::value_ptr(handle.dimensions));
      glVertex2iv(glm::value_ptr(bottom_right));

      glTexCoord2iv(glm::value_ptr(glm::ivec2(handle.dimensions.x, 0)));
      glVertex2iv(glm::value_ptr(glm::ivec2(bottom_right.x, top_left.y)));
      glEnd();

      glDisable(GL_TEXTURE_RECTANGLE);
    }

    void opengl::render(std::tuple< glm::vec2, glm::vec2, glm::vec2, glm::vec2 > const& points, glm::vec2 const& range, glm::vec2 const& fading, glm::vec4 const& color, double const width) {
      double curve_start   = range.x;
      double curve_end     = range.y;
      size_t segment_count = static_cast< uint64_t >((curve_end - curve_start) * 50) + 1;
      double curve_step    = (curve_end - curve_start) / static_cast< double >(segment_count);

      auto const get_fading_alpha = [&](double const position) -> double {
                                      if (fading.x <= fading.y)
                                        return glm::clamp((position - fading.x) / (fading.y - fading.x), 0.0, 1.0);
                                      else
                                        return -glm::clamp((position - fading.x) / (fading.x - fading.y), -1.0, 0.0);
                                    };

      std::vector< glm::vec2 > points_vector;

      points_vector.emplace_back(std::get< 0 >(points));
      points_vector.emplace_back(std::get< 1 >(points));
      points_vector.emplace_back(std::get< 2 >(points));
      points_vector.emplace_back(std::get< 3 >(points));

      glm::vec2 prev;
      glm::vec2 next = bezier_impl< >::compute(curve_start, points_vector);

      std::vector< std::pair< glm::vec3, glm::vec3 > > segments;
      for (double curve_position = curve_start + curve_step; curve_position < curve_end; curve_position += curve_step) {
        prev = next;
        next = bezier_impl< >::compute(curve_position, points_vector);
        segments.push_back(std::make_pair(glm::vec3(prev.x, prev.y, get_fading_alpha(curve_position)),
                                          glm::vec3(next.x, next.y, get_fading_alpha(curve_position + curve_step))));
      }

      prev = next;
      next = bezier_impl< >::compute(curve_end, points_vector);
      segments.push_back(std::make_pair(glm::vec3(prev.x, prev.y, get_fading_alpha(curve_end)),
                                        glm::vec3(next.x, next.y, get_fading_alpha(curve_end))));

      glColor4fv(glm::value_ptr(color));
      glLineWidth(width);
      glBegin(GL_LINES);
      for (auto& pair : segments) {
        glColor4fv(glm::value_ptr(glm::vec4(color.x, color.y, color.z, color.w * pair.first.z)));
        glVertex2fv(glm::value_ptr(pair.first));
        glColor4fv(glm::value_ptr(glm::vec4(color.x, color.y, color.z, color.w * pair.second.z)));
        glVertex2fv(glm::value_ptr(pair.second));
      }
      glEnd();
    }

    void opengl::read(glm::ivec2 const& offset, glm::ivec2 const& dimension, uint8_t* const& target) {
      glReadPixels(offset.x, offset.y, dimension.x, dimension.y, GL_BGRA, GL_UNSIGNED_BYTE, target);
    }

    void opengl::write(glm::ivec2 const& offset, glm::ivec2 const& dimension, uint8_t const* const& source) {
      if (!__glWindowPos2iv)
        __glWindowPos2iv = (PFNGLWINDOWPOS2IVPROC) glfwGetProcAddress("glWindowPos2iv");
      __glWindowPos2iv(glm::value_ptr(offset));
      glDrawPixels(dimension.x, dimension.y, GL_BGRA, GL_UNSIGNED_BYTE, source);
    }

  }
}
