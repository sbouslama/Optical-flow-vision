#include"top.h"
using namespace hls;
void Derivate(GRAY_IMAGE& src, SHORT_IMAGE& dst, const int tab[2][2]){

	hls::Window<2,2,int> 	kernel;
    hls::Point_<int> 		anchor;

    for (int i = 0; i < 2; i++)
    	for (int j = 0; j < 2; j++)
            kernel.val[i][j] = tab[i][j];



    anchor.x = -1;
    anchor.y = -1;

    hls::Filter2D(src, dst, kernel, anchor);
}
template<int ROWS, int COLS, int SRC_T>
void DisplayMatrix(Mat<ROWS,COLS,SRC_T>& src){
	assert("Source and destination images must have required number of channels"
			&& HLS_MAT_CN(SRC_T) == 1);

	Scalar < HLS_MAT_CN(SRC_T),HLS_TNAME(SRC_T)>	s;

	HLS_SIZE_T rows = src.rows;
	HLS_SIZE_T cols = src.cols;

	loop1:for(HLS_SIZE_T j =0; j < rows ; j++){
		loop2:for(HLS_SIZE_T i =0; i < cols ; i++){
			src >> s ;
			printf("%d,",(int)s.val[0]);
		}
		printf("\n");
		}
	printf("end \n");

}

template<int ROWS, int COLS, int SRC_T>
void Mat2Vect( Mat<ROWS,COLS,SRC_T>& src,
			   signed short Vector[ROWS*COLS][1]){

	Scalar < HLS_MAT_CN(SRC_T),HLS_TNAME(SRC_T)>	s;

		HLS_SIZE_T rows = src.rows;
		HLS_SIZE_T cols = src.cols;
		HLS_SIZE_T k=0;
		loop1:for(HLS_SIZE_T j =0; j < rows ; j++){
			loop2:for(HLS_SIZE_T i =0; i < cols ; i++){
				src >> s ;
				Vector[k][0]= s.val[0];
				//printf("vector= %d \n ",(int)Vector[k][0]);
				k++;
			}
		}

}
template<int ROWS, int COLS, int SRC_T,int DST_T>
void SubMatrix( Mat<ROWS,COLS,SRC_T>& src,
				int x,int y,
				Mat<2*w,2*w,DST_T>& dst){

	Scalar < HLS_MAT_CN(SRC_T),HLS_TNAME(SRC_T)>	s;
	Scalar < HLS_MAT_CN(DST_T),HLS_TNAME(DST_T)>	d;

		HLS_SIZE_T rows = src.rows;
		HLS_SIZE_T cols = src.cols;
		HLS_SIZE_T m=0,n=0;
		loop1:for(HLS_SIZE_T j =0;j <rows ; j++){
			loop2:for(HLS_SIZE_T i =0; i < cols ; i++){
				src >> s ;
				if (j >= x-w && i >= y-w &&  j<x+w && i<y+w){
						dst << s ;
				}
			}
		}
}

template<int N, int SRC_T>
void Vect2Mat( signed short Vector1 [N][1],
                 signed short Vector2 [N][1],
                 Mat<N,2,SRC_T>& dst ){

	Scalar < HLS_MAT_CN(SRC_T),HLS_TNAME(SRC_T)>	d;

		HLS_SIZE_T rows=dst.rows;
		HLS_SIZE_T cols=dst.cols;
		HLS_SIZE_T i =0;
		loop1:for(HLS_SIZE_T j =0;j <rows ; j++){
				if (i==0){
				d.val[0]=Vector1[j][0];
				dst << d ;
				i++;
				}
				if(i==1){
				d.val[0]=Vector2[j][0];
				dst << d ;
				}
				i=0;
			}
}

/*template<int N>
void Vect2Mat( signed short Vector1 [4*w*w][1],
               signed short Vector2 [4*w*w][1],
               signed short out[4*w*w][2]
                                   ){
		loop1:for(int j =0; j < 4*w*w ; j++){
				out[j][0]=Vector1[j][0];
				out[j][1]=Vector2[j][0];
				printf("%d,%d",out[j][0],out[j][1] );
			}
}*/


