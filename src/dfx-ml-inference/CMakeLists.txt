#########################################################################
# Copyright (c) 2022, Xilinx Inc. and Contributors. All rights reserved.
# SPDX-License-Identifier: MIT
#########################################################################
cmake_minimum_required(VERSION 3.21)

project(dfx-ml-inference)

SET(CMAKE_INSTALL_PREFIX $ENV{OECORE_TARGET_SYSROOT})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing -fpermissive -rdynamic -Wl,--no-undefined" )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++14")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/")
find_package(GStreamer REQUIRED)
find_package(OpenCV REQUIRED)

SET(INSTALL_PATH "/opt/xilinx/dfx-ml-inference")

add_library(vvas_xpp SHARED lib/vvas_xpp_pipeline.c)
target_include_directories(vvas_xpp PRIVATE ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(vvas_xpp
  jansson vvasutil-2.0 gstvvasinfermeta-2.0)
install(TARGETS vvas_xpp DESTINATION ${INSTALL_PATH}/lib)

add_library(vvas_airender SHARED lib/vvas_airender.cpp)
target_include_directories(vvas_airender PRIVATE ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(vvas_airender 
    jansson vvasutil-2.0 gstvvasinfermeta-2.0 
    gstreamer-1.0 gstbase-1.0 glib-2.0
    opencv_core opencv_video opencv_videoio opencv_imgproc)
install(TARGETS vvas_airender DESTINATION ${INSTALL_PATH}/lib)

install(DIRECTORY
    config
    DESTINATION ${INSTALL_PATH}/vvas/)
    
install(DIRECTORY
    vitis_ai_library/models 
    DESTINATION ${INSTALL_PATH}/vitis_ai_library/)
