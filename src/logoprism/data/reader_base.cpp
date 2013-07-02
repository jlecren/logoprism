#include "logoprism/data/reader_base.hpp"

#include "logoprism/data/request_reader.hpp"

#include <cmath>
#include <boost/date_time.hpp>

namespace logoprism {
  namespace data {

    /** functor to hold the reading thread */
    struct request_reader_thread {
      request_reader_thread(reader_base* reader) :
        reader(reader)
      {}

      void operator()() {
        try {
          reader->run();
        } catch (std::out_of_range& e) {}
      }

      reader_base* reader;
    };

    reader_base::reader_base(std::string const& filename, data::simulator& simulator, data::duration const& read_margin, data::duration const& visible_margin) :
      simulator(simulator),
      read_margin(read_margin),
      visible_margin(visible_margin),
      filestream(filename),
      worker_simulator(50),
      reading_thread_running(false),
      reading_thread()
    {}

    reader_base::~reader_base() {}

    void reader_base::stop() {
      this->filestream.close();

      if (this->reading_thread_running) {
        this->reading_thread.interrupt();
        this->reading_thread.join();
      }
    }

    data::request reader_base::next() {
      data::request request;

      // find the next valid request, reading lines one by one and parsing them
      do {
        if (!this->filestream.good())
          throw std::out_of_range("end of file");

        std::string line;
        std::getline(this->filestream, line);
        request = this->parse(line);
      } while (!request.valid);

      // complete the request information using the worker simulator
      this->worker_simulator.handle_request(request);

      return request;
    }

    void reader_base::start() {
      this->reading_thread         = boost::thread(request_reader_thread(this));
      this->reading_thread_running = true;
    }

    data::requests::iterator reader_base::push(data::requests const& requests) {
      std::clog << "pushing " << requests.size() << " requests" << std::endl;

      // push as much requests as possible to the ringbuffer and return an interator to where we stopped
      for (auto pushed = requests.begin(), end = requests.end(); pushed != end; ++pushed) {
        if (!this->buffer.push(*pushed))
          return pushed;
      }

      return requests.end();
    }

    void reader_base::run() {
      data::requests requests;

      // parse the first valid request
      auto pair = requests.insert(this->next());

      // set the simulator' reference time to the start time of the first request parsed
      this->simulator.set_simulation_reference_time(pair.first->start_time);

      data::timings timings;
      do {
        if (boost::this_thread::interruption_requested())
          return;

        timings = this->simulator.timings(timings);
        data::date_margins const& margins = this->read_margins(timings);

        // read requests until the start time of the request overflows the read margin, requests are automatically sorted thanks to the std::set
        // ie: a read margin of 10min means we are going to read 10mins ahead of the current simulation time
        try {
          do {
            pair = requests.insert(this->next());
          } while (pair.first->start_time < margins.second);
        } catch (std::out_of_range const& e) {}

        // if the buffer is at least half full, continue reading, we will fill it later
        if (this->buffer.load() >= 50)
          continue;

        // push the requests to the ringbuffer and erase whichever has been successfully pushed
        requests.erase(requests.begin(), this->push(requests));
      } while (this->filestream.good());

      std::cerr << "end of file reached, pushing " << requests.size() << " requests." << std::endl;

      // push the remaining requests to the buffer until there are no more to push
      do {
        if (boost::this_thread::interruption_requested())
          return;

        requests.erase(requests.begin(), this->push(requests));
      } while (!requests.empty());

      this->reading_thread_running = false;
    }

    data::requests reader_base::visible_requests(data::timings const& timings) {
      data::date_margins const& margins = this->visible_margins(timings);

      // remove requests that are not visible anymore, find the first one with a start_time bigger than the visible margin
      auto const& end = std::find_if(this->visible.begin(), this->visible.end(),
                                     [&](data::request const& r) { return r.start_time > margins.first; });

      // for any request that started before this one, check if it is still visible or not
      for (auto it = std::begin(this->visible); it != end;) {
        if (it->start_time + it->duration < margins.first)
          this->visible.erase(it++);
        else
          ++it;
      }

      // read new requests from the buffer, until we have found one that is not yet visible or until the buffer is empty
      while (true) {
        if (!this->visible.empty() && (this->visible.rbegin()->start_time > margins.second))
          break;

        data::request request;
        if (!this->buffer.pop(request))
          break;

        if (request.start_time + request.duration < margins.first)
          continue;

        this->visible.insert(request);
      }

      return this->visible;
    }

    size_t reader_base::buffering_percentage() {
      return this->buffer.load();
    }

    data::date_margins reader_base::read_margins(data::timings const& timings) {
      return std::make_pair(timings.simulation_time - this->read_margin, timings.simulation_time + this->read_margin);
    }

    data::date_margins reader_base::visible_margins(data::timings const& timings) {
      return std::make_pair(timings.simulation_time - this->visible_margin, timings.simulation_time + this->visible_margin);
    }

    bool reader_base::exhausted() {
      return this->reading_thread.timed_join(data::microseconds(0)) && this->buffer.size() == 0;
    }

  }
}
