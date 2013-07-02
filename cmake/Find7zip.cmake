set(7ZIP_VERSION_STRING 1)

find_program(7ZIP_EXECUTABLE NAMES 7z DOC "7z executable" PATHS $ENV{ProgramW6432}/7-Zip $ENV{ProgramFiles}/7-Zip)
mark_as_advanced(7ZIP_EXECUTABLE)

if(NOT 7ZIP_EXECUTABLE)
  message(FATAL "7Zip was not found.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(7zip REQUIRED_VARS 7ZIP_EXECUTABLE VERSION_VAR 7ZIP_VERSION_STRING)
