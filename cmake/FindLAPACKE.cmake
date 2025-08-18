# FindLAPACKE.cmake
# Finds the LAPACKE library using pkg-config

find_package(PkgConfig REQUIRED)

pkg_check_modules(LAPACKE REQUIRED lapacke)

if(LAPACKE_FOUND)
    set(LAPACKE_INCLUDE_DIRS ${LAPACKE_INCLUDE_DIRS})
    set(LAPACKE_LIBRARIES ${LAPACKE_LIBRARIES})
    set(LAPACKE_LIBRARY_DIRS ${LAPACKE_LIBRARY_DIRS})
    
    # Create imported target
    if(NOT TARGET LAPACKE::LAPACKE)
        add_library(LAPACKE::LAPACKE INTERFACE IMPORTED)
        set_target_properties(LAPACKE::LAPACKE PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${LAPACKE_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${LAPACKE_LIBRARIES}"
            INTERFACE_LINK_DIRECTORIES "${LAPACKE_LIBRARY_DIRS}"
        )
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LAPACKE
    REQUIRED_VARS LAPACKE_LIBRARIES LAPACKE_INCLUDE_DIRS
    VERSION_VAR LAPACKE_VERSION
)