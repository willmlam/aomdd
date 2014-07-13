# ########## Project setup ##########
PROJECT(aomdd)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# ######### General setup ##########
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
    

option(AOMDD_LINK_STATIC "Link binary statically" OFF)

add_definitions(-Wall)

if(WIN32)
  add_definitions(-DWINDOWS)
else()
  add_definitions(-DLINUX)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_RELEASE "-O3")

if(AOMDD_LINK_STATIC)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
    set(Boost_USE_STATIC_LIBS ON)
endif()

find_package( Boost REQUIRED COMPONENTS program_options graph )

include_directories(
  ${AOMDD_SOURCE_DIR}/include
  ${AOMDD_SOURCE_DIR}/lib
)


add_subdirectory(lib)
set(LIBS Cvo)

# ########## src library ##########
# Sources:
SET(src_library_SRCS
    ./src/AOCMGraph.cpp
    ./src/AOMDDFunction.cpp
    ./src/Bucket.cpp
    ./src/BucketTree.cpp
    ./src/CompileBucket.cpp
    ./src/CompileBucketTree.cpp
    ./src/DDMiniBucket.cpp
    ./src/DDMiniBucketTree.cpp
    ./src/Graph.cpp
    ./src/MetaNode.cpp
    ./src/Model.cpp
    ./src/NodeManager.cpp
    ./src/parsers.cpp
    ./src/PseudoTree.cpp
    ./src/Scope.cpp
    ./src/TableFunction.cpp
    ./src/utils.cpp
)

add_executable(aomdd main.cpp ${src_library_SRCS})
target_link_libraries(aomdd ${LIBS} ${Boost_PROGRAM_OPTIONS_LIBRARY})
