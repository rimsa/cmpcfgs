cmake_minimum_required(VERSION 3.10)

project(cmpcfgs VERSION 1.0)
enable_language(CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/include")

# add the executable
add_executable(cmpcfgs
	src/CFG.cpp
	src/CFGsContainer.cpp
	src/CfgData.cpp
	src/CfgNode.cpp
	src/Instruction.cpp
	src/SimpleStrategy.cpp
	src/SpecificStrategy.cpp
	src/Strategy.cpp
	src/cmpcfgs.cpp
)

target_include_directories(cmpcfgs PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           ${EXTRA_INCLUDES})
