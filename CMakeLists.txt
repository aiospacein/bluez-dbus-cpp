cmake_minimum_required(VERSION 3.10)

# Cross-compilation support
option(TARGET_STM32 "Build for STM32 target" OFF)
if(TARGET_STM32)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain-stm32.cmake" CACHE FILEPATH "Toolchain file")
endif()

project(bluez-dbus-cpp VERSION 0.0.1 LANGUAGES C CXX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")

# Default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Compiler options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Debug/Release flags
if(NOT TARGET_STM32)
    set(CMAKE_C_FLAGS "-O2 -Wall -std=c11" CACHE STRING "C flags")
    set(CMAKE_CXX_FLAGS "-O2 -Wall -g -std=c++17 -fno-pic -Wno-unused-function -Wno-unused-variable -Wno-sign-compare -Wno-reorder" CACHE STRING "C++ flags")
    set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -std=c11")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -std=c++17 -fno-pic")
endif()

# Required packages
include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)
find_package(sdbus-c++ REQUIRED)

# Source and header configuration
set(BLUEZ_HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(BLUEZ_HEADER_PREFIX "bluez-dbus-cpp")
set(BLUEZ_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(BLUEZ_HEADERS
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/Adapter1.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/GattManager1.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/GattService1.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/GattCharacteristic1.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/GattDescriptor1.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/LEAdvertisingManager1.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/LEAdvertisement1.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/GattApplication1.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/GenericCharacteristic.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/ReadOnlyCharacteristic.h"
    "${CMAKE_SOURCE_DIR}/example/SerialCharacteristic.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/Util.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/bluez.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/bluez-ext.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/Client.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/Socket.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/proxy/Adapter1_proxy.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/proxy/AgentManager1_proxy.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/adaptor/GattCharacteristic1_adaptor.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/adaptor/GattDescriptor1_adaptor.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/adaptor/GattService1_adaptor.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/adaptor/ObjectManagerExt_adaptor.h"
    "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/adaptor/PropertiesExt_adaptor.h"
)

set(BLUEZ_SOURCES
    "${BLUEZ_SOURCE_DIR}/GattService1.cpp"
    "${BLUEZ_SOURCE_DIR}/GattCharacteristic1.cpp"
    "${BLUEZ_SOURCE_DIR}/GattDescriptor1.cpp"
    "${BLUEZ_SOURCE_DIR}/Util.cpp"
)

# Build library
add_library(bluez-dbus-cpp SHARED
    ${BLUEZ_HEADERS}
    ${BLUEZ_SOURCES}
)

target_include_directories(bluez-dbus-cpp PUBLIC
    $<BUILD_INTERFACE:${BLUEZ_HEADER_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(bluez-dbus-cpp PUBLIC SDBusCpp::sdbus-c++ )

# Shared lib options
option(BUILD_SHARED_LIBS "Build shared libraries (.so) instead of static ones (.a)" ON)
if(DEFINED BUILD_SHARED_LIBS)
    set_target_properties(bluez-dbus-cpp PROPERTIES POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})
endif()

set_target_properties(bluez-dbus-cpp
    PROPERTIES
    PUBLIC_HEADER "${BLUEZ_HEADERS}"
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    OUTPUT_NAME "bluez-dbus-cpp"
)

# -------------------------------
# INSTALL
# -------------------------------
install(TARGETS bluez-dbus-cpp
    EXPORT bluez-dbus-cpp-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${BLUEZ_HEADER_PREFIX}  # ✅ add this line
    COMPONENT dev
)

install(DIRECTORY "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/adaptor/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${BLUEZ_HEADER_PREFIX}/adaptor"
    FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/proxy/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${BLUEZ_HEADER_PREFIX}/proxy"
    FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY "${BLUEZ_HEADER_DIR}/${BLUEZ_HEADER_PREFIX}/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${BLUEZ_HEADER_PREFIX}"
    FILES_MATCHING PATTERN "*.h"
)

install(FILES README.md LICENSE_1_0.txt DESTINATION ${CMAKE_INSTALL_DOCDIR})

# -------------------------------
# EXAMPLES
# -------------------------------
option(BUILD_EXAMPLE "Build standard example" OFF)
option(BUILD_EXAMPLE_ELL "Build ELL socket example" OFF)

if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

if(BUILD_EXAMPLE_ELL)
    add_subdirectory(example-ell-sockets)
endif()

# -------------------------------
# CONFIG FILES FOR find_package()
# -------------------------------
include(CMakePackageConfigHelpers)

install(EXPORT bluez-dbus-cpp-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bluez-dbus-cpp
    NAMESPACE BlueZ::
    COMPONENT dev
)

configure_package_config_file(
    cmake/bluez-dbus-cpp-config.cmake.in
    cmake/bluez-dbus-cpp-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bluez-dbus-cpp
)

write_basic_package_version_file(
    cmake/bluez-dbus-cpp-config-version.cmake
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/bluez-dbus-cpp-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/bluez-dbus-cpp-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bluez-dbus-cpp
    COMPONENT dev
)

# -------------------------------
# PKG-CONFIG
# -------------------------------
configure_file(pkgconfig/bluez-dbus-cpp.pc.in pkgconfig/bluez-dbus-cpp.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/bluez-dbus-cpp.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT dev
)
