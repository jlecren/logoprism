/**
 * @file
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __LOGOPRISM_DATA_WORKER_SIMULATOR_HPP__
#define __LOGOPRISM_DATA_WORKER_SIMULATOR_HPP__

#include "logoprism/data/types.hpp"
#include "logoprism/data/request.hpp"

namespace logoprism {
  namespace data {

    /**
     * Component to complete request information, assigning the requests to the first free worker found,
     * given the precision of the start time of the requests.
     *
     * This splits the simulated time accordingly to the start time precision, and will try to fill
     * any request starting in a given time slice to any worker which has enough free time in this slice.
     *
     * If no worker is found, a new one is allocated and the current request is assigned to it.
     */
    struct worker_simulator {
      public:
        worker_simulator(size_t worker_count=0);

        void handle_request(data::request& request);

      protected:
        size_t                               worker_count;
        size_t                               worker_last_index;
        std::map< std::string, uint64_t >    worker_names;
        std::map< uint64_t, data::datetime > worker_end_times;
    };

  }
}

#endif // ifndef __LOGOPRISM_DATA_WORKER_SIMULATOR_HPP__
