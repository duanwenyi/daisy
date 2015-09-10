//-----------------------------------------------------------------------------
// Title         : DAISY
// Project       : daisy
//-----------------------------------------------------------------------------
// File          : daisy.sv
// Author        : Apachee
// Created       : 10.09.2015
// Last modified : 10.09.2015
//-----------------------------------------------------------------------------
// Description :
//  Daisy top module
//-----------------------------------------------------------------------------
// Copyright (c) 2015 by SpAcW This model is the confidential and
// proprietary property of SpAcW and the possession or use of this
// file requires a written license from SpAcW.
//------------------------------------------------------------------------------
// Modification history :
// 10.09.2015 : created
//-----------------------------------------------------------------------------



module DAISY(/*AUTOARG*/
   // Inputs
   clk, rest_n
   );


   import "DPI-C" function void daisy_hdl_init();
   import "DPI-C" function void daisy_monitor();

   
   input                    clk;
   input 		    rest_n;

  
   initial begin
      daisy_hdl_init();
   end

   always @(posedge clk)
     #1ps daisy_monitor();
   
endmodule
