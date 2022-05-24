cmake_minimum_required( VERSION 2.8 )
project(disjoint_path)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -g -rdynamic")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall")
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})

# add_executable
set(DATASET_FILE_LIST
    dataset.cpp
    Graph.cpp
    Graph.h
    DataSetGraph.cpp
    DataSetGraph.h
    tools.cpp
    tools.h
    config.h
)
add_executable(dataset ${DATASET_FILE_LIST})

set(TEST_FILE_LIST
    test.cpp
    TwoConnected.cpp
    TwoConnected.h
    BoostTools.h
    BoostTools.cpp
    Graph.cpp
    Graph.h
    DataSetGraph.cpp
    DataSetGraph.h
    tools.cpp
    tools.h
    config.h
)
add_executable(test ${TEST_FILE_LIST})

set(MAIN_FILE_LIST
    main.cpp
    TwoConnected.cpp
    TwoConnected.h
    BoostTools.h
    BoostTools.cpp
    Graph.cpp
    Graph.h
    DataSetGraph.cpp
    DataSetGraph.h
    tools.cpp
    tools.h
    config.h
)
add_executable(main ${MAIN_FILE_LIST})