#include "logoprism/view/string.hpp"
#include "logoprism/data/color.hpp"

#include "logoprism/config/config.hpp"

namespace logoprism {
  namespace view {

    template< typename T, typename TT >
    static inline std::basic_ostream< T >& operator<<(std::basic_ostream< T >& stream, glm::detail::tvec2< TT > vec2) {
      return stream << vec2.x << "x" << vec2.y;
    }

    string::string(std::string const& data, view::alignment const alignment) :
      data(data),
      alignment(alignment) {}

    string::~string() {}

    void string::logic(data::timings const& timings) {
      object::logic(timings);

      if (this->is_dead())
        return;

      this->color = data::string_color(this->data);
    }

    void string::draw(renderer::base& renderer, data::timings const& timings) {
      static std::string const string_font = config::get("display.fonts.node-name", "monospace 10");

      view::object::draw(renderer, timings);

      if (this->is_dead())
        return;

      text::anchor anchor = text::anchor::CENTER_LEFT;
      switch (this->alignment) {
        case view::alignment::left:
          anchor = text::anchor::CENTER_LEFT;
          break;

        case view::alignment::center:
          anchor = text::anchor::CENTER_CENTER;
          break;

        case view::alignment::right:
          anchor = text::anchor::CENTER_RIGHT;
          break;
      }

      this->dimension = renderer.measure(this->data, anchor, string_font);
      renderer.render(this->data, this->position, anchor, string_font, this->get_color());
    }

  }
}
