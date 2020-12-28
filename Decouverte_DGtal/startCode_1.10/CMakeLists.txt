PROJECT(startCode)

### Required in DGtal
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
FIND_PACKAGE(DGtal REQUIRED)
INCLUDE_DIRECTORIES(${DGTAL_INCLUDE_DIRS})
LINK_DIRECTORIES(${DGTAL_LIBRARY_DIRS})

# Make sure the compiler can find include files.
include_directories (${PROJECT_SOURCE_DIR}/Volumes/)
include_directories (${PROJECT_BINARY_DIR}/Volumes/)

# ConfigExamples.h instanciation.
configure_file(${PROJECT_SOURCE_DIR}/ConfigExamples.h.in 
  ${PROJECT_SOURCE_DIR}/ConfigExamples.h)

ADD_EXECUTABLE(startCode startCode.cpp)
TARGET_LINK_LIBRARIES(startCode ${DGTAL_LIBRARIES})
