#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SDL3_mixer::SDL3_mixer-static" for configuration "RelWithDebInfo"
set_property(TARGET SDL3_mixer::SDL3_mixer-static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(SDL3_mixer::SDL3_mixer-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libSDL3_mixer.a"
  )

list(APPEND _cmake_import_check_targets SDL3_mixer::SDL3_mixer-static )
list(APPEND _cmake_import_check_files_for_SDL3_mixer::SDL3_mixer-static "${_IMPORT_PREFIX}/lib/libSDL3_mixer.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
