#include "logoprism/view/string_list.hpp"
#include <boost/algorithm/string.hpp>

namespace logoprism {
  namespace view {

    string_list::string_list(glm::vec2 const& top, glm::vec2 const& bottom, std::string const& separator, view::alignment alignment) :
      separator(separator),
      alignment(alignment),
      token_count(0) {
      this->top.position          = top;
      this->top.position_fixed    = true;
      this->bottom.position       = bottom;
      this->bottom.position_fixed = true;
    }

    void string_list::refresh_bonds() {
      this->bonds.clear();
      size_t    view_count     = std::count_if(this->views.begin(), this->views.end(), [](view_map_t::value_type& pair) { return !pair.second.is_dying(); }) + 1;
      glm::vec2 sort_direction = (this->bottom.position - this->top.position) / (static_cast< float >(view_count));

      view::bond bond(sort_direction);
      bond.prev = &this->top;
      for (auto& pair : this->views) {
        bond.next = &pair.second;

        if (pair.second.is_dying())
          continue;

        this->bonds.push_back(bond);
        bond.prev = bond.next;
      }

      bond.next = &this->bottom;
      this->bonds.push_back(bond);
    }

    std::string string_list::split(std::string const& string) {
      if (this->token_count > 0) {
        std::vector< boost::iterator_range< std::string::const_iterator > > split_ranges;
        boost::split(split_ranges, string, boost::is_any_of(this->separator), boost::token_compress_on);

        std::string splitted = std::string(split_ranges[0].begin(), split_ranges[std::min(this->token_count, split_ranges.size()) - 1].end());
        if (splitted != this->separator)
          splitted += this->separator + "*";

        return splitted;
      } else {
        return string;
      }
    }

    size_t string_list::token_limit() {
      return this->token_count;
    }

    void string_list::set_token_limit(size_t const token_count) {
      this->token_count = token_count;
      this->items.clear();
    }

    void string_list::make_view(std::string const& string) {
      std::string const& view_name = this->split(string);
      auto               result    = this->views.insert(std::make_pair(view_name, view::string(view_name, this->alignment)));

      if (!result.second)
        return;

      auto const& it = result.first;

      view::string const& prev = (it == this->views.begin()) ? this->top : std::prev(it)->second;
      view::string const& next = (std::next(it) == this->views.end()) ? this->bottom : std::next(it)->second;

      it->second.position = (prev.position + next.position) / 2.0f;
    }

    glm::vec2 string_list::get_position(std::string const& string) {
      std::string const& view_name = this->split(string);
      auto const         it        = this->views.find(view_name);

      if (it == this->views.end())
        return this->bottom.position;

      glm::vec2        position  = it->second.position;
      glm::vec2 const& dimension = it->second.dimension;

      switch (this->alignment) {
        case view::alignment::left:
          position.x += dimension.x;
          break;

        case view::alignment::right:
          position.x -= dimension.x;
          break;

        default:
          break;
      }

      return position;
    }

    void string_list::logic(data::timings const& timings) {
      if (timings.is_keyframe) {
        std::set< std::string > splitted_items;
        std::transform(std::begin(this->items), std::end(this->items), std::inserter(splitted_items, std::end(splitted_items)),
                       [=](std::string const& s) { this->make_view(s); return this->split(s); });

        for (auto it = std::begin(this->views), end = std::end(this->views); it != end;) {
          if (splitted_items.find(it->first) == splitted_items.end())
            it->second.kill();

          if (it->second.is_dead())
            this->views.erase(it++);
          else
            ++it;
        }

        this->refresh_bonds();
      }

      for (auto& pair : this->views) {
        pair.second.logic(timings);
      }
      for (auto& bond : this->bonds) {
        bond.logic(timings);
      }
    } // logic

    void string_list::draw(renderer::base& renderer, data::timings const& timings) {
      for (auto& pair : this->views) {
        pair.second.draw(renderer, timings);
      }
    }

  }
}
