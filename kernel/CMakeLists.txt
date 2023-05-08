include(${CMAKE_CURRENT_LIST_DIR}/arch/CMakeLists.txt)

file(GLOB_RECURSE SRC_FILES "kernel/*.cpp")

add_executable(kernel ${SRC_FILES} ${ARCH_SRC_FILES})

set(CMAKE_CXX_LINK_EXECUTABLE "x86_64-elf-gcc -static -z max-page-size=0x1000 -nostdlib -lgcc -nodefaultlibs -no-pie -T ${LINKER_SCRIPT} <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

target_link_libraries(kernel libk)

add_dependencies(kernel install_libc_headers install_kernel_headers libk)

add_custom_target(install_kernel_headers
    COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}/include
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/include ${INSTALL_DIR}/include
    COMMENT "Installing kernel headers to ${INSTALL_DIR}/include"
)
