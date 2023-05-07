include(${CMAKE_CURRENT_LIST_DIR}/arch/CMakeLists.txt)

file(GLOB_RECURSE SRC_FILES "kernel/*.cpp")

SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} \
-static                     \
-z max-page-size=0x1000     \
-nostdlib                   \
-lgcc                       \
-nodefaultlibs              \
")


add_executable(kernel ${SRC_FILES} ${ARCH_SRC_FILES})

message("linker script: ${LINKER_SCRIPT}")
target_link_options(kernel PRIVATE -T ${LINKER_SCRIPT})
target_link_libraries(kernel libk)

target_include_directories(kernel PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

add_dependencies(kernel install_libc_headers install_kernel_headers libk)

add_custom_target(install_kernel_headers
    COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}/include
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/include ${INSTALL_DIR}/include
    COMMENT "Installing kernel headers to ${INSTALL_DIR}/include"
)
