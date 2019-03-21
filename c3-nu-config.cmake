# For direct linking
add_library(c3-nu INTERFACE IMPORTED GLOBAL)
set_target_properties(c3-nu PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/include)

find_path (GSL_INCLUDE_DIR NAMES span gsl)
include_directories(${GSL_INCLUDE_DIR})
