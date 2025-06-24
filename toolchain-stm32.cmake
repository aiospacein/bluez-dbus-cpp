# Set target system
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Set sysroot
set(SYSROOT "/home/aiospace/STM32MPU_workspace/STM32MPU-Ecosystem-v6.0.0/Developer-Package/SDK/sysroots/cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi")
set(CMAKE_SYSROOT ${SYSROOT})

# Set compilers
set(CMAKE_C_COMPILER arm-ostl-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER arm-ostl-linux-gnueabi-g++)

# Set compiler flags
set(CMAKE_C_FLAGS "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -O2 -Wall -std=c11 -Wno-psabi" CACHE STRING "")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -O2 -Wall -std=c++17 -Wno-psabi" CACHE STRING "")
set(CMAKE_C_FLAGS_DEBUG "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -g -O0 -Wall -std=c11 -Wno-unused-function -Wno-unused-variable -Wno-sign-compare -Wno-psabi" CACHE STRING "")
set(CMAKE_CXX_FLAGS_DEBUG "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -g -O0 -Wall -std=c++17 -Wno-unused-function -Wno-unused-variable -Wno-sign-compare -Wno-reorder -Wno-psabi" CACHE STRING "")

# Set find root path
set(CMAKE_FIND_ROOT_PATH ${SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Set pkg-config environment
set(ENV{PKG_CONFIG_SYSROOT_DIR} ${SYSROOT})
set(ENV{PKG_CONFIG_PATH} "${SYSROOT}/usr/lib/pkgconfig:${SYSROOT}/usr/share/pkgconfig")

# Set linker flags
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath-link,${SYSROOT}/usr/lib -Wl,-rpath-link,${SYSROOT}/lib -Wl,--dynamic-linker=/lib/ld-linux-armhf.so.3" CACHE STRING "")

# Debug output
message(STATUS "Toolchain: STM32 cross-compilation")
message(STATUS "SYSROOT: ${SYSROOT}")
message(STATUS "PKG_CONFIG_PATH: $ENV{PKG_CONFIG_PATH}")