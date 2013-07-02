set(YAML_VERSION 0.3.0)


set(YAML_ARCHIVE    yaml-cpp-${YAML_VERSION}.tar.gz)
set(YAML_ROOT       ${LOGOPRISM_LIBDIR}/yaml-cpp)
set(YAML_LIBRARYDIR ${YAML_ROOT}/lib/x86)

if(NOT EXISTS ${LOGOPRISM_LIBDIR}/${YAML_ARCHIVE})
  message(STATUS "Downloading YAML dependencies...")
  file(DOWNLOAD
    http://yaml-cpp.googlecode.com/files/${YAML_ARCHIVE}
    ${LOGOPRISM_LIBDIR}/${YAML_ARCHIVE}
    SHOW_PROGRESS
  )
endif()

if(NOT EXISTS ${YAML_ROOT})

  message(STATUS "Extracting YAML dependencies...")
  execute_process(
    COMMAND ${7ZIP_EXECUTABLE} x -y -bd ${LOGOPRISM_LIBDIR}/${YAML_ARCHIVE}
    WORKING_DIRECTORY ${LOGOPRISM_LIBDIR}
  )
  execute_process(
    COMMAND ${7ZIP_EXECUTABLE} x -y -bd ${LOGOPRISM_LIBDIR}/yaml-cpp-${YAML_VERSION}.tar
    WORKING_DIRECTORY ${LOGOPRISM_LIBDIR}
  )
endif()

add_subdirectory(${YAML_ROOT} EXCLUDE_FROM_ALL)
set(YAML_LIBRARIES yaml-cpp)
set(YAML_INCLUDE_DIRS ${YAML_ROOT}/include)
