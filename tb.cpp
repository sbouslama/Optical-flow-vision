#include "OpticalFlow.h"

#include <opencv2/opencv.hpp>
#include "hls_opencv.h"

int main (int argc, char** argv ) {
	IplImage* frame1=cvLoadImage(INPUT_IMAGE1);
	IplImage* frame2=cvLoadImage(INPUT_IMAGE2);
	//IplImage* frame3=cvLoadImage(INPUT_IMAGE3);
	//IplImage* frame4=cvLoadImage(INPUT_IMAGE3);
	IplImage* frame3=cvCreateImage(cvGetSize(frame1), IPL_DEPTH_32F, 1);
	cvZero(frame3);
	IplImage* frame4=cvCreateImage(cvGetSize(frame1), IPL_DEPTH_32F, 1);
	cvZero(frame4);
	IplImage* dst0 = cvCreateImage(cvGetSize(frame1),IPL_DEPTH_32F , 1); //frame1->depth
	IplImage* dst1 = cvCreateImage(cvGetSize(frame1),IPL_DEPTH_32F , 1);

for(int i=0; i<1; i++){
    AXI_STREAM_RGB  frame_input1_axi,frame_input2_axi,frame_input3_axi,frame_input4_axi, frame_output_axi, frame_output1_axi;
	IplImage2AXIvideo(frame1, frame_input1_axi);
	IplImage2AXIvideo(frame2, frame_input2_axi);
	IplImage2AXIvideo(frame3, frame_input3_axi);
	IplImage2AXIvideo(frame4, frame_input4_axi);

	OpticalFlow( frame_input1_axi, frame_input2_axi,frame_input3_axi,frame_input4_axi,frame_output_axi,frame_output1_axi,frame1->height, frame1->width);
	AXIvideo2IplImage(frame_output_axi, dst0);
	AXIvideo2IplImage(frame_output1_axi, dst1);
	frame3=dst0;
	frame4=dst1;
}

	cvConvertScale(dst0,dst0,255);
	cvConvertScale(dst1,dst1,255);
	cvSaveImage(OUTPUT0_IMAGE, dst0);
	cvSaveImage(OUTPUT1_IMAGE, dst1);



	cvReleaseImage(&frame1 );
	cvReleaseImage(&frame2 );
	cvReleaseImage(&dst0);
	cvReleaseImage(&dst1);


	char tempbuf[2000];
	sprintf(tempbuf, "diff --brief -w %s %s", OUTPUT0_IMAGE,GOLDEN_U_IMAGE);
	int ret = system(tempbuf);
	if (ret != 0) {
	    printf("Test1 Failed!\n");
		ret = 1;
	} else {
		printf("Test1 Passed!\n");
	       }

	char tempbuf2[2000];
	sprintf(tempbuf2, "diff --brief -w %s %s", OUTPUT0_IMAGE,GOLDEN_V_IMAGE);
	int ret2 = system(tempbuf2);
	if (ret2 != 0) {
	    printf("Test2 Failed!\n");
		ret2 = 1;
	} else {
		printf("Test2 Passed!\n");
	       }
	return (ret or ret2);

    }
