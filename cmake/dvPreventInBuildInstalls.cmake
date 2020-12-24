# This function will prevent in-build installs
#
# Modified from:
# https://github.com/hinerm/ITK/blob/master/CMake/PreventInBuildInstalls.cmake

function(AssureOutOfBuildInstalls)
  # make sure the user doesn't play dirty with symlinks
  get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)
  get_filename_component(installdir "${CMAKE_INSTALL_PREFIX}" REALPATH)

  if ("${bindir}" STREQUAL "${installdir}")
    message("#######################################################################")
    message("# dv-commandline-utils shouldn't be installed in the build directory. #")
    message("#######################################################################")
    message(FATAL_ERROR "Quitting configuration")
  endif()
endfunction()

AssureOutOfBuildInstalls()
