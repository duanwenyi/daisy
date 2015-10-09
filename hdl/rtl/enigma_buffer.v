
module ENIGMA_BUFFER(/*autoarg*/
   // Outputs
   ready_a, ready_b, valid_c, payload_c, id_c, qos_c,
   // Inputs
   clk, rst_n, payload_a, id_a, qos_a, valid_a, payload_b, id_b,
   qos_b, valid_b, conflict_c, release_c, releaseid_c, ready_c
   );

   
   input 		 clk;
   input 		 rst_n;

   // port A
   input [127:0] payload_a;
   input [4:0] 	 id_a;
   input [1:0] 	 qos_a;
   input 		 valid_a;
   output 		 ready_a;

   // port B
   input [127:0] payload_b;
   input [4:0] 	 id_b;
   input [1:0] 	 qos_b;
   input 		 valid_b;
   output 		 ready_b;

   // output port C
   input 		 conflict_c;
   input 		 release_c;
   input [5:0] 	 releaseid_c;

   input 		 ready_c;
   output 		 valid_c;
   
   output [127:0] payload_c;
   output [5:0]   id_c;
   output [1:0]   qos_c;
   
   
   
   

endmodule
