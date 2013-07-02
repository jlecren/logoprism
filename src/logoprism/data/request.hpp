#ifndef __LOGOPRISM_DATA_REQUEST_HPP__
#define __LOGOPRISM_DATA_REQUEST_HPP__

#include "logoprism/data/datetime.hpp"
#include "logoprism/data/ringbuffer.hpp"

#include <set>

namespace logoprism {
  namespace data {

    /**
     * The internal representation of a request
     */
    struct request {
      /** internal counter to uniquely identify the requests */
      static size_t request_sequence;

      request();
      request(request const& o);
      request& operator=(request const& o);

      /** whether the request has been successfully parsed */
      bool valid;

      /** the unique identifier of this request */
      size_t sequence;

      /** the simulated time the request starts at */
      data::datetime start_time;

      /** the precision of the start time of the request */
      data::duration start_time_resolution;

      /** the duration of the request */
      data::duration duration;

      /** the size of the response sent with this request, in bytes */
      uint64_t size_in_bytes;

      /** the name of the source (client ip) */
      std::string source;

      /** the name of the target (server url) */
      std::string target;

      /** the request status code */
      std::string status;

      /** the name of the worker that has handled the request */
      std::string worker;

      /** whether the request is flagged with keep-alive */
      bool keep_alive;

      /** comparison operators to sort requests by their starting time */
      bool operator<(data::request const& other) const;
      bool operator==(data::request const& other) const;
    };

    typedef std::set< data::request >                  requests;
    typedef data::ringbuffer< data::request, 1000000 > requests_buffer;

    template< typename T >
    static inline std::basic_ostream< T >& operator<<(std::basic_ostream< T >& stream, data::request const& request) {
      return stream << "request {"
                    << " start: " << request.start_time << ","
                    << " end: " << (request.start_time + request.duration) << ","
                    << " duration: " << request.duration << ","
                    << " source: " << request.source << ","
                    << " worker: " << request.worker << ","
                    << " size: " << request.size_in_bytes << ","
                    << " keep_alive: " << request.keep_alive
                    << " }";
    }

  }
}

#endif // ifndef __LOGOPRISM_DATA_REQUEST_HPP__
