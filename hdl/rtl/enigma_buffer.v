
module ENIGMA_BUFFER(/*autoarg*/
   // Outputs
   ready_a, ready_b, valid_c, payload_c, id_c, qos_c,
   // Inputs
   clk, rst_n, payload_a, id_a, qos_a, valid_a, payload_b, id_b,
   qos_b, valid_b, conflict_c, release_c, releaseid_c, ready_c
   );

   
    input  clk;
    input  rst_n;

    // port A
    input bit [127:0] payload_a;
    input bit [4:0]   id_a;
    input bit [1:0]   qos_a;
    input bit         valid_a;
    output bit        ready_a;

    // port B
    input bit [127:0] payload_b;
    input bit [4:0]   id_b;
    input bit [1:0]   qos_b;
    input bit         valid_b;
    output bit        ready_b;

    // output bit port C
    input bit         conflict_c;
    input bit         release_c;
    input bit [5:0]   releaseid_c;

    input bit         ready_c;
    output bit        valid_c;
    
    output bit [127:0] payload_c;
    output bit [5:0]   id_c;
    output bit [1:0]   qos_c;

    // debug using
    reg [31:0]         tick;
    bit [5:0]          chain_id;
    bit [5:0]          last_chain_id;
    bit [6:0]          chain_size;
    bit                pre_out_vld;
    bit [1:0]          max_qos;
    bit                dim_qos_en;
    bit                portSel;
    bit [5:0]          nptr;
    bit [5:0]          nnptr;
    bit [5:0]          head_ptr;
    bit [5:0]          tail_ptr;
    
    chandle            app;

    import "DPI-C" function chandle enigma_buf_init();

    import "DPI-C" function void enigma_buf_port_o( chandle       app,
                                                    output        ready_a, 
                                                    output        ready_b, 

                                                    output [31:0] payload_c_0,
                                                    output [31:0] payload_c_1,
                                                    output [31:0] payload_c_2,
                                                    output [31:0] payload_c_3,
    
                                                    output [4:0]  id_c,
                                                    output [1:0]  qos_c,
                                                    output        valid_c,
                                                    // debug using
                                                    output [11:0]  chain_id,
                                                    output [6:0]  chain_size,
                                                    output        pre_out_vld,
                                                    output [1:0]  max_qos,
                                                    output        dim_qos_en,
                                                    output        portSel,
                                                    output [23:0] nptr

                                                    );
    


    import "DPI-C" function void enigma_buf_port_i( chandle      app,
                                                    input [31:0] payload_a_0,
                                                    input [31:0] payload_a_1,
                                                    input [31:0] payload_a_2,
                                                    input [31:0] payload_a_3,
                                                    input [4:0]  id_a,
                                                    input [1:0]  qos_a,
                                                    input        valid_a,

                                                    input [31:0] payload_b_0,
                                                    input [31:0] payload_b_1,
                                                    input [31:0] payload_b_2,
                                                    input [31:0] payload_b_3,
                                                    input [4:0]  id_b,
                                                    input [1:0]  qos_b,
                                                    input        valid_b,
    
                                                    input        ready_c,
                                                    input        conflict_c,
                                                    input        release_c,
                                                    input [5:0]  releaseid_c,
                                                    
                                                    // debug using
                                                    input [31:0] tick
                                                    );


    initial begin
        app = enigma_buf_init();
    end
    
    always @(posedge clk)
      if(~rst_n)
        tick     <= 32'b0;
      else
        tick     <= tick + 1;

    always @(posedge clk)
      if(rst_n)
        #0.1ns enigma_buf_port_o( app,
                                  ready_a,
                                  ready_b,

                                  payload_c[ 31: 0],
                                  payload_c[ 63:32],
                                  payload_c[ 95:64],
                                  payload_c[127:96],
                                  id_c,
                                  qos_c,
                                  valid_c,
                                  // debug using
                                  {last_chain_id, chain_id},
                                  chain_size,
                                  pre_out_vld,
                                  max_qos,
                                  dim_qos_en,
                                  portSel,
                                  {tail_ptr, head_ptr, nnptr, nptr}
                                  );

    always @(posedge clk)
      if(rst_n)
        enigma_buf_port_i( app,
                           payload_a[ 31: 0],
                           payload_a[ 63:32],
                           payload_a[ 95:64],
                           payload_a[127:96],
                           id_a,
                           qos_a,
                           valid_a,

                           payload_b[ 31: 0],
                           payload_b[ 63:32],
                           payload_b[ 95:64],
                           payload_b[127:96],
                           id_b,
                           qos_b,
                           valid_b,

                           ready_c,
        
                           conflict_c,
                           release_c,
                           releaseid_c,

                           tick
                           );

endmodule
