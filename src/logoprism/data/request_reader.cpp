#include "logoprism/data/request_reader.hpp"
#include "logoprism/data/request.hpp"

#include <boost/algorithm/string/regex.hpp>
#include <boost/lexical_cast.hpp>

#ifdef __APPLE__
#include "logoprism/data/wknd_string_parse_tree.hpp"
#endif

namespace logoprism {
  namespace data {

    request_reader::request_reader(std::string const& filename, data::simulator& simulator, data::duration const& read_margin, data::duration const& visible_margin) :
      reader_base(filename, simulator, read_margin, visible_margin) {
      auto const& formats_config = config::get_child("input.formats");

      // load all the formats from the config file
      for (auto const& node : formats_config) {
        std::string const name = node.second.get("name", "");
        if (name != "") {
          this->regex_map[name]      = boost::regex(node.second.get("regex", ""));
          this->regex_url_map[name]  = boost::regex(node.second.get("regex-url", ""));
          this->regex_date_map[name] = node.second.get("regex-date", "");
          this->resolution_map[name] = data::microseconds(node.second.get("resolution-ns", static_cast< size_t >(1000) * 1000 * 1000) / 1000);
          this->worker_key_map[name] = node.second.get("worker-key", "");
        }
      }

      // select the current format from the known ones
      this->input_format = config::get("input.format", "ges");
      this->regex        = this->regex_map[input_format];
      this->regex_url    = this->regex_url_map[input_format];
      this->regex_date   = this->regex_date_map[input_format];
      this->resolution   = this->resolution_map[input_format];
      this->worker_key   = this->worker_key_map[input_format];

      // configure the date stream to use the given date format
      boost::local_time::local_time_input_facet* input_facet = new boost::local_time::local_time_input_facet();
      this->datestream.imbue(std::locale(this->datestream.getloc(), input_facet));
      input_facet->format(this->regex_date.c_str());
    }

    data::request request_reader::parse(std::string const& line) {
      boost::smatch matches;

      // if the request regex didn't match, return an invalid request */
      if (!boost::regex_match(line, matches, this->regex)) {
        std::clog << "E: didn't match format '" << this->input_format << "': " << line << std::endl;

        return data::request();
      }

      // parse the captured date
      datestream.str(matches["date"]);
      boost::posix_time::ptime datetime;
      datestream >> datetime;

      // create a request and fill the data
      data::request request;
      request.start_time_resolution = this->resolution;
      request.start_time            = datetime;

      if (matches["time-taken-ns"].matched)
        request.duration = data::microseconds(boost::lexical_cast< uint64_t >(matches["time-taken-ns"]) / 1000);
      else if (matches["time-taken-us"].matched)
        request.duration = data::microseconds(boost::lexical_cast< uint64_t >(matches["time-taken-us"]));
      else if (matches["time-taken-ms"].matched)
        request.duration = data::milliseconds(boost::lexical_cast< uint64_t >(matches["time-taken-ms"]));
      else if (matches["time-taken-s"].matched)
        request.duration = data::seconds(boost::lexical_cast< uint64_t >(matches["time-taken-s"]));

      if (matches["bytes"].matched)
        request.size_in_bytes = (matches["bytes"] == "-") ? 0 : boost::lexical_cast< uint32_t >(matches["bytes"]);
      else
        request.size_in_bytes = 512;

      request.source     = matches["host"];
      request.target     = matches["request"];
      request.status     = matches["status"];
      request.keep_alive = matches["keep-alive"] == "+";

      if (this->worker_key != "")
        request.worker = matches[this->worker_key];

      // we have all the data we need, mark the request as valid
      request.valid = true;

      // add padding 0 in IP addresses to have aligned client names
      boost::replace_all_regex(request.source, boost::regex("(\\d+)"), std::string("00\\1"));
      boost::replace_all_regex(request.source, boost::regex("0(\\d{3})"), std::string("\\1"));
      boost::replace_all_regex(request.source, boost::regex("0(\\d{3})"), std::string("\\1"));

      if (boost::regex_match(request.target, matches, this->regex_url))
        request.target = matches["page"];
      else
        std::clog << request.target << std::endl;

      return request;
    } // parse

  }
}
