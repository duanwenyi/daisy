module ENIGMA_CELL (/*AUTOARG*/
    // Outputs
    o_seek_sum, vote_en, vote_id, vote_qos, vote_qos_bin, valid,
    port_attr,
    // Inputs
    clk, rst_n, i_load_seq_num, i_seek_en, i_seek_sel, i_seek_id,
    i_seek_qos, release_en, release_id, lock_en, delect_en, delect_id,
    bypass, cur_hi_qos_bin, cur_c_oen, cur_c_o_sel, cur_c_id
    ) ;
    input          clk;
    input          rst_n;
    
    input [3:0]    i_load_seq_num;   // timing not good !
    
    input          i_seek_en;        // for same ID detect
    input          i_seek_sel;       // valid when i_seek_en
    input [5:0]    i_seek_id;
    input [1:0]    i_seek_qos;
    output [3:0]   o_seek_sum;       // high when detected same ID
    

    input          release_en;       // single cycle to release current locked ID
    input [5:0]    release_id;


    input          lock_en;
    input          delect_en;
    input [5:0]    delect_id;
    input          bypass;
    
    output         vote_en;
    output [5:0]   vote_id;
    output [1:0]   vote_qos;
    
    output [3:0]   vote_qos_bin;

    input [3:0]    cur_hi_qos_bin;   // current highest Qos, onehot coding
    input          cur_c_oen;
    input          cur_c_o_sel;
    input [5:0]    cur_c_id;
    output         valid;
    output         port_attr;        // 0: mark it A port ,   1: mark it B port
    

    reg            vote_en;
    // common group
    reg            valid;
    reg            lock;
    reg [5:0]      id;
    reg [1:0]      qos;

    reg [3:0]      id_seq_num;
    reg            id_seq_tail;
    reg [3:0]      vote_qos_bin;

    wire           delect_en_hit_the_id = delect_en & (delect_id == id) & valid;
    wire           id_seq_not_single = (|id_seq_num);
    wire           reduce_seq_num_en = delect_en_hit_the_id & id_seq_not_single;

    wire           i_seek_en_hit  = i_seek_en & i_seek_sel;
    wire           delect_en_hit  = delect_en_hit_the_id & (~id_seq_not_single);
    wire           flesh_valid_en = ( i_seek_en_hit | delect_en_hit);

    wire           cur_c_o_en = cur_c_oen & cur_c_o_sel;
    wire           cur_c_o_hit_id = cur_c_oen & (cur_c_id == id) & valid;

    assign         port_attr = id[5];

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        id_seq_num   <= 4'b0;
      else if(i_seek_en_hit)
        id_seq_num   <= i_load_seq_num;
      else if(delect_en_hit_the_id & id_seq_not_single)
        id_seq_num   <= id_seq_num - 1;

    wire           mark_id_seq_tail   = i_seek_en_hit;
    wire           ummark_id_seq_tail = i_seek_en & (id == i_seek_id) & valid;
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        id_seq_tail   <= 1'b0;
      else if(mark_id_seq_tail)
        id_seq_tail   <= 1'b1;
      else if(ummark_id_seq_tail)
        id_seq_tail   <= 1'b0;

    assign o_seek_sum = {4{id_seq_tail & valid & (id == i_seek_id)}} & (delect_en_hit_the_id ? id_seq_num : (id_seq_num + 1));

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        valid      <= 1'b0;
      else if(flesh_valid_en)
        valid      <= i_seek_en_hit;

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        {id, qos}      <= 1'b0;
      else if(i_seek_en_hit)
        {id, qos}      <= {i_seek_id, i_seek_qos};

    wire           lock_en_hit    = lock_en & (delect_id == id) & valid;
    wire           release_en_hit = release_en & (release_id == id) & valid;
    
    wire           flesh_lock_en  = ( (lock_en & (delect_id == id)) |
                                      (release_en & (release_id == id)) ) & valid;
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        lock      <= 1'b0;
      else if(flesh_lock_en)
        lock      <= lock_en;

    //wire [3:0]     i_seek_qos_bin = { i_seek_qos == 2'd3, i_seek_qos == 2'd2, 
    //                                  i_seek_qos == 2'd1, i_seek_qos == 2'd0 };

    wire [3:0]     vote_qos_bin_s = { qos == 2'd3, qos == 2'd2, 
                                      qos == 2'd1, qos == 2'd0 };

    //wire           init_load_qos_bin = i_seek_en_hit & ( (i_load_seq_num == 4'b0) | (delect_en_hit_the_id & (i_load_seq_num == 4'b1)));   // the only ID
    wire           invoke_qos_bin    = ( (cur_c_o_hit_id & (id_seq_num == 4'd1)) |
                                         (release_en_hit| valid & ~delect_en_hit_the_id) & ~id_seq_not_single 
                                         );
    wire           dim_local_qos_bin = cur_c_o_en | lock_en_hit | delect_en_hit;
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        vote_qos_bin    <= 4'b0;
      //else if(init_load_qos_bin)
      //  vote_qos_bin    <= i_seek_qos_bin;
      else if( dim_local_qos_bin )
        vote_qos_bin    <= 4'b0;
      else if(invoke_qos_bin)
        vote_qos_bin    <= vote_qos_bin_s;

    wire           vote_en_mix = ( ((|(vote_qos_bin & cur_hi_qos_bin)) & ~lock_en_hit) & 
                                   ~ (cur_c_o_en | delect_en_hit_the_id) &
                                   valid & ~lock);
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        vote_en   <= 1'b0;
      else
        vote_en   <= vote_en_mix;

    assign vote_id  = id;
    assign vote_qos = qos;

    
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
    output reg                      ready_a;

    // port B
    input [127:0]                   payload_b;
    input [4:0]                     id_b;
    input [1:0]                     qos_b;
    input                           valid_b;
    output reg                      ready_b;

    // output port C
    input                           conflict_c;
    input                           release_c;
    input [5:0]                     releaseid_c;

    input                           ready_c;
    output reg                      valid_c;
    
    output reg [127:0]              payload_c;
    output reg [5:0]                id_c;
    output reg [1:0]                qos_c;

    reg                             port_sel;        // 0: select A   1: select B
    reg [5:0]                       local_flit_nums;

    reg [5:0]                       hungry_cnt;      // count hungry FLIT
    reg                             hungry_det;      // Hungry schedule detector

    
    wire [3:0]                      cur_hi_qos_bin;
    reg [5:0]                       post_id_c;
    reg                             load_c_vote_en;

    wire [127:0]                    cur_sel_payload;

    reg                             vld_c_o_en_ff; // delay one cycle of (valid_c & ready_c)
    wire                            vld_c_o_en = valid_c & ready_c;
    wire                            delect_en  = vld_c_o_en_ff & ~conflict_c;

    wire                            vld_i_a    = (valid_a & ready_a);
    wire                            vld_i_b    = (valid_b & ready_b);
    wire                            i_seek_en  = vld_i_a | vld_i_b;
    wire [ENIGMA_CELL_MAX-1:0]      i_seek_sel;
    wire                            i_seek_sel_full;
    wire [5:0]                      i_seek_id[ENIGMA_CELL_MAX-1:0];
    wire [1:0]                      i_seek_qos[ENIGMA_CELL_MAX-1:0];
    wire [127:0]                    i_seek_payload[ENIGMA_CELL_MAX-1:0];
    //wire [5:0]                      i_seek_id  = ready_b ? {1'b1,id_b} : {1'b0,id_a};
    //wire [1:0]                      i_seek_qos = ready_b ? qos_b : qos_a;
    //wire [127:0]                    i_seek_payload = ready_b ? payload_b : payload_a;

    wire [ENIGMA_CELL_MAX-1:0]      vote_en;
    wire [ENIGMA_CELL_MAX-1:0]      valid;
    wire [5:0]                      vote_id[ENIGMA_CELL_MAX-1:0];
    wire [1:0]                      vote_qos[ENIGMA_CELL_MAX-1:0];
    wire [3:0]                      vote_qos_bin[ENIGMA_CELL_MAX-1:0];

    //
    wire                            local_flit_empty    = local_flit_nums == 5'b0;
    wire                            local_flit_single   = local_flit_nums == 5'b1;
    wire [5:0]                      local_flit_nums_new = local_flit_nums + (valid_a & ready_a) + (valid_b & ready_b) - vld_c_o_en  + conflict_c;
    wire                            local_buf_not_full  = ~((local_flit_nums_new == ENIGMA_CELL_MAX) |
                                                            ((local_flit_nums_new == (ENIGMA_CELL_MAX -1)) & ~delect_en & i_seek_en) |
                                                            ((local_flit_nums_new == (ENIGMA_CELL_MAX -2)) & ~delect_en & (vld_i_a & vld_i_b) ) |
                                                            (&valid) 
                                                            //~((local_flit_nums > (ENIGMA_CELL_MAX - 4) ) & valid_a & valid_b ) 
                                                            );

    //wire                            port_sel_invert = ~(valid_a ^ valid_b);
    wire                            port_sel_invert = (valid_a & ~ready_a) | (valid_b & ~ready_b);
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        port_sel   <= 1'b0;
      else if(i_seek_sel_full & port_sel_invert)
        port_sel   <= ~port_sel;

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        ready_a   <= 1'b1;
      else
        //ready_a   <= (~port_sel) & local_buf_not_full & ~hungry_det;
        ready_a   <= (~i_seek_sel_full | ~port_sel) & local_buf_not_full & ~hungry_det;
    

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        ready_b   <= 1'b0;
      else
        //ready_b   <= port_sel & local_buf_not_full & ~hungry_det;
        ready_b   <= (~i_seek_sel_full |  port_sel) & local_buf_not_full & ~hungry_det;

    wire [ENIGMA_CELL_MAX-1:0]      port_attr;
    reg [ENIGMA_CELL_MAX-1:0]       pre_sel_en;
    reg                             co_port_sel;   // 0: select A vote to out   1: select B vote to out

    wire [3:0]                      cur_hi_qos_bin_s[ENIGMA_CELL_MAX-1:0];

    wire [3:0]                      o_seek_sum_det_a;
    wire [3:0]                      o_seek_sum_det_b;
    wire [3:0]                      o_seek_sum[ENIGMA_CELL_MAX-1:0];
    wire [3:0]                      o_seek_sum_a[ENIGMA_CELL_MAX-1:0];
    wire [3:0]                      o_seek_sum_b[ENIGMA_CELL_MAX-1:0];
    wire [3:0]                      i_load_seq_num[ENIGMA_CELL_MAX-1:0];

    wire [5:0]                      vote_id_s[ENIGMA_CELL_MAX-1:0];
    wire [1:0]                      vote_qos_s[ENIGMA_CELL_MAX-1:0];

    wire [ENIGMA_CELL_MAX-1:0]      cur_sel_oa;
    wire [ENIGMA_CELL_MAX-1:0]      cur_sel_ob;
    wire [5:0]                      vote_id_a_s[ENIGMA_CELL_MAX-1:0];
    wire [1:0]                      vote_qos_a_s[ENIGMA_CELL_MAX-1:0];

    wire [5:0]                      vote_id_b_s[ENIGMA_CELL_MAX-1:0];
    wire [1:0]                      vote_qos_b_s[ENIGMA_CELL_MAX-1:0];

    // debug using
    wire [5:0]                      vote_id_ss  = vote_id_s[ENIGMA_CELL_MAX-1];
    wire [1:0]                      vote_qos_ss = vote_qos_s[ENIGMA_CELL_MAX-1];

    wire [ENIGMA_CELL_MAX-1:0]      i_seek_sel_a = ( (~valid[0]) ? 24'h00_0001 :
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

    wire [ENIGMA_CELL_MAX-1:0]      i_seek_sel_b = ( (~valid[23]) ? 24'h80_0000 : 
                                                     (~valid[22]) ? 24'h40_0000 :             
                                                     (~valid[21]) ? 24'h20_0000 :             
                                                     (~valid[20]) ? 24'h10_0000 :             
                                                     (~valid[19]) ? 24'h08_0000 :             
                                                     (~valid[18]) ? 24'h04_0000 :             
                                                     (~valid[17]) ? 24'h02_0000 :             
                                                     (~valid[16]) ? 24'h01_0000 :             
                                                     (~valid[15]) ? 24'h00_8000 :             
                                                     (~valid[14]) ? 24'h00_4000 :             
                                                     (~valid[13]) ? 24'h00_2000 :             
                                                     (~valid[12]) ? 24'h00_1000 :             
                                                     (~valid[11]) ? 24'h00_0800 :             
                                                     (~valid[10]) ? 24'h00_0400 :             
                                                     (~valid[ 9]) ? 24'h00_0200 :             
                                                     (~valid[ 8]) ? 24'h00_0100 :             
                                                     (~valid[7]) ? 24'h00_0080 :              
                                                     (~valid[6]) ? 24'h00_0040 :              
                                                     (~valid[5]) ? 24'h00_0020 :              
                                                     (~valid[4]) ? 24'h00_0010 :              
                                                     (~valid[3]) ? 24'h00_0008 :              
                                                     (~valid[2]) ? 24'h00_0004 :              
                                                     (~valid[1]) ? 24'h00_0002 :  
                                                     (~valid[0]) ? 24'h00_0001 : 24'h00_0000 
                                                     );

    assign i_seek_sel_full = i_seek_sel_a == i_seek_sel_b;

    assign i_seek_sel = (i_seek_sel_a & {ENIGMA_CELL_MAX{vld_i_a}}) | (i_seek_sel_b & {ENIGMA_CELL_MAX{vld_i_b}});
    
    generate
        genvar                      cc;
        for(cc=0; cc<ENIGMA_CELL_MAX; cc=cc+1)begin:ENIGMA_CELL_GEN
            ENIGMA_CELL U_ENIGMA_CELL( 
                                       // Outputs
                                       .o_seek_sum           (o_seek_sum[cc]),
                                       .vote_en              (vote_en[cc]),
                                       .vote_id              (vote_id[cc]),
                                       .vote_qos             (vote_qos[cc]),
                                       .vote_qos_bin         (vote_qos_bin[cc]),
                                       .valid                (valid[cc]),
                                       .port_attr            (port_attr[cc]),
                                       // Inputs
                                       .clk                  (clk),
                                       .rst_n                (rst_n),
                                       .i_load_seq_num       (i_load_seq_num[cc]),
                                       .i_seek_en            (i_seek_en),
                                       .i_seek_sel           (i_seek_sel[cc]),
                                       .i_seek_id            (i_seek_id[cc]),
                                       .i_seek_qos           (i_seek_qos[cc]),
                                       .release_en           (release_c),
                                       .release_id           (releaseid_c),
                                       .lock_en              (conflict_c),
                                       .delect_en            (delect_en),
                                       .delect_id            (post_id_c),
                                       .bypass               (~load_c_vote_en),
                                       .cur_hi_qos_bin       (cur_hi_qos_bin),
                                       .cur_c_oen            (vld_c_o_en),
                                       .cur_c_o_sel          (pre_sel_en[cc]),
                                       .cur_c_id             (id_c)
                                       );
            
        end
    endgenerate

    generate
        genvar                      dd;
        for(dd=0; dd<ENIGMA_CELL_MAX; dd=dd+1)begin:ENIGMA_SIG_GEN
            // bug here !
            assign i_seek_qos[dd] = ((port_attr[dd]&valid[dd]) | (i_seek_sel_b[dd]&vld_i_b) ) ? qos_b : qos_a;
            assign i_seek_id[dd]  = ((port_attr[dd]&valid[dd]) | (i_seek_sel_b[dd]&vld_i_b) ) ? { vld_i_b,id_b}  : {~vld_i_a,id_a};
            assign i_seek_payload[dd] = (i_seek_sel_a[dd]&vld_i_a) ? payload_a : payload_b;

            assign i_load_seq_num[dd] = (i_seek_sel_b[dd]&ready_b) ? o_seek_sum_det_b : o_seek_sum_det_a;
            
            if(dd == 0)begin
                assign vote_id_a_s[dd]  = vote_id[dd]  & {6{cur_sel_oa[dd]}};
                assign vote_qos_a_s[dd] = vote_qos[dd] & {2{cur_sel_oa[dd]}};

                assign vote_id_b_s[dd]  = vote_id[dd]  & {6{cur_sel_ob[dd]}};
                assign vote_qos_b_s[dd] = vote_qos[dd] & {2{cur_sel_ob[dd]}};

                assign o_seek_sum_a[dd]     = {4{~port_attr[dd]}} & o_seek_sum[dd];
                assign o_seek_sum_b[dd]     = {4{ port_attr[dd]}} & o_seek_sum[dd];

                assign cur_hi_qos_bin_s[dd] = vote_qos_bin[dd];
            end else begin
                assign vote_id_a_s[dd]  = vote_id_a_s[dd-1]  | (vote_id[dd]  & {6{cur_sel_oa[dd]}});
                assign vote_qos_a_s[dd] = vote_qos_a_s[dd-1] | (vote_qos[dd] & {2{cur_sel_oa[dd]}});

                assign vote_id_b_s[dd]  = vote_id_b_s[dd-1]  | (vote_id[dd]  & {6{cur_sel_ob[dd]}});
                assign vote_qos_b_s[dd] = vote_qos_b_s[dd-1] | (vote_qos[dd] & {2{cur_sel_ob[dd]}});

                assign o_seek_sum_a[dd]     = o_seek_sum_a[dd-1] | {4{~port_attr[dd]}} & o_seek_sum[dd];
                assign o_seek_sum_b[dd]     = o_seek_sum_b[dd-1] | {4{ port_attr[dd]}} & o_seek_sum[dd];
                
                assign cur_hi_qos_bin_s[dd] = cur_hi_qos_bin_s[dd-1] | vote_qos_bin[dd];
            end
        end
    endgenerate

    assign o_seek_sum_det_a = o_seek_sum_a[ENIGMA_CELL_MAX-1];
    assign o_seek_sum_det_b = o_seek_sum_b[ENIGMA_CELL_MAX-1];
    //assign i_load_seq_num = o_seek_sum_det;

    wire [5:0]                      vote_id_a  = vote_id_a_s[ENIGMA_CELL_MAX-1];
    wire [1:0]                      vote_qos_a = vote_qos_a_s[ENIGMA_CELL_MAX-1];
    wire [5:0]                      vote_id_b  = vote_id_b_s[ENIGMA_CELL_MAX-1];
    wire [1:0]                      vote_qos_b = vote_qos_b_s[ENIGMA_CELL_MAX-1];
    wire [127:0]                    o_payload_oa;
    wire [127:0]                    o_payload_ob;
    
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        vld_c_o_en_ff   <= 1'b0;
      else
        vld_c_o_en_ff   <= vld_c_o_en;

    wire                            cur_have_oa_vote = |cur_sel_oa;
    wire                            cur_have_ob_vote = |cur_sel_ob;
    // here have some bug
    wire                            cur_have_vote    = cur_have_oa_vote & ~co_port_sel | cur_have_ob_vote & co_port_sel;

    wire                            co_port_sel_invert = cur_have_oa_vote & co_port_sel | cur_have_ob_vote & ~co_port_sel;
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        co_port_sel     <= 1'b0;
      else if(co_port_sel_invert)
        co_port_sel     <= ~co_port_sel;

    wire [5:0]                      vote_id_c      = co_port_sel ? vote_id_b : vote_id_a;
    wire [1:0]                      vote_qos_c     = co_port_sel ? vote_qos_b : vote_qos_a;
    wire [127:0]                    vote_payload_c = co_port_sel ? o_payload_ob : o_payload_oa;
    

    wire [3:0]                      cur_hi_qos_bin_ss = cur_hi_qos_bin_s[ENIGMA_CELL_MAX-1];
    assign                          cur_hi_qos_bin    = ( cur_hi_qos_bin_ss[3] ? 4'b1000 :
                                                          cur_hi_qos_bin_ss[2] ? 4'b0100 :
                                                          cur_hi_qos_bin_ss[1] ? 4'b0010 :
                                                          cur_hi_qos_bin_ss[0] ? 4'b0001 : 4'b0000
                                                          );
    
    assign cur_sel_oa = ( (vote_en[0]  &~port_attr[0] ) ? 24'h00_0001 :
                          (vote_en[1]  &~port_attr[1] ) ? 24'h00_0002 :
                          (vote_en[2]  &~port_attr[2] ) ? 24'h00_0004 :
                          (vote_en[3]  &~port_attr[3] ) ? 24'h00_0008 :
                          (vote_en[4]  &~port_attr[4] ) ? 24'h00_0010 :
                          (vote_en[5]  &~port_attr[5] ) ? 24'h00_0020 :
                          (vote_en[6]  &~port_attr[6] ) ? 24'h00_0040 :
                          (vote_en[7]  &~port_attr[7] ) ? 24'h00_0080 :
                          (vote_en[8]  &~port_attr[8] ) ? 24'h00_0100 :
                          (vote_en[9]  &~port_attr[9] ) ? 24'h00_0200 :
                          (vote_en[10] &~port_attr[10]) ? 24'h00_0400 :
                          (vote_en[11] &~port_attr[11]) ? 24'h00_0800 :
                          (vote_en[12] &~port_attr[12]) ? 24'h00_1000 :
                          (vote_en[13] &~port_attr[13]) ? 24'h00_2000 :
                          (vote_en[14] &~port_attr[14]) ? 24'h00_4000 :
                          (vote_en[15] &~port_attr[15]) ? 24'h00_8000 :
                          (vote_en[16] &~port_attr[16]) ? 24'h01_0000 :
                          (vote_en[17] &~port_attr[17]) ? 24'h02_0000 :
                          (vote_en[18] &~port_attr[18]) ? 24'h04_0000 :
                          (vote_en[19] &~port_attr[19]) ? 24'h08_0000 :
                          (vote_en[20] &~port_attr[20]) ? 24'h10_0000 :
                          (vote_en[21] &~port_attr[21]) ? 24'h20_0000 :
                          (vote_en[22] &~port_attr[22]) ? 24'h40_0000 :
                          (vote_en[23] &~port_attr[23]) ? 24'h80_0000 : 24'h00_0000
                          );

    assign cur_sel_ob = ( (vote_en[23] & port_attr[23]) ? 24'h80_0000:
                          (vote_en[22] & port_attr[22]) ? 24'h40_0000:
                          (vote_en[21] & port_attr[21]) ? 24'h20_0000:
                          (vote_en[20] & port_attr[20]) ? 24'h10_0000:
                          (vote_en[19] & port_attr[19]) ? 24'h08_0000:
                          (vote_en[18] & port_attr[18]) ? 24'h04_0000:
                          (vote_en[17] & port_attr[17]) ? 24'h02_0000:
                          (vote_en[16] & port_attr[16]) ? 24'h01_0000:
                          (vote_en[15] & port_attr[15]) ? 24'h00_8000:
                          (vote_en[14] & port_attr[14]) ? 24'h00_4000:
                          (vote_en[13] & port_attr[13]) ? 24'h00_2000 :
                          (vote_en[12] & port_attr[12]) ? 24'h00_1000 :
                          (vote_en[11] & port_attr[11]) ? 24'h00_0800 :
                          (vote_en[10] & port_attr[10]) ? 24'h00_0400 :
                          (vote_en[9]  & port_attr[9] ) ? 24'h00_0200  :
                          (vote_en[8]  & port_attr[8] ) ? 24'h00_0100  :
                          (vote_en[7]  & port_attr[7] ) ? 24'h00_0080  :
                          (vote_en[6]  & port_attr[6] ) ? 24'h00_0040  :
                          (vote_en[5]  & port_attr[5] ) ? 24'h00_0020  :
                          (vote_en[4]  & port_attr[4] ) ? 24'h00_0010  :
                          (vote_en[3]  & port_attr[3] ) ? 24'h00_0008  :
                          (vote_en[2]  & port_attr[2] ) ? 24'h00_0004  :
                          (vote_en[1]  & port_attr[1] ) ? 24'h00_0002  :
                          (vote_en[0]  & port_attr[0] ) ? 24'h00_0001  : 24'h00_0000
                          );

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        pre_sel_en   <= {ENIGMA_CELL_MAX{1'b0}};
      else
        pre_sel_en   <= co_port_sel ? cur_sel_ob : cur_sel_oa;

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        local_flit_nums   <= 5'b0;
      else if( i_seek_en | vld_c_o_en | conflict_c)
        local_flit_nums   <= local_flit_nums_new;

    wire                            vld_o_sel_en   = cur_have_vote & ~local_flit_empty;
    wire                            new_flit_sch   = vld_c_o_en & (cur_have_oa_vote^cur_have_ob_vote) & load_c_vote_en;
    wire                            mark_load_c_vote = ~local_flit_empty & ~load_c_vote_en;
    
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        load_c_vote_en   <= 1'b0;
      else if(mark_load_c_vote)
        load_c_vote_en   <= 1'b1;
      else if(local_flit_single & vld_c_o_en)
        load_c_vote_en   <= 1'b0;

    // hungry detect
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        hungry_cnt   <= 6'b0;
      else if(~hungry_det & load_c_vote_en & vld_o_sel_en )
        hungry_cnt   <= {6{(cur_hi_qos_bin != cur_hi_qos_bin_ss)}} & (hungry_cnt + 1);

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        hungry_det   <= 1'b0;
      else if(local_flit_single & vld_c_o_en)
        hungry_det   <= 1'b0;
      else if( (&hungry_cnt))
        hungry_det   <= 1'b1;

    //wire                            first_same_seq_id = local_flit_single & i_seek_id == id_c;
    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        valid_c   <= 1'b0;
      else if(load_c_vote_en)
        valid_c   <= (|vote_en) & ~new_flit_sch;
      //else if(i_seek_en )
      //  valid_c   <= ~first_same_seq_id;
      else if(ready_c )
        valid_c   <= 1'b0;

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        post_id_c   <= 1'b0;
      else if(vld_c_o_en)
        post_id_c   <= id_c;

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        {id_c, qos_c}      <= 1'b0;
      else if(load_c_vote_en )//& ready_c
        //{id_c, qos_c}      <= {vote_id_s[ENIGMA_CELL_MAX-1], vote_qos_s[ENIGMA_CELL_MAX-1]};
        {id_c, qos_c}      <= {vote_id_c,vote_qos_c};
      //else if(i_seek_en)
      //  {id_c, qos_c}      <= {i_seek_id, i_seek_qos};

    always @(posedge clk or negedge rst_n)
      if(~rst_n)
        payload_c   <= 1'b0;
      else if(load_c_vote_en)
        //payload_c   <= cur_sel_payload;
        payload_c   <= vote_payload_c;
      //else  if(i_seek_en)
      //  payload_c   <= i_seek_payload;

  
    // ---------------------- MEMORY ----------------------------------------
    reg [127:0]                  mem[ENIGMA_CELL_MAX-1:0];

    generate
        //genvar                   cc;
        for(cc=0; cc<ENIGMA_CELL_MAX; cc=cc+1)begin:ENIGMA_MEM_WRITE
            always @(posedge clk)
              if(i_seek_sel[cc] & i_seek_en)
                mem[cc]    <= i_seek_payload[cc];
        end
    endgenerate
    
    wire [127:0]               o_payload_oa_s[ENIGMA_CELL_MAX-1:0];
    wire [127:0]               o_payload_ob_s[ENIGMA_CELL_MAX-1:0];
    generate
        genvar                   i;
        for(i=0; i<ENIGMA_CELL_MAX; i=i+1)begin:ENIGMA_MEM_READ
            if(i == 0)begin
                assign o_payload_oa_s[i]   =  {128{cur_sel_oa[i]}} & mem[i];
                assign o_payload_ob_s[i]   =  {128{cur_sel_ob[i]}} & mem[i];
            end else begin
                assign o_payload_oa_s[i]   = o_payload_oa_s[i-1] | ({128{cur_sel_oa[i]}} & mem[i]);
                assign o_payload_ob_s[i]   = o_payload_ob_s[i-1] | ({128{cur_sel_ob[i]}} & mem[i]);
            end
        end
    endgenerate

    assign o_payload_oa = o_payload_oa_s[ENIGMA_CELL_MAX-1];
    assign o_payload_ob = o_payload_ob_s[ENIGMA_CELL_MAX-1];
    // ---------------------- MEMORY ----------------------------------------

endmodule

