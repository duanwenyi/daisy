
module TH;

   // Clock & reset domain
   wire 	    aclk;
   wire 	    hclk;
   
   wire 	    arest_n;
   wire 	    hrest_n;

   // AHB BUS
   wire [1:0] 	    htrans;      
   wire 	    hwrite;      
   wire [31:0] 	    haddr;       
   wire [31:0] 	    hwdata;      
   wire [2:0] 	    hsize;     
   wire [2:0] 	    hburst;    
   wire [3:0] 	    hprot;     
   wire 	    hready_out;
   
   wire 	    hready_in;
   wire [1:0] 	    hresp; 
   wire [31:0] 	    hrdata;
   
   
   
   // AXI BUS
   wire 	    arready;                     
   wire 	    arvalid;                     
   wire [3:0] 	    arid;        // [3:0]        
   wire [31:0] 	    araddr;                  
   wire [5:0] 	    arlen;       // [5:0]        
   wire [2:0] 	    arsize;      // [2:0]  3'b100
   wire [1:0] 	    arburst;     // [1:0]  2'b01 
   wire 	    arlock;                      
   wire [3:0] 	    arcache;     // [3:0]        
   wire [2:0] 	    arprot;      // [2:0]        
   wire [3:0] 	    arregion;    // [3:0]        
   wire [3:0] 	    arqos;       // [3:0]        
   wire [7:0] 	    aruser;      // [7:0]        
   
   wire 	    rready;                      
   wire 	    rvalid;                      
   wire [3:0] 	    rid;                // [3:0] 
   wire [127:0]     rdata;            // [31:0]
   wire 	    rlast;                       
   wire [1:0] 	    rresp;   // [1:0] 


   wire 	    awready;                     
   wire 	    awvalid;                      
   wire [3:0] 	    awid;        // [3:0]         
   wire [31:0] 	    awaddr;                   
   wire [5:0] 	    awlen;       // [5:0]         
   wire [2:0] 	    awsize;      // [2:0]  3'b100 
   wire [1:0] 	    awburst;     // [1:0]  2'b01  
   wire 	    awlock;                       
   wire [3:0] 	    awcache;     // [3:0]         
   wire [2:0] 	    awprot;      // [2:0]         
   wire [3:0] 	    awregion;    // [3:0]         
   wire [3:0] 	    awqos;       // [3:0]         
   wire [7:0] 	    awuser;      // [7:0]         
   
   wire 	    wready;                       
   wire 	    wvalid;                       
   wire 	    wlast;                        
   wire [3:0] 	    wid;                // [3:0]  
   wire [127:0]     wdata;            // [31:0]    
   wire [15:0] 	    wstrb ;   // [15:0] 

   wire 	    bvalid;
   wire [ 1:0] 	    bresp;
   wire [ 3:0] 	    bid;
   wire 	    bready;

   //Replace regexp (default \(\w+\)\(,\) -> \1^I^I(\1^I),^J): 
   DAISY daisy(
	      .clk		(aclk		),
	      .rest_n		(arest_n	)
	      );
   
   
   IVS_CLK_GEN ivs_clk_gen(
			   // Outputs
			   .aclk	(aclk	),
			   .hclk	(hclk	),
			   .arst_n	(arest_n),
			   .hrst_n	(hrest_n)
			   );

   reg [31:0] 	    cnt;

   always @(posedge aclk or negedge arest_n)
     if(~arest_n)
       cnt    <= 32'b0;
     else
       cnt    <= cnt + 1;

   
endmodule // TH

// Local Variables:
// verilog-library-directories:("."  "../rtl")
// verilog-library-files:("../rtl/ivs_top.v")
// verilog-library-extensions:(".v" ".h")
// End:

