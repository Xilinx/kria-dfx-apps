#########################################################################
# Copyright (c) 2022, Xilinx Inc. and Contributors. All rights reserved.
# SPDX-License-Identifier: MIT
#########################################################################

# Native Kria build on Ubuntu 22.04
cmake_minimum_required(VERSION 3.21)
if (POLICY CMP0048)
  cmake_policy (SET CMP0048 NEW)
endif()

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project (kria-dfx-apps CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Insert the extra modules for cmake include
include (collect)
include (options)

add_subdirectory (src/AES128)
add_subdirectory (src/AES192)
add_subdirectory (src/FFT)
add_subdirectory (src/FIR)
