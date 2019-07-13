cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)
cmake_policy(SET CMP0071 NEW)

######################
## Project Settings ##
######################

project(dv-sissr)

# Tell CMake where ITK and VTK are
#set(EIGEN_INCLUDE_DIR "$ENV{HOME}/Developer/eigen" CACHE STRING "Eigen Include Directory")
#set(Ceres_DIR "$ENV{HOME}/Developer/Ceres/bin" CACHE STRING "Ceres Directory")
#set(ITK_DIR "$ENV{HOME}/Developer/ITK/bin" CACHE STRING "ITK Directory")
#set(VTK_DIR "$ENV{HOME}/Developer/VTK/bin" CACHE STRING "VTK Directory")
#set(RapidJSON_DIR "$ENV{HOME}/Developer/RapidJSON/bin" CACHE STRING "RapidJSON Directory")
#set(DVCppUtils_DIR "$ENV{HOME}/Developer/Bitbucket/DVCppUtils/bin")
#set(DVVTKUtils_DIR "$ENV{HOME}/Developer/Bitbucket/DVVTKUtils/bin")

#set(QT_QMAKE_EXECUTABLE "/usr/bin/qmake" CACHE STRING "QMake Executable")

# Set flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -O3 -fno-strict-overflow -ffast-math" CACHE STRING "cxx flags" FORCE)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "build type" FORCE)
#set(CMAKE_CXX_FLAGS_DEBUG "-g -pg -Wall -Wextra" CACHE STRING "build type" FORCE)

# Ceres
find_package(Ceres REQUIRED)
include_directories(${CERES_INCLUDE_DIRS})

# ITK
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# VTK
find_package(VTK REQUIRED
 COMPONENTS
    CommonCore
    CommonDataModel
    CommonMath
    CommonTransforms
    FiltersCore
    FiltersExtraction
    FiltersGeneral
    FiltersModeling
    FiltersSources
    IOGeometry
    IOImage
    IOLegacy
    ImagingColor
    InteractionImage
    InteractionWidgets
    RenderingAnnotation
    RenderingCore
    RenderingOpenGL2
    RenderingVolumeOpenGL2
    InteractionStyle      # implements VTK::RenderingCore
    RenderingFreeType     # implements VTK::RenderingCore
    GUISupportQt
)

# RapidJSON
find_package(RapidJSON REQUIRED)
include_directories(${RapidJSON_INCLUDE_DIRS})

find_library(dv-commandline-utils_LIBRARIES NAME dv-commandline-utils-lib PATHS ${dv-commandline-utils-dir} REQUIRED)
include_directories("$ENV{HOME}/Developer/dv-commandline-utils/src/includes/")

# Set your files and resources here
file(GLOB Srcs "src/*.cxx")
set( Srcs ${Srcs}
          dv-sissr.cxx
          dvSubdivisionRegistrationController.cxx
          dvSubdivisionRegistrationWindow.cxx)

set( Hdrs dvSubdivisionRegistrationController.h
          dvSubdivisionRegistrationWindow.h)

set( MOC_Hdrs dvSubdivisionRegistrationController.h )

set( UIs QtVTKRenderWindows.ui )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  "includes"
)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(${APPLE})
  find_package(Qt5 COMPONENTS
               Core
               Gui
               Widgets)
else()
  find_package(Qt5 COMPONENTS
               Core
               Gui
               Widgets
               X11Extras)
endif()

# Use what VTK was built with
set(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE} CACHE FILEPATH "")
set(QT_MOC_EXECUTABLE ${VTK_QT_MOC_EXECUTABLE} CACHE FILEPATH "")
set(QT_UIC_EXECUTABLE ${VTK_QT_UIC_EXECUTABLE} CACHE FILEPATH "")
qt5_wrap_ui(UI_Srcs ${UIs})

add_executable(dv-sissr ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})
target_link_libraries(dv-sissr ${QT_LIBRARIES}
                               ${VTK_LIBRARIES}
                               ${ITK_LIBRARIES}
                               ${CERES_LIBRARIES}
                               ${dv-commandline-utils_LIBRARIES}
                     )

vtk_module_autoinit(
  TARGETS dv-sissr
  MODULES ${VTK_LIBRARIES}
)
