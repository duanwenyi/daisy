module ENIGMA_MEM (/*AUTOARG*/
    // Outputs
    o_payload,
    // Inputs
    clk, rst_n, load_sel, fetch_sel, i_payload
    );
    parameter  ENIGMA_CELL_MAX = 24;
    input                        clk;
    input                        rst_n;

    input [ENIGMA_CELL_MAX-1:0]  load_sel;

    input [ENIGMA_CELL_MAX-1:0]  fetch_sel;
    
    
    input [127:0]                i_payload;
    output [127:0]               o_payload;

    reg [127:0]                  mem[ENIGMA_CELL_MAX-1:0];

    generate
        genvar                   cc;
        for(cc=0; cc<ENIGMA_CELL_MAX; cc=cc+1)begin:ENIGMA_MEM_WRITE
            always @(posedge clk)
              if(load_sel[cc])
                mem[cc]    <= i_payload;
        end
    endgenerate
    
    wire [127:0]               o_payload_s[ENIGMA_CELL_MAX-1:0];

    generate
        genvar                   i;
        for(i=0; i<ENIGMA_CELL_MAX; i=i+1)begin:ENIGMA_MEM_READ
            if(i == 0)
              assign o_payload_s[i]   =  {128{fetch_sel[i]}} & mem[i];
            else
              assign o_payload_s[i]   = o_payload_s[i-1] | ({128{fetch_sel[i]}} & mem[i]);
        end
    endgenerate

    assign o_payload = o_payload_s[ENIGMA_CELL_MAX-1];
    
endmodule

