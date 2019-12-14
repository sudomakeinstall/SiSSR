cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)
cmake_policy(SET CMP0071 NEW)

######################
## Project Settings ##
######################

project(SiSSR)

# Set flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-std=c++14 -g -Wall -Wextra -O3 -fno-strict-overflow -ffast-math" CACHE STRING "cxx flags" FORCE)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "build type" FORCE)

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

set(dv-commandline-utils "$ENV{HOME}/Developer/repositories/dv-commandline-utils/")
set(dv-commandline-utils_SRC_DIR "${dv-commandline-utils}src/" CACHE STRING "dv-commandline-utils source Directory")
set(dv-commandline-utils_BIN_DIR "${dv-commandline-utils}bin/" CACHE STRING "dv-commandline-utils binary Directory")

find_library(dv-commandline-utils_LIBRARIES NAME dv-commandline-utils-lib PATHS "${dv-commandline-utils_BIN_DIR}" REQUIRED)

include_directories("${dv-commandline-utils_SRC_DIR}/includes/")

# Set your files and resources here
file(GLOB Srcs "src/*.cxx")
set( Srcs ${Srcs}
          SiSSR.cxx
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

add_executable(SiSSR ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})
target_link_libraries(SiSSR ${QT_LIBRARIES}
                               ${VTK_LIBRARIES}
                               ${ITK_LIBRARIES}
                               ${CERES_LIBRARIES}
                               ${dv-commandline-utils_LIBRARIES}
                     )

vtk_module_autoinit(
  TARGETS SiSSR
  MODULES ${VTK_LIBRARIES}
)
