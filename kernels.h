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

void serial_checkboard( input_stream_float* din,output_stream_float* dout );



// extern int32 LUT[NUM_SAMPLES];

#endif
