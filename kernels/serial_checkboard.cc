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
uint32_t lfsr32, lfsr31;
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



void para_checkboard(  input_stream_float* din, output_stream_float* dout ){
 
  
    float a1 = readincr(din);
    float a2 = readincr(din);

  init_lfsrs(); //initialize the random number

  int n,i,j;
  int iter =2000;
//   int xup[VOLUME], yup[VOLUME], xdn[VOLUME], ydn[VOLUME];
  v16int16 xupv[VOLUME/16], yupv[VOLUME/16], xdnv[VOLUME/16], ydnv[VOLUME/16];
  float beta = 0.5;
  float esum,mag;
  v16int16 s[VOLUME/16];
//   float s[VOLUME];

  float esumt, magt;



  /* Set random seed for erand */
    unsigned short int seed[3];
    seed[0]=13; seed[1]=35; seed[2]=17;

  /* Initialize each point randomly */
    for(i = 0;i < VOLUME;i++) chess_prepare_for_pipelining{
      
      if(rand(seed) < 0.5){
        s[i/16] = upd_elem(s[i/16],i%16,1);
        // s[i]=1.0;
      }
      else {
        s[i/16] = upd_elem(s[i/16],i%16,-1);
        // s[i]=-1.0;
      }
           
    }
    // for(int v=0;v<8;v++)
    //  printf("s[2]=%d\n",ext_elem(s[2],v)); 


  /* Create and index of neighbours
     The sites are partitioned to even an odd,
     with even sites first in the array */
  for( j=0; j<N2; j++) for( i=0; i<N1; i++)chess_prepare_for_pipelining {
    int xup,yup,xdn,ydn;
    int i2 = i/2;
    int is = i2 + j*N1d2;
    if( (i+j)%2 == 0 ) {


        xup = ((i+1)/2)%N1d2 + j*N1d2 + VOLUMEd2;
	    yup = i2 + ((j+1)%N2)*N1d2 + VOLUMEd2;
	    xdn = ((i-1+N1)/2)%N1d2 + j*N1d2 + VOLUMEd2;
	    ydn = i2 + ((j-1+N2)%N2)*N1d2 + VOLUMEd2;

        xupv[(i+j*N1d2)/16] = upd_elem(xupv[(i+j*N1d2)/16],(i+j*N1d2)%16,ext_elem(s[xup/16],xup%16));
        yupv[(i+j*N1d2)/16] = upd_elem(yupv[(i+j*N1d2)/16],(i+j*N1d2)%16,ext_elem(s[yup/16],yup%16));
        xdnv[(i+j*N1d2)/16] = upd_elem(xdnv[(i+j*N1d2)/16],(i+j*N1d2)%16,ext_elem(s[xdn/16],xdn%16));
        ydnv[(i+j*N1d2)/16] = upd_elem(ydnv[(i+j*N1d2)/16],(i+j*N1d2)%16,ext_elem(s[ydn/16],ydn%16));

        // printf("%f",ext_elem(xupv[(i+j)/8],(i+j)%8));
    }
    
    else {

        xup = ((i+1)/2)%N1d2 + j*N1d2;
	    yup = i2 + ((j+1)%N2)*N1d2;
	    xdn = ((i-1+N1)/2)%N1d2 + j*N1d2;
	    ydn = i2 + ((j-1+N2)%N2)*N1d2;

        xupv[(i+j*N1d2)/16] = upd_elem(xupv[(i+j*N1d2)/16],(i+j*N1d2)%16,ext_elem(s[xup/16],xup%16));
        yupv[(i+j*N1d2)/16] = upd_elem(yupv[(i+j*N1d2)/16],(i+j*N1d2)%16,ext_elem(s[yup/16],yup%16));
        xdnv[(i+j*N1d2)/16] = upd_elem(xdnv[(i+j*N1d2)/16],(i+j*N1d2)%16,ext_elem(s[xdn/16],xdn%16));
        ydnv[(i+j*N1d2)/16] = upd_elem(ydnv[(i+j*N1d2)/16],(i+j*N1d2)%16,ext_elem(s[ydn/16],ydn%16));

    }
  }
        
    //    for(int v=0;v<8;v++){
    //         printf("xupv[92]=%d\n",ext_elem(xupv[2],v)); 
    //         printf("xupv[92]=%d\n",ext_elem(yupv[2],v)); 
    //         printf("xupv[92]=%d\n",ext_elem(xdnv[2],v)); 
    //         printf("xupv[92]=%d\n",ext_elem(ydnv[2],v)); 
    //         }
  /* Initialize the timestamp */
// aie::tile tile=aie::tile::current(); //get the tile of the kernel
float time1=get_cycles();


  /* Initialize the measurements */
  esumt = 0.0;
  magt = 0.0;
     int sum=0;  
  /* Run a number of iterations */
  for(n = 0;n < iter;n++) chess_prepare_for_pipelining{
    esum = 0.0;
    mag = 0.0;
    v16int16 magv,esumv;

    /* Loop over the lattice and try to flip each atom */
    for(int k = 0;k < VOLUME/16;k++) chess_prepare_for_pipelining{
        
        /*---------------8 para intrinsics-----------------*/
        v16int16 new_energy,energy_now,deltae;
        // v16int16 stmp;
        // v16int16 sv;
        v16int16 neighbours,stmp;
 
        neighbours = operator+(xupv[k],yupv[k]) ;  
        neighbours = operator+(neighbours,xdnv[k]) ; 
        neighbours = operator+(neighbours,ydnv[k]) ; 
        
        stmp = operator-(stmp,stmp);
        stmp = operator-(stmp,s[k]);
        

        energy_now = srs(mul(stmp,neighbours),0); // extract v16 because mul32 is not permitted

        new_energy = srs(mul(s[k],neighbours),0);
        // if(n==2 && k==0){
        //         printf("sk[0]=%d,stmp=%d,neighbor=%d,energy_now  = %d, new_energy = %d\n",ext_elem(s[k],1), ext_elem(stmp,1),ext_elem(neighbours,1), ext_elem(energy_now,1),ext_elem(new_energy,1));
        // }

        deltae = operator-(new_energy,energy_now);
        
    
        for(int v=0;v<16;v++){
            sum = sum + ext_elem(deltae,v); //update the value in parallel
        }

        if (expand(-beta*sum)> 16*rand(seed)){
            s[k] = stmp;
            energy_now = new_energy;
            if(k<32){
                xdnv[k+32] = s[k];
                xupv[k+32] = s[k];
                yupv[k+31] = s[k];
                ydnv[k+33] = s[k];
            }else {
                xdnv[k-32] = s[k];
                xupv[k-32] = s[k];
                yupv[k-33] = s[k];
                ydnv[k-31] = s[k];
            }
            
            
        }

        magv = operator-(magv,magv);
        esumv = operator-(esumv,esumv);

        magv = operator+(magv, s[k]);
        esumv = operator+(esumv,energy_now);
        // for(int v=0;v<16;v++)chess_prepare_for_pipelining{ //update the vector
        //         if( expand(-beta*ext_elem(deltae,v))> rand(seed) ){
        //             s[k]= upd_elem(s[k],v,ext_elem(stmp,v));                   
        //             energy_now= upd_elem(energy_now,v,ext_elem(new_energy,v));  
        //             }
        //         mag = mag + ext_elem(s[k],v);
        //         esum = esum + ext_elem(energy_now,v);
            
        //     } 

     }


    for(int v=0;v<16;v++){
            esum = esum + ext_elem(esumv,v);
            mag = mag + ext_elem(magv,v);
    }

    /* Calculate measurements and add to run averages  */
    esum = esum/(VOLUME);  
    mag = mag/(VOLUME);
    // printf("n==%d,mag=%f,esum=%f",n,mag,esum);
    esumt = esumt + esum;
    magt = magt + fabs(mag);

    //printf("average energy = %f, average magnetization = %f\n",esum,mag);
    
  }
  float time2=get_cycles(); //second time
  float time=time2-time1;  
    

  esumt = esumt/iter;
  magt = magt/iter;
  printf("Over the whole simulation:\n");
  printf("average energy = %f, average magnetization = %f\n", esumt, magt);
    
   
    
    

    //writeincr(dout, esumt);
    writeincr(dout, magt);
    writeincr(dout,time/1250000000);
   
}
