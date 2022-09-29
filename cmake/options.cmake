#########################################################################
# Copyright (c) 2022, Xilinx Inc. and Contributors. All rights reserved.
# SPDX-License-Identifier: MIT
#########################################################################

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Debug)
endif(NOT DEFINED CMAKE_BUILD_TYPE)
message ("-- Build type: ${CMAKE_BUILD_TYPE}")

find_path (XRT_INCD     NAMES xclbin.h        PATH_SUFFIXES xrt     REQUIRED)
find_path (DFX_MGR_INCD NAMES dfxmgr_client.h PATH_SUFFIXES dfx-mgr REQUIRED)
include_directories (${XRT_INCD} ${DFX_MGR_INCD})

find_library (XRT_LIBRARIES NAMES xrt_coreutil REQUIRED)
find_library (XRT_LIBXX NAMES xrt++        REQUIRED)
find_library (DFXMGR_LIB NAMES dfx-mgr     REQUIRED)

message ("-- XRT_LIBRARIES : ${XRT_LIBRARIES}")
message ("-- XRT_LIBXX : ${XRT_LIBXX}")
message ("-- DFXMGR_LIB : ${DFXMGR_LIB}")
