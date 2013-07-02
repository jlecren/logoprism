#ifndef __LOGOPRISM_DATA_SIMULATOR_HPP__
#define __LOGOPRISM_DATA_SIMULATOR_HPP__

#include "logoprism/data/datetime.hpp"

namespace logoprism {
  namespace data {

    /**
     * Time simulation component
     */
    struct simulator {
      simulator(double const& simulation_speed, data::duration const& key_frame_duration);

      /**
       * Compute the next timings, from the current timing and the current speed settings.
       *
       * @param  last_timings the timings of the previous tick
       * @return              the timings of the current tick
       */
      data::timings timings(data::timings const& last_timings);

      /** @return the current speed factor */
      double speed() const;

      /** sets the current speed factor */
      void set_speed(double const& speed);

      /** sets the current simulated time origin */
      void set_simulation_reference_time(data::datetime const& simulation_reference_time);

      /** sets the current tick fixed duration, for fixed frame rate (video rendering) */
      void set_timelapse_duration(data::duration const& timelapse_duration);

      /** pauses/unpauses the simulation */
      void toggle_pause();

      protected:
        data::datetime simulation_reference_time;
        bool           simulation_paused;
        double         simulation_speed;
        data::duration key_frame_duration;
        data::duration timelapse_duration;
        data::duration skip_duration;

      private:
        simulator(simulator const&);
    };

  }
}

#endif // ifndef __LOGOPRISM_DATA_SIMULATOR_HPP__
