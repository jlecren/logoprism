set(SDL_VERSION 1.2.15)


set(SDL_ARCHIVE    SDL-devel-${SDL_VERSION}-VC.zip)
set(SDL_ROOT       ${LOGOPRISM_LIBDIR}/SDL-${SDL_VERSION})
set(SDL_LIBRARYDIR ${SDL_ROOT}/lib/x86)

if(NOT EXISTS ${LOGOPRISM_LIBDIR}/${SDL_ARCHIVE})
  message(STATUS "Downloading SDL dependencies...")
  file(DOWNLOAD
    http://www.libsdl.org/release/${SDL_ARCHIVE}
    ${LOGOPRISM_LIBDIR}/${SDL_ARCHIVE}
    SHOW_PROGRESS
  )
endif()

if(NOT EXISTS ${SDL_ROOT})

  message(STATUS "Extracting SDL dependencies...")
  execute_process(
    COMMAND ${7ZIP_EXECUTABLE} x -y -bd ${LOGOPRISM_LIBDIR}/${SDL_ARCHIVE}
    WORKING_DIRECTORY ${LOGOPRISM_LIBDIR}
  )
endif()

list(APPEND CMAKE_PREFIX_PATH ${SDL_ROOT})
list(APPEND CMAKE_LIBRARY_PATH ${SDL_LIBRARYDIR})
