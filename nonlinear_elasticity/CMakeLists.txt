##
#  CMake script for the dealii-adapter:
##

# Set the name of the project and target:
SET(TARGET "nonlinear_elasticity")

SET(TARGET_SRC
  ${TARGET}.cc
  )

# To make the dimension switch available via cmake
IF (NOT DEFINED DIM)
        SET(DIM 2)
ENDIF()
ADD_DEFINITIONS(-DDIM=${DIM})

# Change default build type for tutorial cases
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release"
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
MESSAGE(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# To cleanup result files
SET(CLEAN_UP_FILES
  # a custom list of globs, e.g. *.log *.vtk
  *.vtk
)
# Usually, you will not need to modify anything beyond this point...

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

FIND_PACKAGE(deal.II 9.2.0 QUIET
  HINTS ${deal.II_DIR} ${DEAL_II_DIR} ../ ../../ $ENV{DEAL_II_DIR}
  )
IF(NOT ${deal.II_FOUND})
  MESSAGE(FATAL_ERROR "\n"
    "*** Could not locate a (sufficiently recent) version of deal.II. ***\n\n"
    "You may want to either pass a flag -DDEAL_II_DIR=/path/to/deal.II to cmake\n"
    "or set an environment variable \"DEAL_II_DIR\" that contains this path."
    )
ENDIF()

DEAL_II_INITIALIZE_CACHED_VARIABLES()

# Store current source directory
SET(TMP ${CMAKE_SOURCE_DIR})
SET(CMAKE_SOURCE_DIR "..")

# Query git information
DEAL_II_QUERY_GIT_INFORMATION()

# Restore source dir
SET(CMAKE_SOURCE_DIR ${TMP})

PROJECT(${TARGET} LANGUAGES CXX)

DEAL_II_INVOKE_AUTOPILOT()

FIND_PACKAGE(precice REQUIRED)
TARGET_LINK_LIBRARIES(${TARGET} precice::precice)

SET_PROPERTY(TARGET  ${TARGET}  APPEND  PROPERTY  COMPILE_DEFINITIONS
            GIT_BRANCH="${GIT_BRANCH}"
            GIT_REVISION="${GIT_REVISION}"
            GIT_SHORTREV="${GIT_SHORTREV}")

IF(${CMAKE_VERSION} VERSION_GREATER "3.16.0")
TARGET_PRECOMPILE_HEADERS(${TARGET}
                        PRIVATE
                         <deal.II/base/function.h>
                         <deal.II/base/parameter_handler.h>
                         <deal.II/base/quadrature_lib.h>
                         <deal.II/base/quadrature_point_data.h>
                         <deal.II/base/revision.h>
                         <deal.II/base/tensor.h>
                         <deal.II/base/timer.h>
                         <deal.II/base/work_stream.h>
                         <deal.II/dofs/dof_accessor.h>
                         <deal.II/dofs/dof_handler.h>
                         <deal.II/dofs/dof_renumbering.h>
                         <deal.II/dofs/dof_tools.h>
                         <deal.II/fe/fe_q.h>
                         <deal.II/fe/fe_system.h>
                         <deal.II/fe/fe_values.h>
                         <deal.II/fe/mapping_q_eulerian.h>
                         <deal.II/grid/grid_generator.h>
                         <deal.II/grid/grid_refinement.h>
                         <deal.II/grid/grid_tools.h>
                         <deal.II/grid/tria.h>
                         <deal.II/grid/tria_accessor.h>
                         <deal.II/grid/tria_iterator.h>
                         <deal.II/lac/affine_constraints.h>
                         <deal.II/lac/block_sparse_matrix.h>
                         <deal.II/lac/block_vector.h>
                         <deal.II/lac/dynamic_sparsity_pattern.h>
                         <deal.II/lac/full_matrix.h>
                         <deal.II/lac/precondition.h>
                         <deal.II/lac/precondition_selector.h>
                         <deal.II/lac/solver_cg.h>
                         <deal.II/lac/sparse_direct.h>
                         <deal.II/lac/sparse_matrix.h>
                         <deal.II/lac/vector.h>
                         <deal.II/numerics/data_out.h>
                         <deal.II/numerics/error_estimator.h>
                         <deal.II/numerics/matrix_tools.h>
                         <deal.II/numerics/vector_tools.h>
                         <deal.II/physics/elasticity/kinematics.h>
                         <deal.II/physics/elasticity/standard_tensors.h>
                         <deal.II/physics/transformations.h>
                         <fstream>)
MESSAGE(STATUS "Using precompiled header files")
ENDIF()
