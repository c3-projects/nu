cmake_minimum_required(VERSION 3.9)

project(nu)

set(VERSION 0.1.3)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 17)

add_compile_options("-Wall" "-Wextra")

find_package(Threads REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE tests tests/*.cxx)

foreach(test ${tests})
  file(RELATIVE_PATH test_rel ${CMAKE_SOURCE_DIR}/tests/ ${test})
  get_filename_component(test_fname ${test_rel} NAME_WE)
  get_filename_component(test_dir ${test_rel} DIRECTORY)

  set(test_name ${test_dir}_${test_fname})

  add_executable(${test_name} ${test})

  target_link_libraries(${test_name} ${CMAKE_THREAD_LIBS_INIT})

  add_test(${test_name} ${test_name})
endforeach()

enable_testing()

SET(CPACK_PACKAGE_VERSION ${VERSION})

install(
  DIRECTORY include/
  DESTINATION include/
  COMPONENT dev
)

install(
  DIRECTORY cmake/
  DESTINATION lib/c3/cmake
  COMPONENT dev
)

if(NOT DEFINED CPACK_GENERATOR)
  # TODO: platform logic
  set(CPACK_GENERATOR "DEB")
endif()

if(CPACK_GENERATOR STREQUAL "DEB")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "c3-projects")
  set(CPACK_DEB_COMPONENT_INSTALL ON)
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
  set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)
  set(CPACK_DEBIAN_PACKAGE_NAME "c3-${PROJECT_NAME}-dev")
  SET(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++-7-dev | libc++-7-dev")
endif()

INCLUDE(CPack)
