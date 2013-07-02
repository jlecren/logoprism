set(ICU_VERSION 50_1-Win32)


set(ICU_ARCHIVE    icu4c-${ICU_VERSION}-msvc10.zip)
set(ICU_ROOT       ${LOGOPRISM_LIBDIR}/icu/)
set(ICU_LIBRARYDIR ${ICU_ROOT}/lib/)

if(NOT EXISTS ${LOGOPRISM_LIBDIR}/${ICU_ARCHIVE})
  message(STATUS "Downloading ICU dependencies...")
  file(DOWNLOAD
    http://download.icu-project.org/files/icu4c/50.1/${ICU_ARCHIVE}
    ${LOGOPRISM_LIBDIR}/${ICU_ARCHIVE}
    SHOW_PROGRESS
  )
endif()

if(NOT EXISTS ${ICU_ROOT})

  message(STATUS "Extracting ICU dependencies...")
  execute_process(
    COMMAND ${7ZIP_EXECUTABLE} x -y -bd ${LOGOPRISM_LIBDIR}/${ICU_ARCHIVE}
    WORKING_DIRECTORY ${LOGOPRISM_LIBDIR}
  )
  
  # Pretend to have debug version of the libraries to satisfy boost configuration
  configure_file(${ICU_LIBRARYDIR}/icuin.lib ${ICU_LIBRARYDIR}/icuind.lib COPYONLY)
  configure_file(${ICU_LIBRARYDIR}/icuin.exp ${ICU_LIBRARYDIR}/icuind.exp COPYONLY)
  configure_file(${ICU_LIBRARYDIR}/icuuc.lib ${ICU_LIBRARYDIR}/icuucd.lib COPYONLY)
  configure_file(${ICU_LIBRARYDIR}/icuuc.exp ${ICU_LIBRARYDIR}/icuucd.exp COPYONLY)
endif()

list(APPEND CMAKE_PREFIX_PATH ${ICU_ROOT})
list(APPEND CMAKE_LIBRARY_PATH ${ICU_LIBRARYDIR})
set(ICU_INCLUDE_DIRS ${ICU_ROOT}/include/)
set(ICU_LIBRARY_DIRS ${ICU_LIBRARYDIR})
set(ICU_LIBRARIES icudt icuin icuio icule iculx icutu icuuc)
