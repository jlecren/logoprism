#ifndef __LOGOPRISM_DATA_REQUEST_READER_GENERIC_HPP__
#define __LOGOPRISM_DATA_REQUEST_READER_GENERIC_HPP__

#include "logoprism/config/config.hpp"
#include "logoprism/data/request.hpp"
#include "logoprism/data/reader_base.hpp"

#include <sstream>
#include <map>
#include <boost/regex.hpp>

namespace logoprism {
  namespace data {

    /**
     * Request parser implementation, loading formats from the config file and parsing the requests using the selected format.
     */
    struct request_reader : public reader_base {
      public:
        request_reader(std::string const& filename, data::simulator& simulator, data::duration const& read_margin, data::duration const& visible_margin);

        /** parses a line a return a request, possibly with valid == false if the parsing failed */
        data::request parse(std::string const& line);

      protected:
        /** stringstream to parse formatted dates accordingly to some given locale */
        std::stringstream datestream;

        /** map of all known request regexes, key is the format id */
        std::map< std::string, boost::regex > regex_map;

        /** map of all known URL regexes, key is the format id */
        std::map< std::string, boost::regex > regex_url_map;

        /** map of all known date formats, key is the format id */
        std::map< std::string, std::string > regex_date_map;

        /** map of all known start time resolutions, key is the format id */
        std::map< std::string, data::duration > resolution_map;

        /** map of all known worker keys, key is the format id */
        std::map< std::string, std::string > worker_key_map;

        /** the selected format to use when parsing the requests */
        std::string input_format;

        /** the current request regex */
        boost::regex regex;

        /** the current URL regex */
        boost::regex regex_url;

        /** the current date format */
        std::string regex_date;

        /** the current start time resolution */
        data::duration resolution;

        /** the current worker key */
        std::string worker_key;
    };

  }
}

#endif // ifndef __LOGOPRISM_DATA_REQUEST_READER_GENERIC_HPP__
