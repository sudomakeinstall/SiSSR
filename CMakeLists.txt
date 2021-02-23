cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

##################
## Set Policies ##
##################

# Double colons must indicate an imported/alias target.
cmake_policy(SET CMP0028 NEW)

# Search prefixes specified by <PackageName>_ROOT.
cmake_policy(SET CMP0074 NEW)
#cmake_policy(SET CMP0071 NEW)

######################
## Project Settings ##
######################

project("dv-sissr" LANGUAGES CXX C)

set(${PROJECT_NAME}_VERSION_MAJOR "2021")
set(${PROJECT_NAME}_VERSION_MINOR "02")
set(${PROJECT_NAME}_VERSION_PATCH "22")
set(${PROJECT_NAME}_VERSION
  "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")

###################
## CMake Modules ##
###################

set(${PROJECT_NAME}_CMAKE_DIR ${${PROJECT_NAME}_SOURCE_DIR}/cmake)
set(CMAKE_MODULE_PATH ${${PROJECT_NAME}_CMAKE_DIR} ${CMAKE_MODULE_PATH})

include(dvInitializeBuildType)
include(dvPreventInSourceBuilds)
include(dvPreventInBuildInstalls)

#############
## Options ##
#############

# Set flags
set(CMAKE_BUILD_TYPE "RelWithDebugInfo" CACHE STRING "build type" FORCE)
set(CMAKE_CXX_STANDARD 17)

##########################
## Third Party Packages ##
##########################

# LAPACKE

find_package(LAPACKE REQUIRED)
include_directories(${LAPACKE_INCLUDE_DIRS})

# Ceres

find_package(Ceres REQUIRED)
include_directories(${CERES_INCLUDE_DIRS})

# ITK

find_package(ITK REQUIRED COMPONENTS
  ITKCommon
  ITKIOImageBase
  ITKIOMeshBase
  ITKIOXML
  ITKMesh
  ITKQuadEdgeMesh
  ITKQuadEdgeMeshFiltering
  ITKRegistrationCommon
  ITKVtkGlue
  ITKImageIO
  ITKMeshIO
  ITKTestKernel
  ITKBinaryMathematicalMorphology
  SubdivisionQuadEdgeMeshFilter
  MeshNoise
  IOMeshSTL
  Cuberille
)

include(${ITK_USE_FILE})

# CGAL

find_package(CGAL QUIET REQUIRED)

# VTK
find_package(VTK REQUIRED
 COMPONENTS
    CommonColor
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

# Boost

find_package(Boost QUIET REQUIRED COMPONENTS
  program_options
)

# dv-commandline-utils

find_package(dv-commandline-utils REQUIRED)

# Set your files and resources here
file(GLOB Srcs "src/*.cxx")
set( Srcs ${Srcs}
          dv-sissr.cxx
          sissrController.cxx
          sissrView.cxx)

set( Hdrs sissrController.h
          sissrView.h)

set( MOC_Hdrs sissrController.h )

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

add_library(dv-sissr-dependencies INTERFACE)
target_link_libraries(dv-sissr-dependencies INTERFACE
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ceres
  dv-commandline-utils::dv-commandline-utils
  CGAL::CGAL
  Boost::program_options
  ${LAPACKE_LIBRARIES}
  )

add_executable(dv-sissr ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})
target_link_libraries(dv-sissr PUBLIC
  dv-sissr-dependencies
)

vtk_module_autoinit(
  TARGETS dv-sissr
  MODULES ${VTK_LIBRARIES}
)

#############
## Testing ##
#############

option(BUILD_TESTING "Build the testing tree." ON)

include(CTest)

if(BUILD_TESTING)

  add_subdirectory(test)

endif()

