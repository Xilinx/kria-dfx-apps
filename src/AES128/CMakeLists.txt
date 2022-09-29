#########################################################################
# Copyright (c) 2022, Xilinx Inc. and Contributors. All rights reserved.
# SPDX-License-Identifier: MIT
#########################################################################

file(GLOB_RECURSE CFILES "${CMAKE_SOURCE_DIR}/*.c")
SET_SOURCE_FILES_PROPERTIES(${CFILES} PROPERTIES LANGUAGE CXX )
add_executable(aes128 main.c ${CMAKE_SOURCE_DIR}/lib/siha.c)
target_link_libraries(aes128 uuid xrt_coreutil xrt++ dfx-mgr)
install(TARGETS aes128 RUNTIME DESTINATION bin)
