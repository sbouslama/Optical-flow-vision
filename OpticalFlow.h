#ifndef _OPTICALFLOW_H_
#define _OPTICALFLOW_H_

#include "hls_video.h"

#include <stdio.h>
#include "hls_math.h"
#include "hls_linear_algebra.h"
//#include "hls_opencv.h"
//#include <opencv2/opencv.hpp>

#define MAX_WIDTH  1920
#define MAX_HEIGHT 1080


#define INPUT_IMAGE1           "RGB_32.png"
#define INPUT_IMAGE2           "RGB_33.png"
#define INPUT_IMAGE3           "ZERO.png"


#define OUTPUT0_IMAGE          "result0.png"
#define OUTPUT1_IMAGE          "result1.png"
#define OUTPUT2_IMAGE          "result2.png"

#define OUTPUT_curr_IMAGE     "curr_frame.png"
#define MAXKEYPOINTS 		   2000
#define smoothness 		   	   0.05
#define numIteration		   3

#define GOLDEN_U_IMAGE          "GOLDEN_U.png"
#define GOLDEN_V_IMAGE          "GOLDEN_U.png"

// typedef video library core structures
typedef hls::stream<ap_axiu<32,1,1,1> >               AXI_STREAM_RGB;
typedef hls::stream<ap_axiu<8,1,1,1> >                AXI_STREAM_GRAY;

typedef hls::Mat<MAX_HEIGHT,MAX_WIDTH,HLS_8UC1> 	  GRAY_IMAGE;
typedef hls::Mat<MAX_HEIGHT,MAX_WIDTH,HLS_8UC3>		  RGB_IMAGE;
typedef hls::Mat<MAX_HEIGHT,MAX_WIDTH,HLS_32FC1>	  FLOAT_IMAGE;
typedef float			 						      MY_DATA ;
typedef hls::Scalar<1, float> 				 		  PIXEL_VAL;
typedef hls::Scalar<1, unsigned char> 				  PIXEL_VAL_CHAR;


//typedef hls::Window<4*w*w,1,HLS_TNAME(HLS_16SC1)> 	  WINDOW;

// top level function for HW synthesis
void OpticalFlow(AXI_STREAM_RGB& currentframe,AXI_STREAM_RGB& previousframe,
				 AXI_STREAM_RGB& u,AXI_STREAM_RGB& v,
				 AXI_STREAM_RGB& uout,AXI_STREAM_RGB& vout,
				 int rows, int cols);
#endif
