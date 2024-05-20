#include "../kernels.h"
// #include "adf/x86sim/streamApi.h"
// #include "../data.h"
// #include <stdio.h>
// #include <math.h>
// #include <stdlib.h>


#define POLY_MASK_32 0xB4BCD35C
#define POLY_MASK_31 0x7A5BC2E3
uint32_t lfsr32 , lfsr31;

/* I have to replace erand48 with other pseudorandom function */

int shift_lfsr(uint32_t *lfsr,uint32_t polynomial_mask)
{
    int feedback;
    feedback = *lfsr &1;
    *lfsr = *lfsr >> 1;
    if(feedback ==1)
        *lfsr = *lfsr ^ polynomial_mask;
    return *lfsr;    
}

void init_lfsrs(void){
    lfsr32 = 0xABCDE;
    lfsr31= 0x23456789;
}

int get_random(void){
    shift_lfsr(&lfsr32, POLY_MASK_32);
    return (shift_lfsr(&lfsr32, POLY_MASK_32)^ shift_lfsr(&lfsr31, POLY_MASK_31)) & 0xffff;
}

float rand(unsigned short int seed[3]){
    float a;    
    int value ;
    
    value = get_random();
    a= (float)(value);
    //printf("%f",a);
    return a/32767.0; //devided with 0x7fff to scale size between 0-1
}

inline float expand(float x){
    x = 1.0 + x/256;

          for(int i=0;i<8;i++){
              x = x*x;
          }

           return x;

}



void serial_checkboard(  input_stream_float* din, output_stream_float* dout ){
 
  
    float a1 = readincr(din);
    float a2 = readincr(din);

  init_lfsrs(); //initialize the random number

  int n,i,j;
  int iter =2000;
  int xup[VOLUME], yup[VOLUME], xdn[VOLUME], ydn[VOLUME];
  float beta = 0.5;
  float esum,mag;
  float s[VOLUME];
  //FILE *fp;

  float esumt, magt;

  /* Read parameters. Beta is the inverse of the temperature */
  //fp = fopen("gold.txt","w");
  


  /* Set random seed for erand */
    unsigned short int seed[3];
    seed[0]=13; seed[1]=35; seed[2]=17;

  /* Initialize each point randomly */
    for(i = 0;i < VOLUME;i++) {
      if(rand(seed) < 0.5){
        s[i] = 1.0;
      }
      else {
        s[i] = -1.0;
      }
    //   printf("%f,\n",s[i]);
    }



  /* Create and index of neighbours
     The sites are partitioned to even an odd,
     with even sites first in the array */
  for( j=0; j<N2; j++) for( i=0; i<N1; i++) {
    int i2 = i/2;
    int is = i2 + j*N1d2;
    if( (i+j)%2 == 0 ) {
	    xup[is] = ((i+1)/2)%N1d2 + j*N1d2 + VOLUMEd2;
	    yup[is] = i2 + ((j+1)%N2)*N1d2 + VOLUMEd2;
	    xdn[is] = ((i-1+N1)/2)%N1d2 + j*N1d2 + VOLUMEd2;
	    ydn[is] = i2 + ((j-1+N2)%N2)*N1d2 + VOLUMEd2;
    }
    else {
	    xup[is+VOLUMEd2] = ((i+1)/2)%N1d2 + j*N1d2;
	    yup[is+VOLUMEd2] = i2 + ((j+1)%N2)*N1d2;
	    xdn[is+VOLUMEd2] = ((i-1+N1)/2)%N1d2 + j*N1d2;
	    ydn[is+VOLUMEd2] = i2 + ((j-1+N2)%N2)*N1d2;
    }
  }

  /* Initialize the timestamp */
    // struct timeval start; 
    // struct timeval end; 


  /* Initialize the measurements */
  esumt = 0.0;
  magt = 0.0;

  /* Run a number of iterations */
  for(n = 0;n < iter;n++) {
    esum = 0.0;
    mag = 0.0;

    /* Loop over the lattice and try to flip each atom */
    for(int k = 0;k < VOLUME;k++) {
      float new_energy, energy_now, deltae;
      float stmp;
      float neighbours = s[xup[k]] + s[yup[k]] + s[xdn[k]] + s[ydn[k]];
     ////////////////////////////////////////////////////////////    


       stmp = -s[k];

      /* Find the energy before and after the flip */
      energy_now = -s[k]*neighbours;
      new_energy = -stmp*neighbours;
      deltae = new_energy-energy_now;
      /* Accept or reject the change */
    //   if( exp(-beta*deltae) > rand(seed) ){
        if( expand(-beta*deltae)> rand(seed) ){
	      s[k] = stmp;
	      energy_now = new_energy;
      }

      /* Measure magnetisation and energy */
      mag = mag + s[k];
      esum = esum + energy_now;
    }

    /* Calculate measurements and add to run averages  */
    esum = esum/(VOLUME);
    mag = mag/(VOLUME);
    esumt = esumt + esum;
    magt = magt + fabs(mag);

    //printf("average energy = %f, average magnetization = %f\n",esum,mag);
  }

  esumt = esumt/iter;
  magt = magt/iter;
  printf("Over the whole simulation:\n");
  printf("average energy = %f, average magnetization = %f\n", esumt, magt);
    
   
    
    

    writeincr(dout, esumt);
    writeincr(dout, magt);

   
}
