#ifndef __LOGOPRISM_APPLICATION_HPP__
#define __LOGOPRISM_APPLICATION_HPP__

#include "logoprism/config/config.hpp"
#include "logoprism/data/types.hpp"
#include "logoprism/data/datetime.hpp"
#include "logoprism/input/keys.hpp"

struct GLFWwindow;

namespace logoprism {

  struct application {
    public:
      application();
      virtual ~application();

      void         run();
      virtual void tick() = 0;
      virtual void stop() {}

    protected:
      input::keyset  pressed_keys;
      data::duration pressed_keys_delay;

      bool const offscreen;
      void       toggle_fullscreen();

    private:
      void open_window(bool const fullscreen);

      glm::ivec2 const dimension;
      glm::ivec2 const fullscreen_dimension;

      bool  fullscreen;
      GLFWwindow* window;
  };

}

#endif // ifndef __LOGOPRISM_APPLICATION_HPP__
