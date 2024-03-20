project(myengine)

set(CMAKE_CXX_STANDARD 11)

# libraries
include_directories(libs)
add_subdirectory(libs)

# engine
add_subdirectory(engine)