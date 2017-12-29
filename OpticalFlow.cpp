#include"OpticalFlow.h"

using namespace hls;

template<int WIDTH>
MY_DATA my_filter(MY_DATA window[WIDTH][WIDTH],MY_DATA kernel[WIDTH][WIDTH]){

	MY_DATA sum = 0 ;

	//MY_DATA kernel[3][3]={{0,1,0},{1,0,1},{0,2,0}};
	//MY_DATA kernel[3][3]={{-1,0,1},{-2,0,2},{-1,0,1}};
	Convolution_Loop_ROW: for(int i=0; i < WIDTH; i++){
	  #pragma HLS PIPELINE II=1
	  Convolution_Loop_COL: for (int j=0; j<WIDTH; j++){
		#pragma HLS UNROLL factor=4
		  sum=sum+(window[i][j]*kernel[i][j]);
	  }
	}

	return sum;
}
template<int WIDTH >
void apply_filter(FLOAT_IMAGE& src,FLOAT_IMAGE& dst,MY_DATA kernel[WIDTH][WIDTH], int rows,int cols){

	PIXEL_VAL pixel_in;
	PIXEL_VAL pixel_out;

	MY_DATA new_pixel;
	MY_DATA window_buf_a[WIDTH][WIDTH];
	MY_DATA line_buf_a[WIDTH][640];
	for(int row = 0; row <= rows; row++){
	#pragma HLS LOOP_TRIPCOUNT min=480 max=480 avg=480
	#pragma HLS PIPELINE II=1
		for(int col = 0; col <= cols; col++){
		#pragma HLS LOOP_TRIPCOUNT min=640 max=640 avg=640
		#pragma HLS loop_flatten
		#pragma HLS dependence variable=line_buf_a false
		#pragma HLS PIPELINE II=1
			if(col < cols && row < rows)
				src >> pixel_in;
			 	new_pixel = pixel_in.val[0];

			 for (int i=0; i<WIDTH-1; i++)
			  #pragma HLS UNROLL factor=4
				 if (col < cols)
					 line_buf_a[i][col] = line_buf_a[i+1][col];

			  line_buf_a[WIDTH-1][col] = new_pixel;

			  for (int i=0; i<WIDTH; i++)
				  for (int j=0; j<WIDTH-1; j++)
			  	  #pragma HLS UNROLL factor=4
					  window_buf_a[i][j] = window_buf_a[i][j+1];

			  for (int i=0; i<WIDTH; i++)
			  #pragma HLS UNROLL factor=4
				  if (col < cols)
					  window_buf_a[i][WIDTH-1] = line_buf_a[i][col];


			  if((row < (WIDTH-1)) || (col < (WIDTH-1)) || (row >= (rows-(WIDTH-1))) || (col >= (cols-(WIDTH-1)))){
				  new_pixel = 0;
			  }
			  else
			  new_pixel = my_filter(window_buf_a,kernel);

			  pixel_out.val[0] = new_pixel*(1.0f/255.0f);
			  if(row > 0 && col > 0)
			  dst << pixel_out;

		}
	}
}
/*template<int ROWS, int COLS, int SRC_T, int DST_T,int N>
void apply_filter(
		Mat<ROWS,COLS,SRC_T>& src,
		Mat<ROWS,COLS,DST_T>& dst,
		MY_DATA tab[N][N]){

	Mat<ROWS,COLS,DST_T> tmp;

	hls::Window< N, N,MY_DATA> 	kernel;
    hls::Point_<MY_DATA> 		anchor;

    for (int i = 0; i < N; i++){
    	for (int j = 0; j <  N; j++){
#pragma HLS PIPELINE II=1
            kernel.val[i][j] = tab[i][j];
    	}
    }



    anchor.x = -1;
    anchor.y = -1;

    hls::Filter2D(src, tmp, kernel, anchor);
    //Scale(tmp,dst,1.0f/255.0f,0.0f);
}*/
void computeDer(FLOAT_IMAGE& src1,
				FLOAT_IMAGE& src2,
				FLOAT_IMAGE& Ix,
				FLOAT_IMAGE& Iy,
				FLOAT_IMAGE& It,
				int rows,
				int cols ){

	FLOAT_IMAGE src_d1(rows,cols);
	FLOAT_IMAGE src_d2(rows,cols);
	FLOAT_IMAGE src_d3(rows,cols);
	FLOAT_IMAGE src_d4(rows,cols);
	FLOAT_IMAGE It1_m(rows,cols);
	FLOAT_IMAGE It2_m(rows,cols);

    MY_DATA kernelx[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};
    MY_DATA kernely[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
    MY_DATA ones[2][2] = {{1,1},{1,1}};
    MY_DATA mones[2][2] = {{-1,-1},{-1,-1}};

//#pragma HLS dataflow
    Duplicate(src1,src_d1,src_d2);
    Duplicate(src_d1,src_d3,src_d4);
    apply_filter(src_d2,Ix,kernelx,rows,cols);
    apply_filter(src_d3,Iy,kernely,rows,cols);
    apply_filter(src_d4,It1_m,ones,rows,cols);
    apply_filter(src2,It2_m,mones,rows,cols);
	AddWeighted(It1_m,1,It2_m,1,0,It);


}

void CreateTensor(FLOAT_IMAGE& src1,
				FLOAT_IMAGE& src2,
				FLOAT_IMAGE& src3,
				FLOAT_IMAGE& J11,
				FLOAT_IMAGE& J12,
				FLOAT_IMAGE& J13,
				FLOAT_IMAGE& J22,
				FLOAT_IMAGE& J23,
				int rows,
				int cols){

	FLOAT_IMAGE     Ix0_m(rows,cols);
	FLOAT_IMAGE     Ix1_m(rows,cols);
	FLOAT_IMAGE     Ix2_m(rows,cols);
	FLOAT_IMAGE     Ix3_m(rows,cols);
	FLOAT_IMAGE     Ix4_m(rows,cols);
	FLOAT_IMAGE     Ix5_m(rows,cols);
	FLOAT_IMAGE     Iy0_m(rows,cols);
	FLOAT_IMAGE     Iy1_m(rows,cols);
	FLOAT_IMAGE     Iy2_m(rows,cols);
	FLOAT_IMAGE     Iy3_m(rows,cols);
	FLOAT_IMAGE     Iy4_m(rows,cols);
	FLOAT_IMAGE     Iy5_m(rows,cols);
	FLOAT_IMAGE     It1_m(rows,cols);
	FLOAT_IMAGE     It2_m(rows,cols);
	FLOAT_IMAGE     It3_m(rows,cols);
	FLOAT_IMAGE     It4_m(rows,cols);

//#pragma HLS dataflow
	Duplicate(src1,Ix0_m,Ix1_m);
    Duplicate(Ix0_m,Ix2_m,Ix3_m);
    Duplicate(Ix1_m,Ix4_m,Ix5_m);
	Duplicate(src2,Iy0_m,Iy1_m);
	Duplicate(Iy0_m,Iy2_m,Iy3_m);
	Duplicate(Iy1_m,Iy4_m,Iy5_m);
	Duplicate(src3,It3_m,It4_m);

	Mul(Ix2_m,Ix3_m,J11);
	Mul(Ix4_m,Iy2_m,J12);
	Mul(Ix5_m,It3_m,J13);
	Mul(Iy3_m,Iy4_m,J22);
	Mul(Iy5_m,It4_m,J23);
}
void HS(FLOAT_IMAGE& src1,
		FLOAT_IMAGE& src2,
		FLOAT_IMAGE& src3,
		FLOAT_IMAGE& src4,
		FLOAT_IMAGE& src5,
		FLOAT_IMAGE& src6,
		FLOAT_IMAGE& src7,
		FLOAT_IMAGE& dst1,
		FLOAT_IMAGE& dst2,
		int rows,
		int cols){


	PIXEL_VAL	s1;
	PIXEL_VAL	s2;
	PIXEL_VAL	s3;
	PIXEL_VAL	s4;
	PIXEL_VAL	s5;
	PIXEL_VAL	s6;
	PIXEL_VAL	s7;
	PIXEL_VAL	d1;
	PIXEL_VAL	d2;
	FLOAT_IMAGE     u_avg(rows,cols);
	FLOAT_IMAGE     v_avg(rows,cols);
	MY_DATA D2=0.0f,N1=0.0f,N2=0.0f,P1=0.0f,P2=0.0f;
	MY_DATA         average[3][3]={{0,1,0},{1,0,1},{0,1,0}};
	apply_filter(src1,u_avg,average,rows,cols);
	apply_filter(src7,v_avg,average,rows,cols);
loop1:for(HLS_SIZE_T j =0; j < rows ; j++){
#pragma HLS LOOP_TRIPCOUNT min=480 max=480 avg=480
#pragma HLS PIPELINE II=1
		loop2:for(HLS_SIZE_T i =0; i < cols ; i++){
		 #pragma HLS LOOP_TRIPCOUNT min=640 max=640 avg=640
		 #pragma HLS PIPELINE II=1
		 //#pragma HLS loop_flatten off
		//	u >> s1;J11=Ix*Ix >> s2;J12=Ix*Iy >> s3;J13=Ix*It >> s4;J22=Iy*Iy >> s5; J23=Iy*It >> s6 ; v >> s7;
			u_avg >> s1;
			src2 >> s2;
			src3 >> s3;
			src4 >> s4;
			src5 >> s5;
			src6 >> s6;
			v_avg >> s7 ;

		     D2=(smoothness*smoothness+s2.val[0]+s5.val[0]);
		     N1=(s2.val[0]*s1.val[0]+s3.val[0]*s7.val[0]+s4.val[0]);
		     N2=(s2.val[0]*s3.val[0]+s5.val[0]*s7.val[0]+s6.val[0]);
		     P1=(N1/D2);
		     P2=(N2/D2);

			d1.val[0]=(s1.val[0]+P1);
			d2.val[0]=(s7.val[0]+P2) ;
			// pixel = d1.val[0];
			//cout << pixel << ",";
			dst1 << d1 ;
			dst2 << d2 ;
			//printf("val2=%d,",(int)(smoothness*smoothness+s2.val[0]*s2.val[0]+s5.val[0]*s5.val[0]));


		}
		//printf("\n");
	}
}
void OpticalFlow(
		AXI_STREAM_RGB& currentframe,AXI_STREAM_RGB& previousframe,
		AXI_STREAM_RGB& u,AXI_STREAM_RGB& v,
		AXI_STREAM_RGB& uout,AXI_STREAM_RGB& vout,
		int rows, int cols)
{

#pragma HLS RESOURCE variable=previousframe core=AXIS metadata="-bus_bundle INPUT_PREVIOUS"
#pragma HLS RESOURCE variable=currentframe core=AXIS metadata="-bus_bundle INPUT_CURRENT"

#pragma HLS RESOURCE variable=u core=AXIS metadata="-bus_bundle INPUT_VELX"
#pragma HLS RESOURCE variable=v core=AXIS metadata="-bus_bundle INPUT_VELY"

#pragma HLS RESOURCE variable=uout core=AXIS metadata="-bus_bundle OUTPUT_VELX"
#pragma HLS RESOURCE variable=vout core=AXIS metadata="-bus_bundle OUTPUT_VELY"
#pragma HLS RESOURCE core=AXI_SLAVE variable=rows metadata="-bus_bundle CONTROL_BUS"
#pragma HLS RESOURCE core=AXI_SLAVE variable=cols metadata="-bus_bundle CONTROL_BUS"
#pragma HLS RESOURCE core=AXI_SLAVE variable=return metadata="-bus_bundle CONTROL_BUS"

#pragma HLS interface ap_stable port=rows
#pragma HLS interface ap_stable port=cols

	RGB_IMAGE      curr_frame(rows,cols);
	RGB_IMAGE      dst1_curr_frame(rows,cols);
	RGB_IMAGE      dst2_curr_frame(rows,cols);
	RGB_IMAGE      dst3_curr_frame(rows,cols);
	RGB_IMAGE      prev_frame(rows,cols);
	FLOAT_IMAGE     curr_gray(rows,cols);
	FLOAT_IMAGE     prev_gray(rows,cols);
	GRAY_IMAGE     _dst1(rows,cols);
	GRAY_IMAGE     _dst2(rows,cols);

	FLOAT_IMAGE     blurr_prev_gray(rows,cols);
	FLOAT_IMAGE     blurr_curr_gray(rows,cols);
	FLOAT_IMAGE     Ix_m(rows,cols);
	FLOAT_IMAGE     Iy_m(rows,cols);
	FLOAT_IMAGE     It_m(rows,cols);
	FLOAT_IMAGE     J11(rows,cols);
	FLOAT_IMAGE     J12(rows,cols);
	FLOAT_IMAGE     J13(rows,cols);
	FLOAT_IMAGE     J22(rows,cols);
	FLOAT_IMAGE     J23(rows,cols);
	FLOAT_IMAGE     _u(rows,cols);
	FLOAT_IMAGE     _uout(rows,cols);
	FLOAT_IMAGE     _v(rows,cols);
	FLOAT_IMAGE     _vout(rows,cols);


	FLOAT_IMAGE 	src_d1(rows,cols);
	FLOAT_IMAGE 	src_d2(rows,cols);
	FLOAT_IMAGE 	src_d3(rows,cols);
	FLOAT_IMAGE 	src_d4(rows,cols);
	FLOAT_IMAGE 	It1_m(rows,cols);
	FLOAT_IMAGE 	It2_m(rows,cols);

	FLOAT_IMAGE     Ix0_m(rows,cols);
	FLOAT_IMAGE     Ix1_m(rows,cols);
	FLOAT_IMAGE     Ix2_m(rows,cols);
	FLOAT_IMAGE     Ix3_m(rows,cols);
	FLOAT_IMAGE     Ix4_m(rows,cols);
	FLOAT_IMAGE     Ix5_m(rows,cols);
	FLOAT_IMAGE     Iy0_m(rows,cols);
	FLOAT_IMAGE     Iy1_m(rows,cols);
	FLOAT_IMAGE     Iy2_m(rows,cols);
	FLOAT_IMAGE     Iy3_m(rows,cols);
	FLOAT_IMAGE     Iy4_m(rows,cols);
	FLOAT_IMAGE     Iy5_m(rows,cols);

	FLOAT_IMAGE     It3_m(rows,cols);
	FLOAT_IMAGE     It4_m(rows,cols);

    MY_DATA kernelx[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};
    MY_DATA kernely[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
    MY_DATA ones[2][2] = {{1,1},{1,1}};
    MY_DATA mones[2][2] = {{-1,-1},{-1,-1}};
#pragma HLS dataflow
	hls::AXIvideo2Mat(currentframe,curr_frame);
	hls::CvtColor<HLS_RGB2GRAY>(curr_frame,curr_gray);
	//hls::GaussianBlur<3,3>(curr_gray,blurr_curr_gray);

	hls::AXIvideo2Mat(previousframe,prev_frame);
	hls::CvtColor<HLS_RGB2GRAY>(prev_frame,prev_gray);
	//::GaussianBlur<3,3>(prev_gray,blurr_prev_gray);

	computeDer(curr_gray,prev_gray,Ix_m,Iy_m,It_m,rows,cols);
	CreateTensor(Ix_m,Iy_m,It_m,J11,J12,J13,J22,J23,rows,cols);

	hls::AXIvideo2Mat(u,_u);
	hls::AXIvideo2Mat(v,_v);



	HS(_u,J11,J12,J13,J22,J23,_v,_uout,_vout,rows,cols);

	//Scale(_uout,_dst1,255,0);
	//Scale(_vout,_dst2,255,0);

	hls::Mat2AXIvideo(_uout, uout);
	hls::Mat2AXIvideo(_vout, vout);

}






