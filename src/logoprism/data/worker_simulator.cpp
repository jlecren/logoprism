#include "logoprism/data/worker_simulator.hpp"

namespace logoprism {
  namespace data {
    
    worker_simulator::worker_simulator(size_t const worker_count) :
      worker_count(worker_count),
      worker_last_index(0) {

      // creates as much workers as configured, with invalid end dates
      for (size_t worker_id = 0; worker_id < worker_count; ++worker_id) {
        this->worker_end_times[worker_id] = boost::date_time::not_a_date_time;
      }
    }

    void worker_simulator::handle_request(data::request& request) {
      std::stringstream worker_text;

      // if the request does not already have a worker name, choose it dynamically
      if (request.worker.size() == 0) {

        // try to find an existing worker with an end time matching, at least,
        // the same time slice as the request start time.
        for (auto& worker_pair : this->worker_end_times) {
          if (worker_pair.second == boost::date_time::not_a_date_time)
            worker_pair.second = request.start_time;

          if (worker_pair.second < request.start_time + request.start_time_resolution) {
            worker_pair.second = std::max(worker_pair.second, request.start_time);

            request.start_time  = worker_pair.second;
            worker_pair.second += request.duration + data::microseconds(1000);

            worker_text.str("");
            worker_text << "Worker#" << std::setfill('0') << std::setw(3) << worker_pair.first;
            request.worker = worker_text.str();

            break;
          }
        }

        // if no free worker was found, create a new one specifically for this request
        if (request.worker.size() == 0) {
          this->worker_end_times[this->worker_count] = request.start_time + request.duration;

          worker_text.str("");
          worker_text << "Worker#" << std::setfill('0') << std::setw(3) << this->worker_count;
          request.worker = worker_text.str();

          this->worker_names[request.worker] = this->worker_count;
          this->worker_count++;
        }

      } else {
        // otherwise, update the request start time using the known worker end time for the worker
        // assigned to the request
        if (this->worker_names.find(request.worker) == this->worker_names.end()) {
          this->worker_names[request.worker] = this->worker_count;
          this->worker_count++;
        }

        size_t worker_id = this->worker_names[request.worker];
        if (this->worker_end_times.find(worker_id) != this->worker_end_times.end())
          request.start_time = this->worker_end_times[worker_id];

        this->worker_end_times[worker_id] = request.start_time + request.duration;
      }
    } // handle_request

  }
}
