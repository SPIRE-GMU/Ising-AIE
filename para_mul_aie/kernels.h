#include <adf.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifndef FUNCTION_KERNELS_H
#define FUNCTION_KERNELS_H

//320
#define N1 32
#define N1d2 (N1/2)
#define N2 32

#define VOLUME N1*N2
#define VOLUMEd2 N1d2*N2

using namespace adf;

void para_checkboard_00(  input_stream<int16> *din0,output_stream<int16> *dout0, input_stream<int16> *din1,output_stream<int16> *dout1,input_buffer<int32> &start );
void para_checkboard_01(  input_stream<int16> *din0,output_stream<int16> *dout0, input_stream<int16> *din3,output_stream<int16> *dout3 );
void para_checkboard_10( input_stream<int16> *din2,output_stream<int16> *dout2, input_stream<int16> *din1,output_stream<int16> *dout1 , output_buffer<float>&out);
void para_checkboard_11(  input_stream<int16> *din2,output_stream<int16> *dout2, input_stream<int16> *din3,output_stream<int16> *dout3 );


// extern int32 LUT[NUM_SAMPLES];

#endif
