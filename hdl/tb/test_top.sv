
module TH;

   // Clock & reset domain
   wire 	    clk;
   wire 	    rst_n;

   /*autowire*/
   // Beginning of automatic wires (for undeclared instantiated-module outputs)
   bit					conflict_c;				// From enigma_sim of ENIGMA_SIM.v
   bit [4:0]			id_a;					// From enigma_sim of ENIGMA_SIM.v
   bit [4:0]			id_b;					// From enigma_sim of ENIGMA_SIM.v
   wire [5:0]			id_c;					// From enigma_buffer of ENIGMA_BUFFER.v
   bit [127:0]			payload_a;				// From enigma_sim of ENIGMA_SIM.v
   bit [127:0]			payload_b;				// From enigma_sim of ENIGMA_SIM.v
   wire [127:0]			payload_c;				// From enigma_buffer of ENIGMA_BUFFER.v
   bit [1:0]			qos_a;					// From enigma_sim of ENIGMA_SIM.v
   bit [1:0]			qos_b;					// From enigma_sim of ENIGMA_SIM.v
   wire [1:0]			qos_c;					// From enigma_buffer of ENIGMA_BUFFER.v
   wire					ready_a;				// From enigma_buffer of ENIGMA_BUFFER.v
   wire					ready_b;				// From enigma_buffer of ENIGMA_BUFFER.v
   bit					ready_c;				// From enigma_sim of ENIGMA_SIM.v
   bit					release_c;				// From enigma_sim of ENIGMA_SIM.v
   bit [5:0]			releaseid_c;			// From enigma_sim of ENIGMA_SIM.v
   bit					valid_a;				// From enigma_sim of ENIGMA_SIM.v
   bit					valid_b;				// From enigma_sim of ENIGMA_SIM.v
   wire					valid_c;				// From enigma_buffer of ENIGMA_BUFFER.v
   // End of automatics

   //Replace regexp (default \(\w+\)\(,\) -> \1^I^I(\1^I),^J): 
   DAISY daisy(
			   .clk		(clk	),
			   .rest_n	(rst_n	)
			   );
   
   
   IVS_CLK_GEN ivs_clk_gen(
						   // Outputs
						   .clk		(clk),
						   .rst_n	(rst_n)
						   );

   reg [31:0] 	cnt;
   always @(posedge clk or negedge rst_n)
     if(~rst_n)
       cnt    <= 32'b0;
     else if(valid_c & ready_c)
       cnt    <= 32'b0;
     else
       cnt    <= cnt + 1;

   always @(posedge clk)
     if( cnt > 32'd1000)
       $finish();

   ENIGMA_BUFFER enigma_buffer(/*autoinst*/
							   // Outputs
							   .ready_a			(ready_a),
							   .ready_b			(ready_b),
							   .valid_c			(valid_c),
							   .payload_c		(payload_c[127:0]),
							   .id_c			(id_c[5:0]),
							   .qos_c			(qos_c[1:0]),
							   // Inputs
							   .clk				(clk),
							   .rst_n			(rst_n),
							   .payload_a		(payload_a[127:0]),
							   .id_a			(id_a[4:0]),
							   .qos_a			(qos_a[1:0]),
							   .valid_a			(valid_a),
							   .payload_b		(payload_b[127:0]),
							   .id_b			(id_b[4:0]),
							   .qos_b			(qos_b[1:0]),
							   .valid_b			(valid_b),
							   .conflict_c		(conflict_c),
							   .release_c		(release_c),
							   .releaseid_c		(releaseid_c[5:0]),
							   .ready_c			(ready_c));

   
   ENIGMA_SIM enigma_sim(/*autoinst*/
						 // Outputs
						 .payload_a				(payload_a[127:0]),
						 .id_a					(id_a[4:0]),
						 .qos_a					(qos_a[1:0]),
						 .valid_a				(valid_a),
						 .payload_b				(payload_b[127:0]),
						 .id_b					(id_b[4:0]),
						 .qos_b					(qos_b[1:0]),
						 .valid_b				(valid_b),
						 .conflict_c			(conflict_c),
						 .release_c				(release_c),
						 .releaseid_c			(releaseid_c[5:0]),
						 .ready_c				(ready_c),
						 // Inputs
						 .clk					(clk),
						 .rst_n					(rst_n),
						 .ready_a				(ready_a),
						 .ready_b				(ready_b),
						 .valid_c				(valid_c),
						 .payload_c				(payload_c[127:0]),
						 .id_c					(id_c[5:0]),
						 .qos_c					(qos_c[1:0]));
   
   
endmodule // TH

// Local Variables:
// verilog-library-directories:("./"  "../rtl")
// verilog-library-files:("enigma_sim.sv" "../rtl/enigma_buffer.v" )
// verilog-library-extensions:(".v" ".sv" ".h")
// End:
