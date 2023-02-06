# CMakeList.txt : CMake project for dbconnector, include source and define
# project specific logic here.
#
# Add source to this project's executable.
add_executable (dbconnector "dbconnector.cpp" "dbconnector.h"  )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET dbconnector PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

message("source dir: ${CMAKE_SOURCE_DIR}")

target_link_libraries(dbconnector ${CONAN_LIBS})
