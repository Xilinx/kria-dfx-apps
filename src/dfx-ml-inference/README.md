# README for dfx ml inference 

Repository structure is outlined below. 

* config
	* facedetect -json files
	* refinedet - json files 
	* ssd
* lib - library for pp_pipeline 
	* vvas_airender.cpp
	* vvas_airender.hpp
	* vvas_xpp_pipeline.c
* vitis_ai_library
 	* models - Compiled using vitis-ai caffe
		* densebox_640_360
		* refinedet_pruned_0_96
		* ssd_adas_pruned_0_95
* CMakeLists.txt 
* FindGStreamer.cmake 
* dfx-ml-inference.ipynb - Jupyter notebook 
