# this is necessary for cmake to detect cross compilation.
set(CMAKE_SYSTEM_NAME "Generic")

# set c and c++ compilers
set(CMAKE_C_COMPILER x86_64-elf-gcc)
set(CMAKE_CXX_COMPILER x86_64-elf-g++)

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# set sysroot
set(ENV_SYSROOT $ENV{SYSROOT})
set(CMAKE_SYSROOT ${ENV_SYSROOT})

include_directories(SYSTEM ${CMAKE_SYSROOT}/include)
