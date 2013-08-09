if(MINGW)
  set(GTK_VERSION      2.24)
  set(GTK_VERSION_FULL 2.24.11-1)
  set(GTK_ARCHIVE      gtk+-${GTK_VERSION_FULL}-bundle.7z)
  set(GTK_URL          http://sourceforge.net/projects/gtk-mingw/files/gtk%2B2/${GTK_ARCHIVE}/download)
else(MINGW)
  set(GTK_VERSION      2.24)
  set(GTK_VERSION_FULL 2.24.10-20120208)
  set(GTK_ARCHIVE      gtk+-bundle_${GTK_VERSION_FULL}_win32.zip)
  set(GTK_URL          http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/${GTK_VERSION}/${GTK_ARCHIVE})
endif(MINGW)

set(GTK_ROOT       ${LOGOPRISM_LIBDIR}/gtk+/)
set(GTK_BUNDLE_DIR ${GTK_ROOT})

if(NOT EXISTS ${LOGOPRISM_LIBDIR}/${GTK_ARCHIVE})
  message(STATUS "Downloading GTK+ dependencies...")
  file(DOWNLOAD
    ${GTK_URL}
    ${LOGOPRISM_LIBDIR}/${GTK_ARCHIVE}
    SHOW_PROGRESS
  )
endif()

if(NOT EXISTS ${GTK_ROOT})
  file(MAKE_DIRECTORY ${GTK_ROOT})
  message(STATUS "Extracting GTK+ dependencies...")
  execute_process(
    COMMAND ${7ZIP_EXECUTABLE} x -y -bd ${LOGOPRISM_LIBDIR}/${GTK_ARCHIVE}
    WORKING_DIRECTORY ${GTK_ROOT}
  )
  
  if(NOT MINGW)
    configure_file(${GTK_ROOT}/lib/libpng.lib ${GTK_ROOT}/lib/png14.lib COPYONLY)
    configure_file(${GTK_ROOT}/lib/zdll.lib ${GTK_ROOT}/lib/z.lib COPYONLY)
  endif()
endif()

list(APPEND CMAKE_PREFIX_PATH ${GTK_BUNDLE_DIR})
