# add_library(obstack SHARED obstack.c)
# set_target_properties(obstack PROPERTIES
#     WINDOWS_EXPORT_ALL_SYMBOLS ON
# )

add_library(obstack STATIC obstack.c)
