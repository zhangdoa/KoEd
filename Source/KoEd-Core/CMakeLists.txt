file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.cpp")
add_library(KoEd-Core ${HEADERS} ${SOURCES})
set_property(TARGET KoEd-Core PROPERTY POSITION_INDEPENDENT_CODE ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set_target_properties(KoEd-Core PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  add_definitions(-DUNICODE -D_UNICODE)
endif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
