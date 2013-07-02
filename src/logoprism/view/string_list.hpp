#ifndef __LOGOPRISM_VIEW_STRING_LIST_HPP__
#define __LOGOPRISM_VIEW_STRING_LIST_HPP__

#include "logoprism/view/string.hpp"
#include "logoprism/view/bond.hpp"

#include <set>

namespace logoprism {
  namespace view {

    struct string_list {
      protected:
        typedef std::map< std::string, view::string > view_map_t;

      public:
        string_list(glm::vec2 const& top, glm::vec2 const& bottom, std::string const& separator, view::alignment const alignment=view::alignment::left);

        std::set< std::string >& get_items() { return this->items; }
        void                     clear() { this->items.clear(); }

        void logic(data::timings const& timings);
        void draw(renderer::base& renderer, data::timings const& timings);

        glm::vec2 get_position(std::string const& string);

        void refresh_bonds();

        size_t token_limit();
        void   set_token_limit(size_t const token_count);

      public:
        view::string    top;
        view::string    bottom;
        std::string     separator;
        view::alignment alignment;
        size_t          token_count;

      protected:
        view_map_t              views;
        std::set< std::string > items;

        std::vector< view::bond > bonds;

        void        make_view(std::string const& string);
        std::string split(std::string const& string);
    };

  }
}

#endif // ifndef __LOGOPRISM_VIEW_STRING_LIST_HPP__
