#########################################################################
# Copyright (c) 2022, Xilinx Inc. and Contributors. All rights reserved.
# SPDX-License-Identifier: MIT
#########################################################################

file(GLOB_RECURSE CFILES "${CMAKE_SOURCE_DIR}/*.c")
SET_SOURCE_FILES_PROPERTIES(${CFILES} PROPERTIES LANGUAGE CXX )
add_executable(fft main.c ${CMAKE_SOURCE_DIR}/lib/siha.c)
target_link_libraries(fft uuid xrt_coreutil xrt++ dfx-mgr)
install(TARGETS fft RUNTIME DESTINATION bin)
