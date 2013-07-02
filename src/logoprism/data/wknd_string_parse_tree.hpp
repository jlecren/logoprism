#ifndef WKND_BOOST_DATE_TIME_STRING_PARSE_TREE___HPP__
#define WKND_BOOST_DATE_TIME_STRING_PARSE_TREE___HPP__

#include "boost/lexical_cast.hpp" //error without?
#include "boost/algorithm/string/case_conv.hpp"
#include "boost/shared_ptr.hpp"
#include <map>
#include <string>
#include <vector>
#include <algorithm>

namespace boost { namespace date_time {

//! Recursive data structure to allow efficient parsing of various strings
/*! This class provides a quick lookup by building what amounts to a
 *  tree data structure.  It also features a match function which can
 *  can handle nasty input interators by caching values as it recurses
 *  the tree so that it can backtrack as needed.
 */
template< >
struct string_parse_tree< char >
{
#if BOOST_WORKAROUND( __BORLANDC__, BOOST_TESTED_AT(0x581) )
  typedef boost::shared_ptr<string_parse_tree< char> > subtree; 
#else
  typedef boost::shared_ptr<string_parse_tree> subtree; 
#endif
  typedef std::multimap<char, subtree> ptree_coll; 
  typedef typename ptree_coll::value_type value_type;
  typedef typename ptree_coll::iterator iterator;
  typedef typename ptree_coll::const_iterator const_iterator;
  typedef std::basic_string<char> string_type;
  typedef std::vector<std::basic_string<char> > collection_type;
  typedef parse_match_result<char> parse_match_result_type;

  /*! Parameter "starting_point" designates where the numbering begins.
   * A starting_point of zero will start the numbering at zero
   * (Sun=0, Mon=1, ...) were a starting_point of one starts the
   * numbering at one (Jan=1, Feb=2, ...). The default is zero,
   * negative vaules are not allowed */
  string_parse_tree(collection_type names, unsigned int starting_point=0)
  {
    // iterate thru all the elements and build the tree
    unsigned short index = 0;
    while (index != names.size() ) {
      string_type s = boost::algorithm::to_lower_copy(names[index]);
      insert(s, static_cast<unsigned short>(index + starting_point));
      index++;
    }
    //set the last tree node = index+1  indicating a value
    index++;
  }


  string_parse_tree(short value = -1) :
    m_value(value)
  {}
  ptree_coll m_next_chars;
  short m_value;

  void insert(const string_type& s, unsigned short value)
  {
    unsigned int i = 0;
    iterator ti;
    while(i < s.size()) {
      if (i==0) {
        if (i == (s.size()-1)) {
          subtree st(new typename subtree::element_type(value)); 
          ti = m_next_chars.insert(value_type(s[i], st)); 
        }
        else {
          subtree st(new typename subtree::element_type()); 
          ti = m_next_chars.insert(value_type(s[i], st)); 
        }
      }
      else {
        if (i == (s.size()-1)) { 
          subtree st(new typename subtree::element_type(value)); 
          ti = ti->second->m_next_chars.insert(value_type(s[i], st)); 
        }

        else {
          subtree st(new typename subtree::element_type()); 
          ti = ti->second->m_next_chars.insert(value_type(s[i], st)); 
        }

      }
      i++;
    }
  }


  //! Recursive function that finds a matching string in the tree.
  /*! Must check match_results::has_remaining() after match() is
   * called. This is required so the user can determine if
   * stream iterator is already pointing to the expected
   * character or not (match() might advance sitr to next char in stream).
   *
   * A parse_match_result that has been returned from a failed match
   * attempt can be sent in to the match function of a different
   * string_parse_tree to attempt a match there. Use the iterators
   * for the partially consumed stream, the parse_match_result object,
   * and '0' for the level parameter. */
  short
  match(std::istreambuf_iterator<char>& sitr,
        std::istreambuf_iterator<char>& stream_end,
        parse_match_result_type& result,
        unsigned int& level)  const
  {

    level++;
    char c;
    // if we conditionally advance sitr, we won't have
    // to consume the next character past the input
    bool adv_itr = true;
    if (level > result.cache.size()) {
      if (sitr == stream_end) return 0; //bail - input exhausted
      c = static_cast<char>(std::tolower(*sitr));
      //result.cache += c;
      //sitr++;
    }
    else {
      // if we're looking for characters from the cache,
      // we don't want to increment sitr
      adv_itr = false;
      c = static_cast<char>(std::tolower(result.cache[level-1]));
    }
    const_iterator litr = m_next_chars.lower_bound(c);
    const_iterator uitr = m_next_chars.upper_bound(c);
    while (litr != uitr) { // equal if not found
      if(adv_itr) {
        sitr++;
        result.cache += c;
      }
      if (litr->second->m_value != -1) { // -1 is default value
        if (result.match_depth < level) {
          result.current_match = litr->second->m_value;
          result.match_depth = static_cast<unsigned short>(level);
        }
        litr->second->match(sitr, stream_end,
                           result, level);
        level--;
      }
      else {
        litr->second->match(sitr, stream_end,
                           result, level);
        level--;
      }

      if(level <= result.cache.size()) {
        adv_itr = false;
      }

      litr++;
    }
    return result.current_match;

  }

  /*! Must check match_results::has_remaining() after match() is
   * called. This is required so the user can determine if
   * stream iterator is already pointing to the expected
   * character or not (match() might advance sitr to next char in stream).
   */
  parse_match_result_type
  match(std::istreambuf_iterator<char>& sitr,
        std::istreambuf_iterator<char>& stream_end) const
  {
    // lookup to_lower of char in tree.
    unsigned int level = 0;
    //    string_type cache;
    parse_match_result_type result;
    match(sitr, stream_end, result, level);
    return result;
  }

  void printme(std::ostream& os, int& level)
  {
    level++;
    iterator itr = m_next_chars.begin();
    iterator end = m_next_chars.end();
    //    os << "starting level: " << level << std::endl;
    while (itr != end) {
      os << "level:  " << level
         << " node:  " << itr->first
         << " value: " << itr->second->m_value
         << std::endl;
      itr->second->printme(os, level);
      itr++;
    }
    level--;
  }

  void print(std::ostream& os)
  {
    int level = 0;
    printme(os, level);
  }

  void printmatch(std::ostream& os, char c)
  {
    iterator litr = m_next_chars.lower_bound(c);
    iterator uitr = m_next_chars.upper_bound(c);
    os << "matches for: " << c << std::endl;
    while (litr != uitr) {
      os << " node:  " << litr->first
         << " value: " << litr->second->m_value
         << std::endl;
      litr++;
    }
  }

};

} } //namespace
#endif
