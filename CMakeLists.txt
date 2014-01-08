add_library(util STATIC util.cpp util.h)
add_library(nbn STATIC nbn.cpp nbn.h)
target_link_libraries(nbn util)
