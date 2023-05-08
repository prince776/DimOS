file(GLOB_RECURSE SRC_FILES "*.c")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
-D__is_libk         \
")

add_library(libk STATIC ${SRC_FILES})
target_include_directories(libk PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

add_dependencies(libk install_libc_headers install_kernel_headers)

add_custom_target(install_libc_headers
    COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}/include
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/include ${INSTALL_DIR}/include
    COMMENT "Installing libc headers to ${INSTALL_DIR}/include"
)
