cmake_minimum_required(VERSION 3.0.0)

project(
  DimOS
  VERSION 0.1.0
  LANGUAGES CXX C ASM
)

include(CTest)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(INSTALL_DIR $ENV{SYSROOT})

set(COMMON_FLAGS "\
-Wall                \
-Wextra 			       \
-ffreestanding       \
-fno-stack-protector \
-fno-stack-check     \
-fno-lto             \
-fno-pie             \
-fno-pic             \
-m64                 \
-march=x86-64        \
-mabi=sysv           \
-mno-80387           \
-mno-mmx             \
-mno-sse             \
-mno-sse2            \
-mno-red-zone        \
-mcmodel=kernel      \
-MMD                 \
")

set(CMAKE_C_FLAGS "${COMMON_FLAGS} \
-std=c11             \
")

set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} \
-std=c++20          \
-mgeneral-regs-only \
-fno-rtti           \
-fno-exceptions     \
")

SET(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")

add_subdirectory(${CMAKE_SOURCE_DIR}/libc)
add_subdirectory(${CMAKE_SOURCE_DIR}/kernel)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
