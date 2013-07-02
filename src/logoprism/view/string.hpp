#ifndef __LOGOPRISM_VIEW_STRING_HPP__
#define __LOGOPRISM_VIEW_STRING_HPP__

#include "logoprism/view/object.hpp"

namespace logoprism {
  namespace view {
    enum class alignment {
      left,
      right,
      center,
    };

    struct string : public view::object {
      string() {}

      string(std::string const& data, view::alignment const alignment=view::alignment::left);
      ~string();

      void logic(data::timings const& timings);
      void draw(renderer::base& renderer, data::timings const& timings);

      public:
        std::string     data;
        view::alignment alignment;
    };

  }
}

#endif // ifndef __LOGOPRISM_VIEW_STRING_HPP__
