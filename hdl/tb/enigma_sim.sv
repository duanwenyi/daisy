
module ENIGMA_SIM(/*autoarg*/
   // Outputs
   payload_a, id_a, qos_a, valid_a, payload_b, id_b, qos_b, valid_b,
   conflict_c, release_c, releaseid_c, ready_c,
   // Inputs
   clk, rst_n, ready_a, ready_b, valid_c, payload_c, id_c, qos_c
   );
   
   input 		 clk;
   input 		 rst_n;

   parameter     TEST_ID = 0;
   parameter     DLY     = 0.1ns;

   // port A
   output bit [127:0] payload_a;
   output bit [4:0]   id_a;
   output bit [1:0]   qos_a;
   output bit  		  valid_a;
   input 			  ready_a;

   // port B
   output bit [127:0] payload_b;
   output bit [4:0]   id_b;
   output bit [1:0]   qos_b;
   output bit  		  valid_b;
   input 			  ready_b;

   // input port C
   output bit  		  conflict_c;
   output bit  		  release_c;
   output bit [5:0]   releaseid_c;

   output bit  		  ready_c;
   input 			  valid_c;
   
   input [127:0] 	  payload_c;
   input [5:0] 		  id_c;
   input [1:0] 		  qos_c;

   bit 				  error;
 
   
   chandle 		  app;

   import "DPI-C" function chandle enigma_init( int TEST_ID);

   import "DPI-C" function void enigma_port_com_sim( chandle      app,
													 input 		  ready_a, 
													 input 		  ready_b, 

													 input [31:0] payload_c_0,
													 input [31:0] payload_c_1,
													 input [31:0] payload_c_2,
													 input [31:0] payload_c_3,
													 
													 input [4:0]  id_c,
													 input [1:0]  qos_c,
													 input 		  valid_c
													 );
   
													 

   import "DPI-C" function void enigma_port_A_sim( chandle       app,
												   output [31:0] payload_a_0,
												   output [31:0] payload_a_1,
												   output [31:0] payload_a_2,
												   output [31:0] payload_a_3,
												   output [4:0]  id_a,
												   output [1:0]  qos_a,
												   output 		 valid_a
												   );

   import "DPI-C" function void enigma_port_B_sim( chandle       app,
												   output [31:0] payload_b_0,
												   output [31:0] payload_b_1,
												   output [31:0] payload_b_2,
												   output [31:0] payload_b_3,
												   output [4:0]  id_b,
												   output [1:0]  qos_b,
												   output 		 valid_b
												   );

   import "DPI-C" function void enigma_port_C_sim( chandle       app,
												   output 		 ready_c,

												   output 		 conflict_c,
												   output 		 release_c,
												   output [5:0]  releaseid_c,
												   
												   output 		 error
												   );

   initial begin
	  app = enigma_init(TEST_ID);
   end
   
   always @(posedge clk)
	 if(~rst_n)
	   enigma_port_com_sim( app,
							ready_a,
							ready_b,

							payload_c[ 31: 0],
							payload_c[ 63:32],
							payload_c[ 95:64],
							payload_c[127:96],
							id_c,
							qos_c,
							valid_c
							);

   always @(posedge clk)
	 if(~rst_n)
	   #DLY enigma_port_A_sim( app,
							   payload_a[ 31: 0],
							   payload_a[ 63:32],
							   payload_a[ 95:64],
							   payload_a[127:96],
							   id_a,
							   qos_a,
							   valid_a
							   );

   always @(posedge clk)
	 if(~rst_n)
	   #DLY enigma_port_B_sim( app,
							   payload_b[ 31: 0],
							   payload_b[ 63:32],
							   payload_b[ 95:64],
							   payload_b[127:96],
							   id_b,
							   qos_b,
							   valid_b
							   );

   always @(posedge clk)
	 if(~rst_n)
	   #DLY enigma_port_C_sim( app,
							   ready_c,
 	   
							   conflict_c,
							   release_c,
							   releaseid_c,
	   
							   error
							   );

endmodule
