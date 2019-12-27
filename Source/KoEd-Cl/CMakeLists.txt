file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.cpp")
add_executable(KoEd-Cl ${HEADERS} ${SOURCES})
target_link_libraries(KoEd-Cl KoEd-Core)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set_target_properties(KoEd-Cl PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  add_definitions(-DUNICODE -D_UNICODE)
endif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
