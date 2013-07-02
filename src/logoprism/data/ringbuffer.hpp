#ifndef __LOGOPRISM_DATA_RINGBUFFER_HPP__
#define __LOGOPRISM_DATA_RINGBUFFER_HPP__

#include <boost/integer/static_log2.hpp>
#include <boost/integer/integer_mask.hpp>
#include <atomic>

namespace logoprism {
  namespace data {

    /**
     * Lock-free ringbuffer, working with power-of-two number of elements and on x86 only, due to memory model issues.
     * Should be written using std::atomic< ... > variables to support other architectures
     */
    template< typename T, size_t const Size = 2048 >
    class ringbuffer {
      protected:
        static size_t const try_item_count_order = boost::static_log2< Size >::value;
        static size_t const try_item_count       = 1 << try_item_count_order;

        static size_t const item_count_order = try_item_count_order + (try_item_count == Size ? 0 : 1);
        static size_t const item_count       = 1 << item_count_order;

        typedef boost::low_bits_mask_t< item_count_order > position_mask_type;
        typedef typename position_mask_type::fast          position_type;

        static position_type const position_mask = position_mask_type::sig_bits_fast;

        struct item_type {
          T             item;
          bool volatile valid;

          item_type() : item(), valid(false) {}
        };

        item_type              items[item_count];
        position_type volatile writer;
        position_type volatile reader;

      public:
        ringbuffer() :
          writer(0),
          reader(0)
        {}

        /**
         * Pushes an item to the ringbuffer.
         * @param  item the value to push
         * @return      wether the value has successfully been pushed or if the buffer was full
         */
        inline bool push(T const& item) {
          position_type const write_position = this->writer & position_mask;
          item_type&          write_item     = this->items[write_position];

          if (write_item.valid)
            return false;

          write_item.item = item;

          // TODO: compiler reordering barrier, replace with c++11 std::atomic variables
#ifdef _WIN32
          _ReadWriteBarrier();
#else // ifdef _WIN32
          __asm__ __volatile__ ("" ::: "memory");
#endif // ifdef _WIN32

          write_item.valid = true;

          this->writer++;
          return true;
        }

        /**
         * Pop an item from the ringbuffer
         * @param  item the value to read
         * @return      whether the value could be read or if the buffer was empty
         */
        inline bool pop(T& item) {
          position_type const read_position = this->reader & position_mask;
          item_type&          read_item     = this->items[read_position];

          if (!read_item.valid)
            return false;

          item = read_item.item;

          // TODO: compiler reordering barrier, replace with c++11 std::atomic variables
#ifdef _WIN32
          _ReadWriteBarrier();
#else // ifdef _WIN32
          __asm__ __volatile__ ("" ::: "memory");
#endif // ifdef _WIN32

          read_item.valid = false;

          this->reader++;
          return true;
        }

        inline size_t size() {
          return this->writer - this->reader;
        }

        inline size_t load() {
          return this->size() * 100 / Size;
        }

    };

  }
}

#endif // ifndef __LOGOPRISM_DATA_RINGBUFFER_HPP__
