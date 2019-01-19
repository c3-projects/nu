# This is where it should be
# TODO: actual logic

find_path(c3-nu_INCLUDE_DIRS c3/upsilon)
find_package_handle_standard_args(c3-upsilon  DEFAULT_MSG
                                  c3-nu_INCLUDE_DIRS
mark_as_advanced(c3-nu_INCLUDE_DIRS)
