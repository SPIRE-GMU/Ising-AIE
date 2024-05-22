#include <adf.h>
#include "kernels.h"
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
using namespace adf;

class simpleGraph : public adf::graph {
private:
  kernel block_00; 
  kernel block_01; 
  kernel block_10; 
  kernel block_11;     
//   parameter lut0;

public:
//   input_plio in0, in1;
//   input_port vectorInput, factorInput; 
//   inout_port vectorOutput, factorOutput;
  output_plio out0;
  input_plio in0;

  output_plio out1;//for time stamp
  simpleGraph()
  {
    // in0 = input_plio::create(plio_32_bits, "data/input_1.txt");
    // in1 = input_plio::create(plio_32_bits, "data/input_1.txt");
    in0 =input_plio::create("DataIn1",adf::plio_32_bits, "data/in_1.txt");
    out0 = output_plio::create("DataOut1",adf::plio_32_bits, "data/output_1.txt");
     out1 = output_plio::create(plio_32_bits, "data/output_2.txt");
    
    block_00 = kernel::create(para_checkboard_00);
    block_01 = kernel::create(para_checkboard_01);
    block_10 = kernel::create(para_checkboard_10);
    block_11 = kernel::create(para_checkboard_11);
    // connect(in0.out[0], first.in[0]); 
    // dimensions(first.in[0]) = { 8 };
    //single_buffer(first.in[0]);               //uncomment for single buffer, by default double buffer will be used.
    connect  (in0.out[0], block_00.in[2]);
    dimensions(block_00.in[2]) = { 8 };
    
     connect  (block_10.out[2], out0.in[0]);
    dimensions(block_10.out[2]) = { 8 };
   
    connect<stream> net0 (block_10.out[0],out1.in[0]);
    

     connect<stream> net2 (block_00.out[0], block_01.in[0]);  //net connections for stream-stream 
     connect<stream> net3 (block_00.out[1], block_10.in[1]);
    
     connect<stream> net4 (block_01.out[0], block_00.in[0]);  //net connections for stream-stream 
     connect<stream> net5 (block_01.out[1], block_11.in[1]);
    
     connect<stream> net6 (block_10.out[0], block_11.in[0]);  //net connections for stream-stream 
     connect<stream> net7 (block_10.out[1], block_00.in[1]);
    
     connect<stream> net8 (block_11.out[0], block_10.in[0]);  //net connections for stream-stream 
     connect<stream> net9 (block_11.out[1], block_01.in[1]);

    source(block_00) = "src/kernels/para_checkboard_00.cc";
    adf::location<kernel>(block_00)=adf::tile(25,1); 

    source(block_01) = "src/kernels/para_checkboard_01.cc";
    adf::location<kernel>(block_01)=adf::tile(26,1);

    source(block_10) = "src/kernels/para_checkboard_10.cc";
    adf::location<kernel>(block_10)=adf::tile(25,0);

    source(block_11) = "src/kernels/para_checkboard_11.cc";
    adf::location<kernel>(block_11)=adf::tile(26,0);

    fifo_depth(net2)=32;
    fifo_depth(net3)=32;
    runtime<ratio>(block_00) = 0.1;
    runtime<ratio>(block_01) = 0.1;
    runtime<ratio>(block_10) = 0.1;
    runtime<ratio>(block_11) = 0.1;
    }
};

