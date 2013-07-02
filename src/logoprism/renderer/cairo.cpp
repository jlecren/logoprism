#include "logoprism/renderer/cairo.hpp"
#include "logoprism/view/object.hpp"
#include "logoprism/config/config.hpp"

#include <GL/glext.h>

#include <cairo/cairo.h>
#include <pango/pango.h>
#include <pango/pango-layout.h>
#include <pango/pangocairo.h>

#include <boost/lexical_cast.hpp>

namespace logoprism {
  namespace renderer {

    struct cairo::impl {
      impl(glm::ivec2 const& source_dimension) :
        source_dimension(source_dimension),
        pango_context(pango_font_map_create_context(pango_cairo_font_map_get_default())),
        cairo_surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->source_dimension.x, this->source_dimension.y)),
        cairo_context(cairo_create(this->cairo_surface)) {
        cairo_font_options_t* const font_options = cairo_font_options_create();

        cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_SUBPIXEL);
        cairo_font_options_set_hint_style(font_options, CAIRO_HINT_STYLE_FULL);
        cairo_font_options_set_hint_metrics(font_options, CAIRO_HINT_METRICS_ON);

        pango_cairo_context_set_font_options(pango_context, font_options);
        cairo_font_options_destroy(font_options);

        cairo_set_antialias(this->cairo_context, CAIRO_ANTIALIAS_SUBPIXEL);
        cairo_set_source_rgba(this->cairo_context, 0.0, 0.0, 0.0, 1.0);
        cairo_reset_clip(this->cairo_context);
        cairo_paint(this->cairo_context);
        cairo_surface_flush(this->cairo_surface);

