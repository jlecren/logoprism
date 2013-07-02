#ifndef __LOGOPRISM_DATA_DATETIME_HPP__
#define __LOGOPRISM_DATA_DATETIME_HPP__

#include <boost/date_time.hpp>

namespace logoprism {
  namespace data {

    /**
     * Some date/time imports from boost::date_time and helper functions.
     */

    using boost::posix_time::microseconds;
    using boost::posix_time::milliseconds;
    using boost::posix_time::seconds;
    using boost::date_time::not_a_date_time;

    typedef boost::posix_time::microsec_clock clock;

    typedef boost::posix_time::ptime         datetime;
    typedef boost::posix_time::time_duration duration;

    static data::datetime const epoch      = boost::posix_time::ptime(boost::gregorian::date(2000, 1, 1));
    static data::datetime const unix_epoch = boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1));

    typedef std::pair< data::datetime, data::datetime > date_margins;

    /**
     * Converts a duration to the corresponding number of seconds, in floating point
     * @param  duration the duration to convert
     * @return          the number of seconds, as a double
     */
    static inline double floating_seconds(data::duration const& duration) {
      return duration.total_microseconds() / 1000000.0;
    }

    /**
     * Aggregation of various timings representing a moment of the life of logoprism
     */
    struct timings {
      /** the current system time */
      data::datetime time;

      /** the elapsed system time since the previous tick */
      data::duration timelapse;

      /** the current simulated time */
      data::datetime simulation_time;

      /** the elapsed simulated time since the previous tick */
      data::duration simulation_timelapse;

      /** the current simulated time flow speed */
      double simulation_speed;

      /** the system time of the last keyframe */
      data::datetime keyframe_time;

      /** whether this tick is a keyframe or not */
      bool is_keyframe;

      /** the system time to skip at next simulation time calculation */
      data::duration skip_timelapse;
    };

    template< typename T >
    static inline std::basic_ostream< T >& operator<<(std::basic_ostream< T >& stream, data::timings const& timings) {
      return stream << "timings {"
                    << " time: " << timings.time << ","
                    << " timelapse: " << timings.timelapse << ","
                    << " simulation_time: " << timings.simulation_time << ","
                    << " simulation_timelapse: " << timings.simulation_timelapse << ","
                    << " simulation_speed: " << timings.simulation_speed << ","
                    << " keyframe_time: " << timings.keyframe_time << ","
                    << " is_keyframe: " << timings.is_keyframe
                    << " }";
    }

  }
}

#endif // ifndef __LOGOPRISM_DATA_DATETIME_HPP__
