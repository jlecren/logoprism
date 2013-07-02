#include "logoprism/application.hpp"
#include "logoprism/utils/signals.hpp"

#include "GLFW/glfw3.h"

#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/filesystem.hpp>

#ifdef _WIN32
# include <windows.h>
#endif // ifdef _WIN32

namespace logoprism {

  static glm::ivec2 detect_fullscreen_dimension(glm::ivec2 const& dimension, bool const offscreen) {
    if (offscreen)
      return dimension;

    glfwInit();

    int                        count       = 0;
    GLFWvidmode const* const   video_modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
    std::vector< GLFWvidmode > modes       = std::vector< GLFWvidmode >(count);
    std::memcpy(modes.data(), video_modes, count * sizeof(GLFWvidmode));

    modes.push_back(*glfwGetVideoMode(glfwGetPrimaryMonitor()));
    if (modes.back().width == 0)
      modes.pop_back();

    // sort by increasing size and decreasing color bits
    std::sort(modes.begin(), modes.end(),
              [](GLFWvidmode const& a, GLFWvidmode const& b) {
                return a.width < b.width || a.height < b.height || a.redBits > b.redBits || a.greenBits > b.greenBits || a.blueBits > b.blueBits;
              });
    auto const mode = *std::find_if(modes.begin(), modes.end(), [&](GLFWvidmode const& m) { return m.width >= dimension.x && m.height >= dimension.y; });

    return glm::ivec2(mode.width, mode.height);
  }

  application::application() :
    offscreen(config::get("display.offscreen")),
    dimension(config::get("display.width"), config::get("display.height")),
    fullscreen_dimension(detect_fullscreen_dimension(this->dimension, this->offscreen)),
    fullscreen(config::get("display.fullscreen")),
    window(NULL) {
    if (this->offscreen)
      return;

    this->open_window(this->fullscreen);
  }

  application::~application() {
    if (this->offscreen)
      return;

    if (this->window)
      glfwDestroyWindow(this->window);

    glfwTerminate();
  }

  void application::run() {
    while (!utils::signals::is_killed()) {
      this->tick();

      if (this->offscreen)
        continue;

      glfwSwapBuffers(this->window);
      glfwPollEvents();
    }

    this->stop();
  }

  void application::toggle_fullscreen() {
    if (this->offscreen)
      return;

    this->pressed_keys.reset();
    this->open_window(!this->fullscreen);
  }

  void application::open_window(bool const fullscreen) {
    if (this->offscreen)
      return;

    this->fullscreen = fullscreen;

    if (this->window)
      glfwDestroyWindow(this->window);

    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);

    if (config::get("display.multisampling"))
      glfwWindowHint(GLFW_SAMPLES, 4);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    if (fullscreen)
      this->window = glfwCreateWindow(this->fullscreen_dimension.x, this->fullscreen_dimension.y, config::get("command.program").c_str(), glfwGetPrimaryMonitor(), NULL);
    else
      this->window = glfwCreateWindow(this->dimension.x, this->dimension.y, config::get("command.program").c_str(), NULL, NULL);

    if (!this->window)
      throw std::runtime_error("Unable to open an OpenGL window, please copy opengl32.dll from the 'lib' folder to where logoprism.exe is located and try again.");
    else
      glfwMakeContextCurrent(this->window);

    glfwSetWindowUserPointer(this->window, static_cast< void* >(this));
    glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwSetKeyCallback(this->window,
                       [](GLFWwindow* window, int key, int, int action, int) {
                         application& self = *static_cast< application* >(glfwGetWindowUserPointer(window));
                         switch (action) {
                           case GLFW_PRESS:
                             switch (key) {
                               case GLFW_KEY_ESCAPE:
                                 utils::signals::kill();
                                 break;

                               default:
                                 self.pressed_keys.set(key);
                                 self.pressed_keys_delay = data::microseconds(0);
                                 break;
                             }
                             break;

                           case GLFW_RELEASE:
                             self.pressed_keys.reset(key);
                             break;
                         }
                       });
    glfwSetWindowCloseCallback(this->window, [](GLFWwindow*) { utils::signals::kill(); });

    glfwSwapInterval(1);
  } // open_window

}