void top(
		AXI_STREAM_RGB& currentframe,AXI_STREAM_RGB& previousframe,
		AXI_STREAM_RGB& dstcurr,AXI_STREAM_RGB& motion,
		int rows, int cols)
{

#pragma HLS RESOURCE variable=previousframe core=AXIS metadata="-bus_bundle INPUT_PREVIOUS"
#pragma HLS RESOURCE variable=currentframe core=AXIS metadata="-bus_bundle INPUT_CURRENT"

#pragma HLS RESOURCE variable=dstcurr core=AXIS metadata="-bus_bundle OUTPUT_DSTCURRENT"
#pragma HLS RESOURCE variable=motion core=AXIS metadata="-bus_bundle OUTPUT_MOTION"
#pragma HLS RESOURCE core=AXI_SLAVE variable=rows metadata="-bus_bundle CONTROL_BUS"
#pragma HLS RESOURCE core=AXI_SLAVE variable=cols metadata="-bus_bundle CONTROL_BUS"
#pragma HLS RESOURCE core=AXI_SLAVE variable=return metadata="-bus_bundle CONTROL_BUS"

#pragma HLS interface ap_stable port=rows
#pragma HLS interface ap_stable port=cols

	RGB_IMAGE      curr_frame(rows,cols);
	RGB_IMAGE      dst0_curr_frame(rows,cols);
	RGB_IMAGE      dst1_curr_frame(rows,cols);
	RGB_IMAGE      dst2_curr_frame(rows,cols);
	RGB_IMAGE      dst3_curr_frame(rows,cols);
	GRAY_IMAGE     curr_gray(rows,cols);
	GRAY_IMAGE     curr0_gray(rows,cols);
	GRAY_IMAGE     curr1_gray(rows,cols);
	GRAY_IMAGE     curr2_gray(rows,cols);
	GRAY_IMAGE     curr3_gray(rows,cols);
	GRAY_IMAGE     curr4_gray(rows,cols);
	GRAY_IMAGE     curr5_gray(rows,cols);

	RGB_IMAGE      prev_frame(rows,cols);
	GRAY_IMAGE     prev_gray(rows,cols);
	//GRAY_IMAGE     mask(rows,cols);
	//GRAY_IMAGE     dmask(rows,cols);

	SHORT_IMAGE     Ix_m(rows,cols);
	SHORT_IMAGE     Iy_m(rows,cols);
	SHORT_IMAGE     Iy1_m(rows,cols);
	SHORT_IMAGE     Iy2_m(rows,cols);

	SHORT_IMAGE     It1_m(rows,cols);
	SHORT_IMAGE     It2_m(rows,cols);
	SHORT_IMAGE     It_m(rows,cols);
	SHORT_IMAGE     tmp_It_m(rows,cols);
	//SHORT_IMAGE     Ix(2*w,2*w);
	//SHORT_IMAGE     Iy(2*w,2*w);
	//SHORT_IMAGE     It(2*w,2*w);

	signed short 	b[4*w*w][1];
	signed short 	Ix_vect[4*w*w][1];
	signed short 	Iy_vect[4*w*w][1];
    //signed short 	A[4*w*w][2];


	//signed short 	tab[rows][cols];
	//signed short    Vector[2*w][2*w];
	//Mat<N,1,HLS_16SC1> b ;
	Mat<2*w,2*w,HLS_16SC1> Ix ;
	Mat<2*w,2*w,HLS_16SC1> Iy ;
	Mat<2*w,2*w,HLS_16SC1> tmp_b ;
	Mat<4*w*w,2,HLS_16SC1> A ;

	//Mat<4*w*w,1,HLS_16SC1> Iy ;
	//Mat<4*w*w,1,HLS_16SC1> It ;
	//WINDOW 			Ix;

	//SHORT_IMAGE     Ix(rows,cols);


    hls::Point                      C1[MAXKEYPOINTS];
    hls::Scalar<3,unsigned char> 	color(255,0,0);
    const int kernelx[2][2] = {{-1,1},{-1,1}};
    const int kernely[2][2] = {{-1,-1},{1,1}};
    const int ones[2][2] = {{1,1},{1,1}};
    const int mones[2][2] = {{-1,-1},{-1,-1}};


//#pragma HLS dataflow
	hls::AXIvideo2Mat(currentframe,curr_frame);
	hls::Duplicate(curr_frame,dst0_curr_frame,dst1_curr_frame);

//#pragma HLS stream depth=20000 variable=dst3_curr_frame.data_stream
	//hls::Duplicate(dst1_curr_frame,dst2_curr_frame,dst3_curr_frame);
	hls::CvtColor<HLS_RGB2GRAY>(dst1_curr_frame,curr_gray);

	hls::Duplicate(curr_gray,curr0_gray,curr1_gray);
	hls::Duplicate(curr1_gray,curr2_gray,curr3_gray);
	hls::Duplicate(curr3_gray,curr4_gray,curr5_gray);


	 hls::AXIvideo2Mat(previousframe,prev_frame);
	 hls::CvtColor<HLS_RGB2GRAY>(prev_frame,prev_gray);

/*******************************point of interest ****************/
	 hls::FASTX(curr0_gray,C1,50,false);
	 int nb_pt=0;
	 loop_keypoints: for(HLS_SIZE_T i = 0; i < MAXKEYPOINTS ; i++){
		 #pragma HLS PIPELINE II=1
 	 	 #pragma HLS LOOP_FLATTEN
		 	 unsigned char x=C1[i].x;
		 	 unsigned char y=C1[i].y;
	 		    	if ((int)x-w >= 1 && (int)y-w>=1 && (int)x+w <= rows-1 && (int)y+w <= cols-1){
	 		    		nb_pt++;
	 		    		//printf("C= %d,%d \n",(int)C[i].x,(int)C[i].y);
	 		    	}
	 		    }
	 hls::Point                      C [nb_pt];

	 HLS_SIZE_T l=0;

	 loop_keypoints2:for(HLS_SIZE_T i = 0; i < nb_pt ; i++){
	 		 #pragma HLS PIPELINE II=1
	  	 	 #pragma HLS LOOP_FLATTEN
		 	 	 	 unsigned char x=C1[i].x;
		 		 	 unsigned char y=C1[i].y;
		 	 	 if ((int)x-w >= 1 && (int)y-w>=1 && (int)x+w <= rows-1 && (int)y+w <= cols-1){
		 	 		    		C[l].x=C1[i].x;
		 	 		    		C[l].y=C1[i].y;
		 	 		    		l++;
	 	 		    	//printf("C= %d,%d \n",(int)C[l].x,(int)C[l].y);
	 	 		    }
	 }

/*****************************************************************/

/****************************ImplementLucas&kanade****************/
	 Derivate(curr2_gray,Ix_m,kernelx);
	 Derivate(curr4_gray,Iy_m,kernely);
	 Derivate(curr5_gray,It1_m,ones);
	 Derivate(prev_gray,It2_m,mones);
	 AddWeighted(It1_m,1,It2_m,1,0,It_m);

	 /*int x=(int)C[0].x ;
	 int y=(int)C[0].y ;
	 printf("coor=%d,%d \n",x,y);
	 SubMatrix(Ix_m,x,y,Ix);
	 hls::Duplicate(Iy_m,Iy1_m,Iy2_m);
	 SubMatrix(Iy1_m,x,y,Iy);
	 hls::Muls(It_m,-1,tmp_It_m);
	 SubMatrix(tmp_It_m,x,y,tmp_b);
	 Mat2Vect(tmp_b,b);
	 Mat2Vect(Ix,Ix_vect);
	 Mat2Vect(Iy,Iy_vect);
	 Vect2Mat(Ix_vect,Iy_vect,A);
	 fct1(A, b, nu);*/


	 /*loop3:for(int i=0;i<4*w*w;i++){
		 printf("Ix_vect= %d \n ,",Ix_vect[i][0]);
	 }

	 loop4:for(int i=0;i<4*w*w;i++){
	 		 printf("Iy_vect= %d \n ,",Iy_vect[i][0]);
	 	 }*/

	 //DisplayMatrix(A);
	/*loop_keypoints3:for(int i = 0; i < nb_pt ; i++){

		 int x=(int)C[i].x;
		 int y=(int)C[i].y;
		 SubMatrix(Ix_m,x,y,Ix);
		 SubMatrix(Iy_m,x,y,Iy);
		 SubMatrix(It_m,x,y,It);
		 hls::Muls(It_m,-1,tmp_b);
		 Mat2Vect(tmp_b,b);
	 }*/


		 /*loop3:for(int i=0;i<4*w*w;i++){
			 printf("buff =%d \n ",(int)Ix.getval(i,0));
		 }*/


	hls::Mat2AXIvideo(Iy_m, dstcurr);
	hls::Mat2AXIvideo(Ix_m, motion);


}
/*hls::FASTX(curr_gray,mask,50,true);
	 hls::Dilate(mask,dmask);
    hls::PaintMask(dst3_curr_frame,dmask,_motion,color);
    hls::Scalar < HLS_MAT_CN(HLS_8UC3),HLS_TNAME(HLS_8UC3)>	s;

    	    	loop_height1:for(int i = 0; i < rows; i++){
    	        	loop_width1:for(int j = 0; j < cols; j++){
    					#pragma HLS PIPELINE II=1
    	    			#pragma HLS LOOP_FLATTEN
    	        		_motion >> s;
    					printf("motion[ %d",i,",%d",j,"]=",s.val[0],"\n");

    				}
    			}*/
