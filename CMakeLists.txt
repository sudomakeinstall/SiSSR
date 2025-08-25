cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

##################
## Set Policies ##
##################

# Double colons must indicate an imported/alias target.
cmake_policy(SET CMP0028 NEW)

# Search prefixes specified by <PackageName>_ROOT.
cmake_policy(SET CMP0074 NEW)

######################
## Project Settings ##
######################

project("dv-sissr" LANGUAGES CXX C)

set(${PROJECT_NAME}_VERSION_MAJOR "2025")
set(${PROJECT_NAME}_VERSION_MINOR "08")
set(${PROJECT_NAME}_VERSION_PATCH "24")
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
  ITKImageIO
  ITKMeshIO
  ITKTestKernel
  SubdivisionQuadEdgeMeshFilter
  IOMeshSTL
)

include(${ITK_USE_FILE})

# RapidJSON
find_package(RapidJSON REQUIRED)
include_directories(${RapidJSON_INCLUDE_DIRS})

# Boost
find_package(Boost QUIET REQUIRED COMPONENTS
  program_options
)


# Set your files and resources here
file(GLOB Srcs "src/*.cxx")
set( Srcs ${Srcs}
          dv-sissr.cxx)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  "includes"
)

add_library(dv-sissr-dependencies INTERFACE)
target_link_libraries(dv-sissr-dependencies INTERFACE
  ${ITK_LIBRARIES}
  ceres
  Boost::program_options
  ${LAPACKE_LIBRARIES}
  )

add_executable(dv-sissr ${Srcs})
target_link_libraries(dv-sissr PUBLIC
  dv-sissr-dependencies
)

#############
## Testing ##
#############

option(BUILD_TESTING "Build the testing tree." ON)

include(CTest)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
