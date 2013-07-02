#include "logoprism/data/simulator.hpp"

namespace logoprism {
  namespace data {

    simulator::simulator(double const& simulation_speed, data::duration const& key_frame_duration) :
      simulation_reference_time(data::not_a_date_time),
      simulation_paused(false),
      simulation_speed(simulation_speed),
      key_frame_duration(key_frame_duration / std::max(1.0, simulation_speed)),
      timelapse_duration(data::microseconds(0)),
      skip_duration(data::microseconds(0))
    {}

    data::timings simulator::timings(data::timings const& last_timings) {
      data::timings timings = last_timings;

      // update the speed of the timings
      timings.simulation_speed = this->speed();

      // if the time is not yet set, probably first tick, use the current time
      if (timings.time == data::not_a_date_time)
        timings.time = data::clock::local_time();

      // if the keyframe time is not set, probably first tick, use the current time
      if (timings.keyframe_time == data::not_a_date_time)
        timings.keyframe_time = data::clock::local_time();

      // if the simulation time is not set, probably first tick, use the provided reference point
      if (timings.simulation_time == data::not_a_date_time)
        timings.simulation_time = this->simulation_reference_time;

      // if we have been provided a fixed timelapse, use it, or find the difference between current time and previous time
      if (this->timelapse_duration != data::microseconds(0))
        timings.time += this->timelapse_duration;
      else
        timings.time = data::clock::local_time();

      // if the previous tick had a system time, compute the timelapse and add the skip_timelapse if any
      if (last_timings.time != data::not_a_date_time)
        timings.timelapse = timings.time - last_timings.time + timings.skip_timelapse;
      else
        timings.timelapse = data::microseconds(0);
      timings.skip_timelapse = data::microseconds(0);

      // compute the simulated timelapse from the current speed and update the simulated time
      timings.simulation_timelapse = data::microseconds(static_cast< int64_t >(timings.timelapse.total_microseconds() * this->speed()));
      timings.simulation_time     += timings.simulation_timelapse;

      // compute whether this is a keyframe or not, depending on the configured keyframe duration
      timings.is_keyframe  = (timings.time - timings.keyframe_time) >= (this->key_frame_duration / std::max(1.0, timings.simulation_speed));
      timings.is_keyframe |= last_timings.time >= timings.time;

      // if this is a keyframe, update the keyframe_time
      if (timings.is_keyframe)
        timings.keyframe_time = timings.time;

      return timings;
    }

    double simulator::speed() const {
      return this->simulation_paused ? 0.0 : this->simulation_speed;
    }

    void simulator::set_speed(double const& simulation_speed) {
      if (!this->simulation_paused)
        this->simulation_speed = simulation_speed;
    }

    void simulator::set_simulation_reference_time(data::datetime const& simulation_reference_time) {
      this->simulation_reference_time = simulation_reference_time;
    }

    void simulator::set_timelapse_duration(data::duration const& timelapse_duration) {
      this->timelapse_duration = timelapse_duration;
    }

    void simulator::toggle_pause() {
      this->simulation_paused = !this->simulation_paused;
    }

  }
}
