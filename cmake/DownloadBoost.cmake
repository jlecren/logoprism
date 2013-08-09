set(BOOST_VERSION 1.54.0)
string(REPLACE "." "_" BOOST_VERSION_STRING "${BOOST_VERSION}")

if(DEFINED BOOST_ROOT)
  if(NOT DEFINED BOOST_LIBRARYDIR)
    set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib)
  endif()

  return()
endif()

set(BOOST_ARCHIVE    boost_${BOOST_VERSION_STRING}.7z)
set(BOOST_ROOT       ${LOGOPRISM_LIBDIR}/boost_${BOOST_VERSION_STRING})
set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib)

if(NOT EXISTS ${LOGOPRISM_LIBDIR}/${BOOST_ARCHIVE})
  message(STATUS "Downloading Boost dependencies...")
  file(DOWNLOAD
    http://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/${BOOST_ARCHIVE}/download
    ${LOGOPRISM_LIBDIR}/${BOOST_ARCHIVE}
    SHOW_PROGRESS
  )
endif()

if(NOT EXISTS ${BOOST_ROOT})

  message(STATUS "Extracting Boost dependencies...")
  execute_process(
    COMMAND ${7ZIP_EXECUTABLE} x -y -bd ${LOGOPRISM_LIBDIR}/${BOOST_ARCHIVE}
    WORKING_DIRECTORY ${LOGOPRISM_LIBDIR}
  )
endif()

if(NOT EXISTS ${BOOST_LIBRARYDIR})
  message(STATUS "Building Boost dependencies...")
  if(MINGW)
    set(BOOST_BOOTSTRAP_OPTIONS    mingw)
    set(BOOST_TOOLSET              "toolset=gcc")
  else(MINGW)
    set(BOOST_BOOTSTRAP_OPTIONS    "")
    set(BOOST_TOOLSET              "")
  endif(MINGW)
  
  execute_process(
    COMMAND bootstrap.bat ${BOOST_BOOTSTRAP_OPTIONS}
    WORKING_DIRECTORY ${BOOST_ROOT}
  )
  execute_process(
    COMMAND b2 -sICU_PATH=${ICU_ROOT} -j 4 ${BOOST_TOOLSET} boost.locale.icu=on debug release link=static --with-program_options --with-filesystem --with-system --with-regex 
            --with-thread --with-chrono --with-date_time --with-locale
    WORKING_DIRECTORY ${BOOST_ROOT}
  )
endif()