module ENIGMA_CELL (/*AUTOARG*/
    // Outputs
    o_seek_det, o_seek_id_full, vote_en, vote_id, vote_qos,
    vote_qos_bin, valid,
    // Inputs
    clk, rst_n, i_load_en, i_load_sel, i_load_seq_num, i_load_id,
    i_load_qos, i_seek_en, i_seek_id, i_seek_qos, release_en,
    release_id, lock_en, delect_en, delect_id, cur_hi_qos_bin,
    cur_sel_en
    ) ;
    input          clk;
    input          rst_n;
    
    input          i_load_en;      // single cycle latch the in data and mark the valid flag
    input          i_load_sel;     // valid when i_load_en
    input [3:0]    i_load_seq_num;
    input [5:0]    i_load_id;
    input [1:0]    i_load_qos;

    
    input          i_seek_en;      // for same ID detect
    input [5:0]    i_seek_id;
    input [1:0]    i_seek_qos;
    output         o_seek_det;     // high when detected same ID
    output         o_seek_id_full;
    

    input          release_en;     // single cycle to release current locked ID
    input [5:0]    release_id;


    input          lock_en;
    input          delect_en;
    input [5:0]    delect_id;
    
    output         vote_en;
    output [5:0]   vote_id;
    output [1:0]   vote_qos;
    
    output [3:0]   vote_qos_bin;

    input [3:0]    cur_hi_qos_bin; // current highest Qos, onehot coding
    input          cur_sel_en;     // current selected to output
    output         valid;
    

    reg            vote_en;
    // common group
    reg            valid;
    reg            lock;
    reg [5:0]      id;
    reg [1:0]      qos;

    reg [3:0]      id_seq_num;

    reg            o_seek_det;
    reg            o_seek_id_full;
    wire           hit_i_id = i_seek_id == id;
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        o_seek_det   <= 1'b0;
      else if(i_seek_en & valid)
        o_seek_det   <= hit_i_id;
      else if(o_seek_det)
        o_seek_det   <= 1'b0;

    wire           seek_id_hit_max_depth = i_seek_en & valid & hit_i_id & (id_seq_num == 4'd14);
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        o_seek_id_full   <= 1'b0;
      else if(seek_id_hit_max_depth)
        o_seek_id_full   <= 1'b1;
      else if(o_seek_id_full)
        o_seek_id_full   <= 1'b0;

    wire [3:0]     flesh_seq_num = ( {4{(valid & o_seek_det)}} &  (id_seq_num + 1) |
                                     {4{i_load_sel}} & i_load_seq_num );
    wire           id_seq_not_single = (|id_seq_num);
    wire           reduce_seq_num_en = delect_en & id_seq_not_single;
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        id_seq_num   <= 4'b0;
      else if(i_load_en)
        id_seq_num   <= flesh_seq_num;
      else if(reduce_seq_num_en)
        id_seq_num   <= id_seq_num - 1;
    
    wire           delect_en_hit = delect_en & (~id_seq_not_single) & (delect_id == id) & valid;
    wire           i_load_hit = i_load_en & i_load_sel;
    wire           flesh_valid_en = ( i_load_hit | delect_en_hit);
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        valid      <= 1'b0;
      else if(flesh_valid_en)
        valid      <= i_load_hit;

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        {id, qos}      <= 1'b0;
      else if(i_load_hit)
        {id, qos}      <= {i_load_id, i_load_qos};

    wire           flesh_lock_en = ( (lock_en & (delect_id == id)) |
                                     (release_en & (release_id == id)) ) & valid;
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        lock      <= 1'b0;
      else if(flesh_lock_en)
        lock      <= lock_en;

    wire [3:0]     vote_qos_bin_s = { qos == 2'd3, qos == 2'd2, 
                                      qos == 2'd1, qos == 2'd0 } & {4{valid}};
    assign vote_qos_bin = vote_qos_bin_s & {4{~lock_en}} & {4{~id_seq_not_single}};

    wire [3:0]     cmb_qos_bin = cur_hi_qos_bin & vote_qos_bin_s & {4{~lock_en}};
    wire           vote_en_s = (|cmb_qos_bin) & (id_seq_num == 4'b0);
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        vote_en   <= 1'b0;
      else if(delect_en_hit)
        vote_en   <= 1'b0;
      else if(valid)
        vote_en   <= vote_en_s;

    
    assign vote_id  = id & {6{cur_sel_en}};
    assign vote_qos = qos & {2{cur_sel_en}};

    
endmodule // ENIGMA_CELL

module ENIGMA_BUFFER(/*autoarg*/
    // Outputs
    ready_a, ready_b, valid_c, payload_c, id_c, qos_c,
    // Inputs
    clk, rst_n, payload_a, id_a, qos_a, valid_a, payload_b, id_b,
    qos_b, valid_b, conflict_c, release_c, releaseid_c, ready_c
    );

    parameter  ENIGMA_CELL_MAX = 24;
   
    input                           clk;
    input                           rst_n;
    
    // port A
    input [127:0]                   payload_a;
    input [4:0]                     id_a;
    input [1:0]                     qos_a;
    input                           valid_a;
    output                          ready_a;

    // port B
    input [127:0]                   payload_b;
    input [4:0]                     id_b;
    input [1:0]                     qos_b;
    input                           valid_b;
    output                          ready_b;

    // output port C
    input                           conflict_c;
    input                           release_c;
    input [5:0]                     releaseid_c;

    input                           ready_c;
    output                          valid_c;
    
    output [127:0]                  payload_c;
    output [5:0]                    id_c;
    output [1:0]                    qos_c;

    reg                             port_sel; // 0: select A   1: select B
    reg                             local_buf_not_full;
    
    reg [127:0]                     cur_o_payload;
    reg [5:0]                       cur_o_id;
    reg [1:0]                       cur_o_qos;

    //reg [127:0]                     i_load_payload;
    reg                             i_load_en;
    wire [ENIGMA_CELL_MAX-1:0]      i_load_sel;
    reg [5:0]                       i_load_id;
    reg [1:0]                       i_load_qos;
    
    reg [3:0]                       cur_hi_qos_bin;
    reg                             valid_c;
    reg [5:0]                       id_c;
    reg [1:0]                       qos_c;
    reg [5:0]                       post_id_c;

    reg [5:0]                       post_o_id;
    reg                             vld_c_o_en; // delay one cycle of (valid_c & ready_c)
    wire                            delect_en = vld_c_o_en & ~conflict_c;

    wire                            i_seek_en  = (valid_a & ready_a) | (valid_b & ready_b);
    wire [5:0]                      i_seek_id  = port_sel ? {1'b1,id_b} : {1'b0,id_a};
    wire [1:0]                      i_seek_qos = port_sel ? qos_b : qos_a;
    wire [127:0]                    i_seek_payload = port_sel ? payload_b : payload_a;
    
    
    wire [ENIGMA_CELL_MAX-1:0]      o_seek_det;
    wire [ENIGMA_CELL_MAX-1:0]      o_seek_id_full;
    wire [ENIGMA_CELL_MAX-1:0]      vote_en;
    wire [ENIGMA_CELL_MAX-1:0]      valid;
    wire [5:0]                      vote_id[ENIGMA_CELL_MAX-1:0];
    wire [1:0]                      vote_qos[ENIGMA_CELL_MAX-1:0];
    wire [3:0]                      vote_qos_bin[ENIGMA_CELL_MAX-1:0];

    reg                             mark_full_id_en;
    reg [5:0]                       mark_full_id;

    wire                            vld_c_o_en_s = valid_c & ready_c;
    wire                            o_seek_id_full_det = |o_seek_id_full;
    wire                            cancle_full_id_en = (post_id_c == mark_full_id) & delect_en;


    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        mark_full_id_en   <= 1'b0;
      else if(o_seek_id_full_det)
        mark_full_id_en   <= 1'b1;
      else if(cancle_full_id_en)
        mark_full_id_en   <= 1'b0;
                
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        mark_full_id   <= 1'b0;
      else if(o_seek_id_full_det)
        mark_full_id   <= i_seek_id;

    //
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        local_buf_not_full   <= 1'b0;
      else
        local_buf_not_full   <= ~(&valid);

    wire                            port_sel_s = (valid_a & valid_b)?(~port_sel) : (valid_b & (~valid_a));
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        port_sel   <= 1'b0;
      else
        port_sel   <= port_sel_s;

    assign ready_a = (~port_sel) & local_buf_not_full & ~(mark_full_id_en & (~mark_full_id[5]) & (mark_full_id[4:0] == id_a));
    assign ready_b = ( port_sel) & local_buf_not_full & ~(mark_full_id_en & ( mark_full_id[5]) & (mark_full_id[4:0] == id_b));

    wire [ENIGMA_CELL_MAX-1:0]      cur_sel_en;

    wire [3:0]                      i_load_seq_num;
    wire [3:0]                      i_load_seq_num_s[ENIGMA_CELL_MAX-1:0];
    wire [3:0]                      cur_hi_qos_bin_s[ENIGMA_CELL_MAX-1:0];

    wire [5:0]                      vote_id_s[ENIGMA_CELL_MAX-1:0];
    wire [1:0]                      vote_qos_s[ENIGMA_CELL_MAX-1:0];

    generate
        genvar                      cc;
        for(cc=0; cc<ENIGMA_CELL_MAX; cc=cc+1)begin:ENIGMA_CELL_GEN
            ENIGMA_CELL U_ENIGMA_CELL( 
                                       // Outputs
                                       .o_seek_det           (o_seek_det[cc]),
                                       .o_seek_id_full       (o_seek_id_full[cc]),
                                       .vote_en              (vote_en[cc]),
                                       .vote_id              (vote_id[cc]),
                                       .vote_qos             (vote_qos[cc]),
                                       .vote_qos_bin         (vote_qos_bin[cc]),
                                       .valid                (valid[cc]),
                                       // Inputs
                                       .clk                  (clk),
                                       .rst_n                (rst_n),
                                       .i_load_en            (i_load_en),
                                       .i_load_sel           (i_load_sel[cc]),
                                       .i_load_seq_num       (i_load_seq_num),
                                       .i_load_id            (i_load_id),
                                       .i_load_qos           (i_load_qos),
                                       .i_seek_en            (i_seek_en),
                                       .i_seek_id            (i_seek_id),
                                       .i_seek_qos           (i_seek_qos),
                                       .release_en           (release_c),
                                       .release_id           (releaseid_c),
                                       .lock_en              (conflict_c),
                                       .delect_en            (delect_en),
                                       .delect_id            (post_id_c),
                                       .cur_hi_qos_bin       (cur_hi_qos_bin),
                                       .cur_sel_en           (cur_sel_en[cc])
                                       );
            
        end
    endgenerate

    generate
        genvar                      dd;
        for(dd=0; dd<ENIGMA_CELL_MAX; dd=dd+1)begin:ENIGMA_SIG_GEN
            if(dd == 0)begin
                assign vote_id_s[dd]  = vote_id[dd];
                assign vote_qos_s[dd] = vote_qos[dd];
                
                assign i_load_seq_num_s[dd] = o_seek_det[dd];
                assign cur_hi_qos_bin_s[dd] = vote_qos_bin[dd];
            end else begin
                assign vote_id_s[dd]  = vote_id_s[dd-1]  | vote_id[dd];
                assign vote_qos_s[dd] = vote_qos_s[dd-1] | vote_qos[dd];

                assign i_load_seq_num_s[dd] = i_load_seq_num_s[dd-1] + o_seek_det[dd];
                assign cur_hi_qos_bin_s[dd] = cur_hi_qos_bin_s[dd-1] | vote_qos_bin[dd];
            end
        end
    endgenerate

    assign i_load_seq_num = i_load_seq_num_s[ENIGMA_CELL_MAX-1];
    
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        vld_c_o_en   <= 1'b0;
      else
        vld_c_o_en   <= vld_c_o_en_s;

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        cur_hi_qos_bin      <= 4'b0;
      else
        cur_hi_qos_bin      <= cur_hi_qos_bin_s[ENIGMA_CELL_MAX-1];

    assign cur_sel_en = ( vote_en[0] ? 24'h00_0001 :
                          vote_en[1] ? 24'h00_0002 :
                          vote_en[2] ? 24'h00_0004 :
                          vote_en[3] ? 24'h00_0008 :
                          vote_en[4] ? 24'h00_0010 :
                          vote_en[5] ? 24'h00_0020 :
                          vote_en[6] ? 24'h00_0040 :
                          vote_en[7] ? 24'h00_0080 :
                          vote_en[ 8] ? 24'h00_0100 :
                          vote_en[ 9] ? 24'h00_0200 :
                          vote_en[10] ? 24'h00_0400 :
                          vote_en[11] ? 24'h00_0800 :
                          vote_en[12] ? 24'h00_1000 :
                          vote_en[13] ? 24'h00_2000 :
                          vote_en[14] ? 24'h00_4000 :
                          vote_en[15] ? 24'h00_8000 :
                          vote_en[16] ? 24'h01_0000 :
                          vote_en[17] ? 24'h02_0000 :
                          vote_en[18] ? 24'h04_0000 :
                          vote_en[19] ? 24'h08_0000 :
                          vote_en[20] ? 24'h10_0000 :
                          vote_en[21] ? 24'h20_0000 :
                          vote_en[22] ? 24'h40_0000 :
                          vote_en[23] ? 24'h80_0000 : 24'h00_0000
                          );
    
    assign i_load_sel = ( (~valid[0]) ? 24'h00_0001 :
                          (~valid[1]) ? 24'h00_0002 :
                          (~valid[2]) ? 24'h00_0004 :
                          (~valid[3]) ? 24'h00_0008 :
                          (~valid[4]) ? 24'h00_0010 :
                          (~valid[5]) ? 24'h00_0020 :
                          (~valid[6]) ? 24'h00_0040 :
                          (~valid[7]) ? 24'h00_0080 :
                          (~valid[ 8]) ? 24'h00_0100 :
                          (~valid[ 9]) ? 24'h00_0200 :
                          (~valid[10]) ? 24'h00_0400 :
                          (~valid[11]) ? 24'h00_0800 :
                          (~valid[12]) ? 24'h00_1000 :
                          (~valid[13]) ? 24'h00_2000 :
                          (~valid[14]) ? 24'h00_4000 :
                          (~valid[15]) ? 24'h00_8000 :
                          (~valid[16]) ? 24'h01_0000 :
                          (~valid[17]) ? 24'h02_0000 :
                          (~valid[18]) ? 24'h04_0000 :
                          (~valid[19]) ? 24'h08_0000 :
                          (~valid[20]) ? 24'h10_0000 :
                          (~valid[21]) ? 24'h20_0000 :
                          (~valid[22]) ? 24'h40_0000 :
                          (~valid[23]) ? 24'h80_0000 : 24'h00_0000
                          );
                           
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        {i_load_id, i_load_qos}      <= 1'b0;
      else if(i_seek_en)
        {i_load_id, i_load_qos}      <= {i_seek_id, i_seek_qos};
    
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        valid_c   <= 1'b0;
      else
        valid_c   <= |cur_sel_en;

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        post_id_c   <= 1'b0;
      else if(valid_c & ready_c)
        post_id_c   <= id_c;

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        {id_c, qos_c}      <= 1'b0;
      else if(|cur_sel_en)
        {id_c, qos_c}      <= {vote_id_s[ENIGMA_CELL_MAX-1], vote_qos_s[ENIGMA_CELL_MAX-1]};

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        i_load_en   <= 1'b0;
      else
        i_load_en   <= i_seek_en;


    ENIGMA_MEM #(ENIGMA_CELL_MAX) U_ENIGMA_MEM(// Outputs
                                               .o_payload          (payload_c),
                                               // Inputs
                                               .clk                (clk),
                                               .rst_n              (rst_n),
                                               .load_sel           (i_load_sel & {ENIGMA_CELL_MAX{i_seek_en}} ),
                                               .fetch_sel          (cur_sel_en),
                                               .i_payload          (i_seek_payload)
                                               );

endmodule

