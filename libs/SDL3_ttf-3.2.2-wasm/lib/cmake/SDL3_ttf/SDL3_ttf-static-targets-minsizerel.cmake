#----------------------------------------------------------------
# Generated CMake target import file for configuration "MinSizeRel".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SDL3_ttf::SDL3_ttf-static" for configuration "MinSizeRel"
set_property(TARGET SDL3_ttf::SDL3_ttf-static APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(SDL3_ttf::SDL3_ttf-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "C"
  IMPORTED_LOCATION_MINSIZEREL "${_IMPORT_PREFIX}/lib/libSDL3_ttf.a"
  )

list(APPEND _cmake_import_check_targets SDL3_ttf::SDL3_ttf-static )
list(APPEND _cmake_import_check_files_for_SDL3_ttf::SDL3_ttf-static "${_IMPORT_PREFIX}/lib/libSDL3_ttf.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
