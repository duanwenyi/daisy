
module IVS_CLK_GEN(/*AUTOARG*/
   // Outputs
   clk, rst_n
   );

   output reg     clk;
   output reg 	  rst_n;


   initial begin
      clk = 1'b0;
      #3ns;
      // 500 MHz
      forever begin
		 #1ns;
		 clk = ~clk;
      end
   end
   
   initial begin
      rst_n  = 1'b0;
      #201.5ns;
      rst_n  = 1'b1;
   end
  
   
endmodule // IVS_CLK_GEN