        cairo_scale(this->cairo_context, 1.0, -1.0);
        cairo_translate(this->cairo_context, 0.0, -this->source_dimension.y);
      }

      ~impl() {
        cairo_destroy(this->cairo_context);
        cairo_surface_destroy(this->cairo_surface);
        g_object_unref(this->pango_context);
      }

      /**
       * Cached text handle, keeping reference to a pango context and layout, in order not to have to
       * recompute them each time.
       */
      struct text_handle {
        text_handle() : pango_font(nullptr), pango_layout(nullptr) {}
        text_handle(PangoContext* const pango_context, std::string const& text, text::anchor const& anchor, std::string const& font) :
          pango_font(pango_font_description_from_string(font.c_str())),
          pango_layout(pango_layout_new(pango_context)) {
          pango_context_set_font_description(pango_context, this->pango_font);
          pango_layout_context_changed(this->pango_layout);

          switch (anchor) {
            case text::anchor::TOP_LEFT:
            case text::anchor::CENTER_LEFT:
            case text::anchor::BOTTOM_LEFT:
              pango_layout_set_alignment(this->pango_layout, PANGO_ALIGN_LEFT);
              break;

            case text::anchor::TOP_CENTER:
            case text::anchor::CENTER_CENTER:
            case text::anchor::BOTTOM_CENTER:
              pango_layout_set_alignment(this->pango_layout, PANGO_ALIGN_CENTER);
              break;

            case text::anchor::TOP_RIGHT:
            case text::anchor::CENTER_RIGHT:
            case text::anchor::BOTTOM_RIGHT:
              pango_layout_set_alignment(this->pango_layout, PANGO_ALIGN_RIGHT);
              break;
          }
          pango_layout_set_markup(this->pango_layout, text.c_str(), -1);

          pango_layout_get_pixel_size(this->pango_layout, &this->dimensions.x, &this->dimensions.y);
          this->dimensions = glm::ivec2(glm::vec2(this->dimensions));
        }

        text_handle(text_handle&& o) :
          pango_font(o.pango_font),
          pango_layout(o.pango_layout),
          dimensions(o.dimensions),
          used(o.used) {
          o.pango_font   = nullptr;
          o.pango_layout = nullptr;
        }

        ~text_handle() {
          if (this->pango_font)
            pango_font_description_free(this->pango_font);

          if (this->pango_layout)
            g_object_unref(this->pango_layout);
        }

        PangoFontDescription* pango_font;
        PangoLayout*          pango_layout;
        glm::ivec2            dimensions;
        bool                  used;
      };

      text_handle const& make_text(std::string const& text, text::anchor const& anchor, std::string const& font) {
        std::string const& key = text + " - " + font + " - " + boost::lexical_cast< std::string >(static_cast< size_t >(anchor));

        {
          auto const& it = this->text_cache.find(key);
          if (it != this->text_cache.end()) {
            it->second.used = true;
            return it->second;
          }
        }

        text_handle& handle = this->text_cache.insert(std::make_pair(key, text_handle(this->pango_context, text, anchor, font))).first->second;

        handle.used = true;

        return handle;
      } // make_text

      glm::ivec2 const    source_dimension;
      PangoContext* const pango_context;
      cairo_surface_t*    cairo_surface;
      cairo_t*            cairo_context;

      std::unordered_map< std::string, text_handle > text_cache;
    };

    namespace {
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

    cairo::cairo(glm::ivec2 const& source_dimension) :
      renderer::base(source_dimension),
      impl_ptr(new cairo::impl(source_dimension)) {

      static bool const offscreen = config::get("display.offscreen");

      if (offscreen)
        return;

      // if not rendering offscreen, use OpenGL to render the Cairo image on the screen
      glDisable(GL_FOG);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, this->source_dimension.x, this->source_dimension.y, 0, -1.0, 1.0);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
      glDisable(GL_LIGHTING);
    }

    void cairo::cleanup_cache() {
      if (this->impl_ptr->text_cache.size() < 50)
        return;

      std::vector< std::string > unused;

      for (auto& pair : this->impl_ptr->text_cache) {
        if (pair.second.used)
          pair.second.used = false;
        else
          unused.push_back(pair.first);
      }

      for (auto const& key : unused) {
        this->impl_ptr->text_cache.erase(key);
      }
    }

    void cairo::clear_cache() {
      this->impl_ptr->text_cache.clear();
    }

    void cairo::erase() {
      auto& self = *this->impl_ptr;

      static bool const offscreen = config::get("display.offscreen");

      if (!offscreen)
        glDrawPixels(self.source_dimension.x, self.source_dimension.y, GL_BGRA, GL_UNSIGNED_BYTE, cairo_image_surface_get_data(self.cairo_surface));

      cairo_set_source_rgba(self.cairo_context, 0.0, 0.0, 0.0, 1.0);
      cairo_reset_clip(self.cairo_context);
      cairo_paint(self.cairo_context);
      cairo_surface_flush(self.cairo_surface);
    }

    glm::vec2 cairo::measure(std::string const& text, text::anchor const& anchor, std::string const& font) {
      auto const& handle = this->impl_ptr->make_text(text, anchor, font);

      return glm::vec2(handle.dimensions);
    }

    void cairo::render(std::string const& text, glm::vec2 const& position, text::anchor const& anchor, std::string const& font, glm::vec4 const& color) {
      auto&       self   = *this->impl_ptr;
      auto const& handle = self.make_text(text, anchor, font);

      glm::vec2 const& dimensions = glm::vec2(handle.dimensions);
      glm::vec2 const& top_left   = position + alignment_ratio(anchor) * dimensions;

      pango_cairo_update_context(self.cairo_context, self.pango_context);
      pango_context_set_font_description(self.pango_context, handle.pango_font);

      cairo_save(self.cairo_context);
      cairo_move_to(self.cairo_context, top_left.x, top_left.y);
      cairo_set_source_rgba(self.cairo_context, color.x, color.y, color.z, color.w);
      pango_cairo_update_layout(self.cairo_context, handle.pango_layout);
      pango_cairo_show_layout(self.cairo_context, handle.pango_layout);
      cairo_restore(self.cairo_context);
    }

    void cairo::render(std::tuple< glm::vec2, glm::vec2, glm::vec2, glm::vec2 > const& points, glm::vec2 const& range, glm::vec2 const&, glm::vec4 const& color, double const width) {
      auto& self = *this->impl_ptr;

      cairo_pattern_t* cairo_pattern = cairo_pattern_create_linear(std::get< 3 >(points).x, std::get< 3 >(points).y, std::get< 0 >(points).x, std::get< 0 >(points).y);

      if ((range.y <= 0.0) || (range.x >= 1.0)) {
        cairo_pattern_add_color_stop_rgba(cairo_pattern, 0.0, color.x, color.y, color.z, 0.0);
        cairo_pattern_add_color_stop_rgba(cairo_pattern, 1.0, color.x, color.y, color.z, 0.0);
      } else {
        cairo_pattern_add_color_stop_rgba(cairo_pattern, range.y, color.x, color.y, color.z, color.w);
        if (range.y < 1.0)
          cairo_pattern_add_color_stop_rgba(cairo_pattern, std::min(1.0, range.y + 0.000001), color.x, color.y, color.z, 0.0);

        cairo_pattern_add_color_stop_rgba(cairo_pattern, range.x, color.x, color.y, color.z, color.w);
        if (range.x > 0.0)
          cairo_pattern_add_color_stop_rgba(cairo_pattern, std::max(0.0, range.x - 0.000001), color.x, color.y, color.z, 0.0);
      }

      cairo_save(self.cairo_context);
      cairo_move_to(self.cairo_context, std::get< 0 >(points).x, std::get< 0 >(points).y);
      cairo_curve_to(self.cairo_context, std::get< 1 >(points).x, std::get< 1 >(points).y, std::get< 2 >(points).x, std::get< 2 >(points).y, std::get< 3 >(points).x, std::get< 3 >(points).y);
      cairo_set_source(self.cairo_context, cairo_pattern);
      cairo_set_line_width(self.cairo_context, width);
      cairo_set_line_cap(self.cairo_context, CAIRO_LINE_CAP_ROUND);
      cairo_stroke(self.cairo_context);
      cairo_restore(self.cairo_context);

      cairo_pattern_destroy(cairo_pattern);
    }

    void cairo::read(glm::ivec2 const& offset, glm::ivec2 const& dimension, uint8_t* const& target) {
      auto&                 self        = *this->impl_ptr;
      uint32_t const* const source_data = reinterpret_cast< uint32_t const* >(cairo_image_surface_get_data(self.cairo_surface));
      uint32_t* const       target_data = reinterpret_cast< uint32_t* >(target);

      cairo_surface_flush(self.cairo_surface);
      for (int32_t y = 0; y < dimension.y; ++y) {
        std::memcpy(target_data + y * dimension.x, source_data + (y + offset.y) * self.source_dimension.x + offset.x, dimension.x * sizeof(uint32_t));
      }
    }

    void cairo::write(glm::ivec2 const& offset, glm::ivec2 const& dimension, uint8_t const* const& source) {
      auto&                 self        = *this->impl_ptr;
      uint32_t* const       target_data = reinterpret_cast< uint32_t* >(cairo_image_surface_get_data(self.cairo_surface));
      uint32_t const* const source_data = reinterpret_cast< uint32_t const* >(source);

      cairo_surface_flush(self.cairo_surface);
      for (int32_t y = 0; y < dimension.y; ++y) {
        std::memcpy(target_data + (y + offset.y) * self.source_dimension.x + offset.x, source_data + y * dimension.x, dimension.x * sizeof(uint32_t));
      }
    }

  }
}
