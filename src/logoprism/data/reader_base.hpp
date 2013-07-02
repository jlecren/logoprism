#ifndef __LOGOPRISM_DATA_READER_BASE_HPP__
#define __LOGOPRISM_DATA_READER_BASE_HPP__

#include "logoprism/config/config.hpp"
#include "logoprism/data/datetime.hpp"
#include "logoprism/data/simulator.hpp"
#include "logoprism/data/request.hpp"
#include "logoprism/data/worker_simulator.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace logoprism {
  namespace data {

    /**
     * Base class for request parsers. Handles the reading, sorting and completion of the requests as well as
     * the inter-thread communication with the display thread.
     */
    struct reader_base {
      public:
        /**
         * Creates a new request reader/parser/completer.
         * @param filename       the log file name to load
         * @param simulator      the time simulator to use
         * @param read_margin    the read margin (how much time we should read ahead of the current simulation time)
         * @param visible_margin the visible margin (how much time ahead we should use when computing visible requests)
         */
        reader_base(std::string const& filename, data::simulator& simulator, data::duration const& read_margin, data::duration const& visible_margin);
        virtual ~reader_base();

        /** starts the reading thread */
        void start();

        /** stops the reading thread, waiting for it to terminate */
        void stop();

        /** get the current fill percentage of the ringbuffer */
        size_t buffering_percentage();

        /** get the currently visible sorted requests */
        data::requests visible_requests(data::timings const& timings);

        /** whether there are no more requests buffered and no more requests in the file */
        bool exhausted();

      protected:
        friend struct request_reader_thread;

        data::simulator&     simulator;
        data::duration const read_margin;
        data::duration       visible_margin;

        data::requests_buffer buffer;
        data::requests        visible;

        std::ifstream          filestream;
        data::worker_simulator worker_simulator;
        bool volatile          reading_thread_running;
        boost::thread          reading_thread;

        data::date_margins read_margins(data::timings const& timings);
        data::date_margins visible_margins(data::timings const& timings);

        data::requests::iterator push(data::requests const& requests);
        data::request            next();
        void                     run();

        virtual data::request parse(std::string const& line) = 0;
    };

  }
}

#endif // ifndef __LOGOPRISM_DATA_READER_BASE_HPP__
