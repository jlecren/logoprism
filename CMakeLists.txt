cmake_minimum_required(VERSION 2.8)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

project(logoprism C CXX)
set(LOGOPRISM_LIBDIR ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(7zip REQUIRED)

if(WIN32)
  add_definitions(-DLOGOPRISM_PLATFORM_WIN32)

  set(Boost_USE_STATIC_LIBS ON)
  set(BUILD_SHARED_LIBS OFF)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
  add_definitions(-D_USE_MATH_DEFINES -D_WIN32 -DWIN32)
  set(CMAKE_LINK_LIBRARY_FLAG "")

  include(DownloadPango)
  include(DownloadGlm)
  include(DownloadICU)
  include(DownloadBoost)

  find_package(PkgConfig REQUIRED)
  find_package(GLM REQUIRED)

  enable_language(RC)
  set(LOGOPRISM_RESOURCES share/icons/win32/logoprism.rc)

elseif(APPLE)
  add_definitions(-DLOGOPRISM_PLATFORM_APPLE)

  include(GNUInstallDirs)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Foundation -framework ApplicationServices -framework IOKit -framework Cocoa -framework OpenGL")

  find_package(PkgConfig REQUIRED)
  pkg_check_modules(X11 REQUIRED x11 xi xrandr xxf86vm)

else()
  add_definitions(-DLOGOPRISM_PLATFORM_LINUX)

  include(GNUInstallDirs)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O3 -Wl,-O1 -flto")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -fwhole-program -Wl,-O1 -flto")

  find_package(PkgConfig REQUIRED)
  pkg_check_modules(X11 REQUIRED x11 xi xrandr xxf86vm)

  add_definitions(-DLOGOPRISM_ENABLE_GSTREAMER)
  pkg_check_modules(ENCODER REQUIRED gstreamer-1.0 gstreamer-plugins-base-1.0 gstreamer-app-1.0)
endif()

include(DownloadYaml)

pkg_check_modules(PANGO_CAIRO REQUIRED pangocairo)

find_package(Boost COMPONENTS program_options filesystem system regex thread chrono locale REQUIRED)
find_package(OpenGL REQUIRED)

option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
add_subdirectory(lib/glfw EXCLUDE_FROM_ALL)
include_directories(lib/glfw/include)

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${PANGO_CAIRO_INCLUDE_DIRS}
  ${ENCODER_INCLUDE_DIRS}
  ${YAML_INCLUDE_DIRS}
  ${ICU_INCLUDE_DIRS}
  ${X11_INCLUDE_DIRS}
  include
  src
  lib/glm-0.9.3.4
)
link_directories(
  ${Boost_LIBRARY_DIRS}
  ${PANGO_CAIRO_LIBRARY_DIRS}
  ${ENCODER_LIBRARY_DIRS}
  ${YAML_LIBRARY_DIRS}
  ${ICU_LIBRARY_DIRS}
  ${X11_LIBRARY_DIRS}
)

# -------------------------------------------------------------------------
# logoprism
file(GLOB_RECURSE LOGOPRISM_SOURCES src/*.cpp)
list(REMOVE_ITEM LOGOPRISM_SOURCES ${CMAKE_SOURCE_DIR}/src/vsct_to_raw.cpp)
add_executable(logoprism WIN32
  ${LOGOPRISM_SOURCES}
  ${LOGOPRISM_RESOURCES}
)
target_link_libraries(logoprism
  glfw
  ${OPENGL_LIBRARIES}
  ${Boost_LIBRARIES}
  ${PANGO_CAIRO_LIBRARIES}
  ${ENCODER_LIBRARIES}
  ${YAML_LIBRARIES}
  ${ICU_LIBRARIES}
  ${X11_LIBRARIES}
)

if(CMAKE_HOST_WIN32)
  target_link_libraries(logoprism comdlg32.lib user32.lib)

  add_custom_command(TARGET logoprism POST_BUILD
    COMMAND ${CMAKE_COMMAND} -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}" -DLOGOPRISM_TARGET=$<TARGET_FILE:logoprism> -DLOGOPRISM_TARGET_PATH=$<TARGET_FILE_DIR:logoprism> -P ${CMAKE_SOURCE_DIR}/cmake/CopyPrerequisites.cmake
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/etc/logoprism.conf $<TARGET_FILE_DIR:logoprism>/logoprism.conf
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:logoprism>/lib
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/lib/mesa/opengl32.dll $<TARGET_FILE_DIR:logoprism>/lib/opengl32.dll
  )
endif()

# -------------------------------------------------------------------------
# installation
if(CMAKE_HOST_WIN32)
  install(TARGETS logoprism     RUNTIME DESTINATION bin)
  install(FILES   etc/logoprism.conf    DESTINATION bin)
  install(FILES   lib/mesa/opengl32.dll DESTINATION lib)

  set(LOGOPRISM_DEPENDENCY_DIRS "")
  foreach(LOGOPRISM_DEPENDENCY_DIR ${CMAKE_PREFIX_PATH})
    list(APPEND LOGOPRISM_DEPENDENCY_DIRS ${LOGOPRISM_DEPENDENCY_DIR}/bin/)
    list(APPEND LOGOPRISM_DEPENDENCY_DIRS ${LOGOPRISM_DEPENDENCY_DIR}/lib/)
    list(APPEND LOGOPRISM_DEPENDENCY_DIRS ${LOGOPRISM_DEPENDENCY_DIR}/lib/x86)
  endforeach()

  install(CODE "
    include(BundleUtilities)
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/bin/logoprism.exe\" \"\" \"${LOGOPRISM_DEPENDENCY_DIRS}\")
  ")

  include(InstallRequiredSystemLibraries)

  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LogO'Prism")
  set(CPACK_PACKAGE_VENDOR "Prologism")
  set(CPACK_PACKAGE_VERSION_MAJOR "1")
  set(CPACK_PACKAGE_VERSION_MINOR "0")
  set(CPACK_PACKAGE_VERSION_PATCH "0")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "Logoprism")

  if(WIN32 AND NOT UNIX)
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/share/icons/win32\\\\logoprism-nsis.bmp")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "logoprism.exe")
    set(CPACK_NSIS_DISPLAY_NAME "LogO'Prism")
    set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.prologism.fr")
    set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.prologism.fr")
    set(CPACK_NSIS_CONTACT "contact@prologism.fr")
    set(CPACK_NSIS_MODIFY_PATH ON)
  endif()

  set(CPACK_PACKAGE_EXECUTABLES "logoprism" "LogO'Prism")
  include(CPack)

else()
  if(CMAKE_INSTALL_PREFIX MATCHES ".*/usr/?$")
    set(LOGOPRISM_INSTALL_ETCDIR ../etc)
  else()
    set(LOGOPRISM_INSTALL_ETCDIR etc)
  endif()

  install(TARGETS   logoprism RUNTIME   DESTINATION bin)
  install(FILES     etc/logoprism.conf  DESTINATION ${LOGOPRISM_INSTALL_ETCDIR})
  install(DIRECTORY share/icons/hicolor DESTINATION share/icons)

endif()

