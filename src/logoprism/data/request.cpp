#include "logoprism/data/request.hpp"

namespace logoprism {
  namespace data {

    size_t request::request_sequence = 0;

    request::request() :
      valid(false),
      sequence(++data::request::request_sequence)
    {}

    request::request(request const& o) :
      valid(o.valid),
      sequence(o.sequence),
      start_time(o.start_time),
      start_time_resolution(o.start_time_resolution),
      duration(o.duration),
      size_in_bytes(o.size_in_bytes),
      source(),
      target(),
      status(),
      worker(),
      keep_alive(o.keep_alive) {
      this->source.assign(o.source.begin(), o.source.end());
      this->target.assign(o.target.begin(), o.target.end());
      this->status.assign(o.status.begin(), o.status.end());
      this->worker.assign(o.worker.begin(), o.worker.end());
    }

    request& request::operator=(request const& o) {
      this->valid                 = o.valid;
      this->sequence              = o.sequence;
      this->start_time            = o.start_time;
      this->start_time_resolution = o.start_time_resolution;
      this->duration              = o.duration;
      this->size_in_bytes         = o.size_in_bytes;
      this->keep_alive            = o.keep_alive;
      this->source.assign(o.source.begin(), o.source.end());
      this->target.assign(o.target.begin(), o.target.end());
      this->status.assign(o.status.begin(), o.status.end());
      this->worker.assign(o.worker.begin(), o.worker.end());

      return *this;
    }

    bool request::operator<(data::request const& other) const {
      return this->valid && this->start_time < other.start_time;
    }

    bool request::operator==(data::request const& other) const {
      return this->sequence == other.sequence;
    }

  }
}
