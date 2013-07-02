set(GLM_VERSION 0.9.3.4)


set(GLM_ARCHIVE     glm-${GLM_VERSION}.zip)
set(GLM_ROOT        ${LOGOPRISM_LIBDIR}/glm-${GLM_VERSION})
set(GLM_PACKAGE_DIR ${GLM_ROOT})

if(NOT EXISTS ${LOGOPRISM_LIBDIR}/${GLM_ARCHIVE})
  message(STATUS "Downloading GLM dependencies...")
  file(DOWNLOAD
    http://sourceforge.net/projects/ogl-math/files/glm-${GLM_VERSION}/${GLM_ARCHIVE}/download
    ${LOGOPRISM_LIBDIR}/${GLM_ARCHIVE}
    SHOW_PROGRESS
  )
endif()

if(NOT EXISTS ${GLM_ROOT})
  
  message(STATUS "Extracting GLM dependencies...")
  execute_process(
    COMMAND ${7ZIP_EXECUTABLE} x -y -bd ${LOGOPRISM_LIBDIR}/${GLM_ARCHIVE}
    WORKING_DIRECTORY ${LOGOPRISM_LIBDIR}
  )
endif()

list(APPEND CMAKE_MODULE_PATH ${GLM_PACKAGE_DIR}/util)
find_package(GLM REQUIRED)
