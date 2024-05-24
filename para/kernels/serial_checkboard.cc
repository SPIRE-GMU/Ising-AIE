#include "../kernels.h"
#include <adf.h>
#include <aie_api/aie.hpp>
#include <cstdio>
#include "aie_api/aie_types.hpp"
#include "aie_api/aie_adf.hpp"
#include <aie_api/utils.hpp>

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
  int iter =1000;
  int xup[VOLUME], yup[VOLUME], xdn[VOLUME], ydn[VOLUME];
//   aie::vector<int,32> xup[VOLUME/16], yup[VOLUME/16], xdn[VOLUME/16], ydn[VOLUME];
  float beta = 0.5;
  float esum,mag;
//   aie::vector<float,32> s[VOLUME/16];
  float s[VOLUME];

  float esumt, magt;



  /* Set random seed for erand */
    unsigned short int seed[3];
    seed[0]=13; seed[1]=35; seed[2]=17;

  /* Initialize each point randomly */
    for(i = 0;i < VOLUME;i++) chess_prepare_for_pipelining{
      
      if(rand(seed) < 0.5){
        // s[i/16].set(1.0,i%16);
        s[i]=1.0;
      }
      else {
        // s[i/16].set(-1.0,i%16);
        s[i]=-1.0;
      }
            
    }
    //    aie::print(s[1],true,"s[1]=");


  /* Create and index of neighbours
     The sites are partitioned to even an odd,
     with even sites first in the array */
  for( j=0; j<N2; j++) for( i=0; i<N1; i++)chess_prepare_for_pipelining {
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
// aie::tile tile=aie::tile::current(); //get the tile of the kernel
float time1=get_cycles();


  /* Initialize the measurements */
  esumt = 0.0;
  magt = 0.0;

  /* Run a number of iterations */
  for(n = 0;n < iter;n++) chess_prepare_for_pipelining{
    esum = 0.0;
    mag = 0.0;
    v8float esumv,magv;

    /* Loop over the lattice and try to flip each atom */
    for(int k = 0;k < VOLUME/8;k++) chess_prepare_for_pipelining{
        //    /*---------------serial-----------------*/
        //   float new_energy, energy_now, deltae;
        //   float stmp;
        //   float neighbours = s[xup[k]] + s[yup[k]] + s[xdn[k]] + s[ydn[k]];
       
        //    stmp = -s[k];

        //   /* Find the energy before and after the flip */
        //   energy_now = -s[k]*neighbours;
        //   new_energy = -stmp*neighbours;
        //   deltae = new_energy-energy_now;
        //   /* Accept or reject the change */
        //  //   if( exp(-beta*deltae) > rand(seed) ){ //exp doesnot work
        //     if( expand(-beta*deltae)> rand(seed) ){
        //       s[k] = stmp;
        //       energy_now = new_energy;
        //     }

        //   /* Measure magnetisation and energy */
        //   mag = mag + s[k];
        //   esum = esum + energy_now;

    //    /*---------------16 para api 0.438 sec-----------------*/
    //    aie::vector<float, 16> new_energy,energy_now,deltae;
    //    aie::vector<float, 16> stmp;
    //    aie::vector<float, 16> sv;
    //    aie::vector<float, 16> neighbours;
    //    aie::vector<float, 16> xupv,yupv,xdnv,ydnv;
    //         for(int v=0;v<16;v++)chess_prepare_for_pipelining{ //fill the vector
    //             xupv.set(s[xup[v+k*16]],v);
    //             yupv.set(s[yup[v+k*16]],v);
    //             xdnv.set(s[xdn[v+k*16]],v);
    //             ydnv.set(s[ydn[v+k*16]],v);
    //         }   
    //     neighbours = aie::add(xupv,yupv) ;  
    //     neighbours = aie::add(neighbours,xdnv) ; 
    //     neighbours = aie::add(neighbours,ydnv) ;       
    //     //aie::print(neighbours,true,"neighbours=");
    //     for(int v=0;v<16;v++)chess_prepare_for_pipelining{ //fill the vector
    //             sv.set(s[v+k*16],v);               
    //         } 
    //     stmp = aie::neg(sv);
    //     energy_now = aie::mul(stmp,neighbours);
    //     new_energy = aie::mul(sv,neighbours);
    //     deltae = aie::sub(new_energy,energy_now);
    //     for(int v=0;v<16;v++)chess_prepare_for_pipelining{ //update the vector
    //             if( expand(-beta*deltae[v])> rand(seed) ){
    //                 s[v+k*16] = stmp[v];
    //                 energy_now[v] = new_energy[v];
    //             }                             
    //         } 
    //     mag = aie::reduce_add(sv);
    //     esum = esum + aie::reduce_add(energy_now);
        /*---------------8 para intrinsics-----------------*/
       v8float new_energy,energy_now,deltae;
       v8float stmp;
       v8float sv;
       v8float neighbours;
       v8float xupv,yupv,xdnv,ydnv;
            for(int v=0;v<8;v++)chess_prepare_for_pipelining{ //fill the vector              
                xupv = upd_elem(xupv,v,s[xup[v+k*8]]);
                yupv = upd_elem(yupv,v,s[xup[v+k*8]]);
                xdnv = upd_elem(xdnv,v,s[xup[v+k*8]]);
                ydnv = upd_elem(ydnv,v,s[xup[v+k*8]]);
                sv = upd_elem(sv,v,s[v+k*8]);
            }   
            // printf("%f",ext_elem(sv,2));
        neighbours = fpadd(xupv,yupv) ;  
        neighbours = fpadd(neighbours,xdnv) ; 
        neighbours = fpadd(neighbours,ydnv) ; 
        // printf("%f",ext_elem(neighbours,2));      
        
        stmp = fpneg(sv);
        energy_now = fpmul(stmp,neighbours);
        new_energy = fpmul(sv,neighbours);
        deltae = fpsub(new_energy,energy_now);
        for(int v=0;v<8;v++)chess_prepare_for_pipelining{ //update the vector
                if( expand(-beta*ext_elem(deltae,v))> rand(seed) ){
                    s[v+k*8] = ext_elem(stmp,v);
                    upd_elem(energy_now,v,ext_elem(new_energy,v));                    
                } 
                mag = mag + s[v+k*8];
                esum = esum + ext_elem(energy_now,v);
            } 
     
    }

     

    /* Calculate measurements and add to run averages  */
    esum = esum/(VOLUME);
    mag = mag/(VOLUME);
    esumt = esumt + esum;
    magt = magt + fabs(mag);

    //printf("average energy = %f, average magnetization = %f\n",esum,mag);
  }
  
  float time2=get_cycles(); //second time
  float time=time2-time1;  
    

  esumt = esumt/iter;
  magt = magt/iter;
//   printf("Over the whole simulation:\n");
//   printf("average energy = %f, average magnetization = %f\n", esumt, magt);
    
   
    
    

    writeincr(dout, esumt);
    //writeincr(dout, magt);
    writeincr(dout,time/1250000000);
   
}
