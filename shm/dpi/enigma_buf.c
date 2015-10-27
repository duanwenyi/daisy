#include <cstdio>
#include <cstdlib>
#include <iostream> 
#include <sstream> 
#include <fstream>
#include "enigma.h"

#define ENIGMA_DEBUG

EnigmaBuf::EnigmaBuf(){
	chain.clear();
    chain.resize(ENIGMA_MEM_MAX_SIZE);
    
    mem.full       = 0;
    mem.nptr       = 0;
    mem.count      = 0;
    mem.lock_count = 0;
    mem.vld_bits   = 0;

    qos_count[0]   = 0;
    qos_count[1]   = 0;
    qos_count[2]   = 0;
    qos_count[3]   = 0;

    dim_qos_en     = 0;
    seq_qos_count  = 0;
    last_qos       = 0;
    pre_last_qos   = 0;

    lptr  = 0;
    hptr  = 0;
    head_ptr = 0;
    tail_ptr = 0;
    update_tail_nptr_en = 0;

    pre_chain_id = 0;
    cur_chain_id = 0;

	fprintf(stderr," Enigma Buffer initialed !\n");
	
}



void EnigmaBuf::showFlitInfo(int chain_id){
    if(chain_id < ENIGMA_MEM_MAX_SIZE){
        fprintf(stderr," @%x\t+ChainID[%2d]->[%2d]  : [%s]ID[%2x] QOS[%d] +Mem[%6x][%2d->%2d] +lastQos[%d->%d:%2d][%2d:%2d:%2d:%2d]+->%d items (lock %d) Payload:[%8x %8x %8x %8x]\n", 
                signal.tick,
                chain_id,
                chain.at(chain_id).next_ptr,
                (chain.at(chain_id).id & ENIGMA_ID_EXP ) ? "B" : "A",
                chain.at(chain_id).id,
                chain.at(chain_id).qos,
                mem.vld_bits,
                mem.nptr,
                mem.nnptr,
                pre_last_qos,
                last_qos,
                seq_qos_count,
                qos_count[0],qos_count[1],qos_count[2],qos_count[3],
                mem.count,
                mem.lock_count,
                mem.payload[chain_id][0],
                mem.payload[chain_id][1],
                mem.payload[chain_id][2],
                mem.payload[chain_id][3]
                );
    }

}

void EnigmaBuf::showFlitAll(){
	fprintf(stderr,"----------------------------------------------------------------------------------->>>\n");
    int chain_id = head_ptr;
    int cc = 0;
    while( cc < mem.count){
        showFlitInfo(chain_id);
        chain_id = chain.at(chain_id).next_ptr;
        cc++;
    }
    
}
void EnigmaBuf::lockAllId(int id){
    vector<ENIGMA_FLIT_U>::iterator iter;

    for(iter = chain.begin(); iter != chain.end(); iter++){  
        if(iter->id == id){
            if(iter->lock == 1){
                fprintf(stderr," +Error: @lockAllId : re-lock of ID %2x is detected ! @%x\n", id, signal.tick);
            }
            iter->lock = 1;
            mem.lock_count++;

            if(qos_count[iter->qos] > 0)  // forbiden error
                qos_count[iter->qos]--;
            else
                fprintf(stderr," +Error: @lockAllId : Qos %d is 0 when reduce counter! @%x\n", iter->qos, signal.tick);
        }  
    }
}

void EnigmaBuf::unlockAllId(int id){
    vector<ENIGMA_FLIT_U>::iterator iter;

    for(iter = chain.begin(); iter != chain.end(); iter++){  
        if(iter->id == id){
            if(iter->lock == 0){
                fprintf(stderr," +Error: @unlockAllId : re-unlock of ID %2x is detected ! @%x\n", id, signal.tick);
            }
            iter->lock = 0;
            mem.lock_count--;

            qos_count[iter->qos]++;
        }  
    }
}

int  EnigmaBuf::getlowestQos(){
    int qos = ( (qos_count[0] != 0) ? 0:
                (qos_count[1] != 0) ? 1:
                (qos_count[2] != 0) ? 2: 
                (qos_count[3] != 0) ? 3:  0 );

    return qos;

}

bool EnigmaBuf::isHighQosNotEmpty(int qos){
    int qos_bits = ( (qos_count[3] !=0 ) |
                     ((qos_count[2] !=0 ) << 1) |
                     ((qos_count[1] !=0 ) << 2) |
                     ((qos_count[0] !=0 ) << 3) 
                     );
    
    return ( (qos != 3) &&
             ( ( (qos == 2) && (qos_bits & 0x1 )) ||
               ( (qos == 1) && (qos_bits & 0x3 )) ||
               ( (qos == 0) && (qos_bits & 0x7 )) 
               )
             
             );
}

bool EnigmaBuf::isLowQosNotEmpty(int qos){
    int qos_bits = ( (qos_count[0] !=0 ) |
                     ((qos_count[1] !=0 ) << 1) |
                     ((qos_count[2] !=0 ) << 2) |
                     ((qos_count[3] !=0 ) << 3) 
                     );
    
    return ( (qos != 0) &&
             ( ( (qos == 1) && (qos_bits & 0x1 )) ||
               ( (qos == 2) && (qos_bits & 0x3 )) ||
               ( (qos == 3) && (qos_bits & 0x7 )) 
               )
             
             );
}

int EnigmaBuf::getHighestQos(){
    int qos = ( (qos_count[3] != 0) ? 3:
                (qos_count[2] != 0) ? 2:
                (qos_count[1] != 0) ? 1: 0 );

    return qos;
        
}

void EnigmaBuf::updateMemStatus(){
    mem.nptr  = 0;
    mem.nnptr = 0;
    int cc;
    int post_vld_bits = mem.vld_bits;
    for(cc=0; cc < ENIGMA_MEM_MAX_SIZE; cc++){
        if( !(mem.vld_bits & (1<<cc)) ){
            mem.nptr = cc;
            post_vld_bits = post_vld_bits | (1<<cc);
            break;
        }
    }

    for(cc=0; cc < ENIGMA_MEM_MAX_SIZE; cc++){
        if( !(post_vld_bits & (1<<cc)) ){
            mem.nnptr = cc;
            break;
        }
    }


}

bool EnigmaBuf::isIDUnique(int chain_id){
    bool single = true;
    int id = chain.at(chain_id).id;
    for(int cc=0; cc < ENIGMA_MEM_MAX_SIZE; cc++){
        if( (mem.vld_bits & (1<<cc)) && (chain_id != cc) && (chain.at(cc).id == id) ){
            single = false;
            break;
        }
    }
    
    return single;
}

void EnigmaBuf::consumOneCell(int id){
    if(mem.vld_bits  & (1 << id)){


        mem.vld_bits  &= ~(1 << id);  // unmask the bit
        mem.count--;

#ifdef ENIGMA_DEBUG
        fprintf(stderr, " +C FLIT: " );
        showFlitInfo( id );
#endif
        if(qos_count[chain.at(id).qos] > 0)  // forbiden error
            qos_count[chain.at(id).qos]--;
        else
            fprintf(stderr," +Error: @consumOneCell : Qos %d is 0 when reduce counter! @%x\n", chain.at(id).qos, signal.tick);

        if( (mem.count != 0) )
            chain.at(pre_chain_id).next_ptr = chain.at(id).next_ptr;

        if(id == tail_ptr)
            tail_ptr = pre_chain_id;

        if(id == head_ptr)
            head_ptr = chain.at(id).next_ptr;

        //chain.at(id).lock = 0;

    }else{
        fprintf(stderr," +Error: @consumOneCell : ID %d not valid when VLD_BITS %x ! @%x\n", id, mem.vld_bits, signal.tick);
    }
}

void *enigma_buf_init(){

    EnigmaBuf *app = new EnigmaBuf();
    return (void *)app;

}

void enigma_buf_port_o( void        * app,
                        svBit       * ready_a, 	  
                        svBit       * ready_b,	  

                        svBitVecVal * payload_c_0,
                        svBitVecVal * payload_c_1,
                        svBitVecVal * payload_c_2,
                        svBitVecVal * payload_c_3,
                        svBitVecVal * id_c,
                        svBitVecVal * qos_c,
                        svBit       * valid_c,
                        
                        // debug using
                        svBitVecVal * chain_id,
                        svBitVecVal * chain_size,
                        svBit       * pre_out_vld,
                        svBitVecVal * max_qos,
                        svBit       * dim_qos_en,
                        svBit       * portSel,
                        svBitVecVal * nptr
                        ){
    EnigmaBuf *sim = (EnigmaBuf *)app;

    sim->cur_out_vld_mark = sim->signal.pre_valid_c && sim->signal.ready_c;

    // port C
    if( sim->pre_out_vld_mark ){
        if( sim->signal.conflict_c ){
            sim->lockAllId( sim->last_chain_id );
        }else{

            // gen dim qos
            if( !sim->mem.empty &&
                !sim->dim_qos_en &&
                (sim->pre_last_qos == sim->last_qos) && 
                sim->isHighQosNotEmpty( sim->last_qos )
                ){
                sim->seq_qos_count++;
                    
                if( sim->seq_qos_count == DIM_QOS_SEQ_THRESH){
                    sim->dim_qos_en    = 1;  // !! mark here
                    sim->seq_qos_count = 0;
                }
            }else{
                sim->seq_qos_count = 0;
            }


            sim->last_qos = sim->signal.pre_qos_c;
            sim->pre_last_qos = sim->last_qos;

            sim->showFlitAll();
            sim->consumOneCell( sim->last_chain_id );
            sim->showFlitAll();
        }
    }

    if(sim->dim_qos_en && sim->mem.empty){
        sim->dim_qos_en = 0;
    }
        
    if( sim->signal.release_c ){
        sim->unlockAllId( sim->signal.releaseid_c );
    }


    // *************** INNER FUNCTIONS *******************
    // invoke one valid FLIT to sending !
    if((!sim->mem.empty) &&
       (sim->mem.lock_count < sim->mem.count)
       ){

        
        if((!sim->signal.pre_valid_c) || sim->signal.ready_c){

            sim->pre_chain_id = sim->cur_chain_id;

            if(sim->cur_chain_id == sim->tail_ptr){
                sim->cur_chain_id = sim->head_ptr;
            }else{
                sim->cur_chain_id = sim->chain.at(sim->cur_chain_id).next_ptr;
            }
        }

        *id_c    = sim->chain.at(sim->cur_chain_id).id;
        *qos_c   = sim->chain.at(sim->cur_chain_id).qos;

        *payload_c_0 = sim->mem.payload[sim->cur_chain_id][0];
        *payload_c_1 = sim->mem.payload[sim->cur_chain_id][1];
        *payload_c_2 = sim->mem.payload[sim->cur_chain_id][2];
        *payload_c_3 = sim->mem.payload[sim->cur_chain_id][3];

        if( (!sim->chain.at(sim->cur_chain_id).lock) && 
            ( (sim->chain.at(sim->cur_chain_id).qos == sim->getHighestQos() &&
               (sim->chain.at(sim->cur_chain_id).id != sim->chain.at(sim->head_ptr).id ) &&
               (sim->cur_chain_id != sim->head_ptr)
               ) ||
              (sim->mem.count == 1) ||
              (!sim->isIDUnique(sim->cur_chain_id))
              )){
#ifdef ENIGMA_DEBUG
        fprintf(stderr, " +S FLIT: " );
        sim->showFlitInfo( sim->cur_chain_id );
#endif
                *valid_c = 1;
            }else{
                *valid_c = 0;
            }

    }else{
        *valid_c = 0;
    }
        
    if(sim->cur_out_vld_mark)
        sim->last_chain_id      = sim->cur_chain_id;
    // *************** INNER FUNCTIONS *******************

    if(sim->update_tail_nptr_en){
        sim->chain.at(sim->tail_ptr).next_ptr = sim->mem.nptr;
        sim->update_tail_nptr_en = 0;
    }

    // port A or B
    if( (sim->signal.pre_ready_a && sim->signal.valid_a) ||
        (sim->signal.pre_ready_b && sim->signal.valid_b)
       ){ 

        if(sim->mem.full){
            fprintf(stderr," +Error: @PORT %s IN, memory FULL is detected when recieve FLIT. check design !!! @%x\n", 
                    (sim->signal.pre_ready_a && sim->signal.valid_a) ? "A" : "B",
                    sim->signal.tick);
            return ;
        }

        sim->chain.at(sim->mem.nptr).lock = 0;

        sim->chain.at(sim->mem.nptr).next_ptr = sim->mem.nnptr;

        if(sim->signal.pre_ready_a && sim->signal.valid_a){
            sim->chain.at(sim->mem.nptr).id   = sim->signal.id_a;
            sim->chain.at(sim->mem.nptr).qos  = sim->signal.qos_a;
            
            sim->mem.payload[sim->mem.nptr][0] = sim->signal.payload_a_0;
            sim->mem.payload[sim->mem.nptr][1] = sim->signal.payload_a_1;
            sim->mem.payload[sim->mem.nptr][2] = sim->signal.payload_a_2;
            sim->mem.payload[sim->mem.nptr][3] = sim->signal.payload_a_3;
        }else{
            sim->chain.at(sim->mem.nptr).id   = sim->signal.id_b | ENIGMA_ID_EXP;
            sim->chain.at(sim->mem.nptr).qos  = sim->signal.qos_b;
            
            sim->mem.payload[sim->mem.nptr][0] = sim->signal.payload_b_0;
            sim->mem.payload[sim->mem.nptr][1] = sim->signal.payload_b_1;
            sim->mem.payload[sim->mem.nptr][2] = sim->signal.payload_b_2;
            sim->mem.payload[sim->mem.nptr][3] = sim->signal.payload_b_3;
        }

        sim->qos_count[sim->chain.at(sim->mem.nptr).qos]++;
			
        sim->tail_ptr  = sim->mem.nptr;
        if(sim->pre_out_vld_mark)
            sim->update_tail_nptr_en = 1;

        // update status
        sim->mem.vld_bits              |= (1 << sim->mem.nptr);
        sim->mem.count++;

        sim->mem.full  = sim->mem.count == ENIGMA_MEM_MAX_SIZE;
        sim->mem.empty = sim->mem.count == 0;

        // mark first lowest Qos
        if((sim->mem.vld_bits & (1<< sim->lptr))  ){
            if(sim->chain.at(sim->lptr).qos > sim->chain.at(sim->mem.nptr).qos)
                sim->lptr = sim->mem.nptr;
        }

        // mark first lowest Qos
        if((sim->mem.vld_bits & (1<< sim->hptr))  ){
            if(sim->chain.at(sim->hptr).qos < sim->chain.at(sim->mem.nptr).qos)
                sim->hptr = sim->mem.nptr;
        }

#ifdef ENIGMA_DEBUG
        fprintf(stderr, " +%s FLIT: " ,
                (sim->signal.pre_ready_a && sim->signal.valid_a) ? "A" : "B"
                );
        sim->showFlitInfo( sim->mem.nptr );
        sim->showFlitAll();
#endif
        
    }

	
    sim->pre_out_vld_mark       = sim->cur_out_vld_mark;

    //fprintf(stderr, " +> %8x : [%d] : %d  @%x\n", sim->mem.vld_bits, sim->mem.full, sim->mem.nptr, sim->signal.tick );

    sim->updateMemStatus();

    sim->portSel = ( (sim->signal.valid_b && sim->signal.valid_a) ? (!sim->portSel):
                     (sim->signal.valid_b && (!sim->signal.valid_a)) );

    // generate ready to portA or portB
    *ready_a = !sim->mem.full  && (sim->portSel == PORT_SEL_A) && !sim->dim_qos_en;
    *ready_b = !sim->mem.full  && (sim->portSel == PORT_SEL_B) && !sim->dim_qos_en;
    

    // debug using
    *chain_id    = sim->cur_chain_id | (sim->last_chain_id << 6) | (sim->pre_chain_id << 12);
    *chain_size  = sim->mem.count;
    *pre_out_vld = sim->pre_out_vld_mark;
    *max_qos     = sim->getHighestQos();
    *dim_qos_en  = sim->dim_qos_en;
    *portSel     = sim->portSel;
    *nptr        = sim->mem.nptr | (sim->mem.nnptr << 6) | (sim->head_ptr << 12) | (sim->tail_ptr << 18);

    // backup current cycle value
    sim->signal.pre_ready_a		= *ready_a    ; 	  
    sim->signal.pre_ready_b		= *ready_b    ;	  
    sim->signal.pre_payload_c_0	= *payload_c_0;
    sim->signal.pre_payload_c_1	= *payload_c_1;
    sim->signal.pre_payload_c_2	= *payload_c_2;
    sim->signal.pre_payload_c_3	= *payload_c_3;
    sim->signal.pre_id_c		= *id_c       ;
    sim->signal.pre_qos_c		= *qos_c      ;
    sim->signal.pre_valid_c		= *valid_c    ;
	
}
   
													 

void enigma_buf_port_i( void *             app,
                        const svBitVecVal *payload_a_0,
                        const svBitVecVal *payload_a_1,
                        const svBitVecVal *payload_a_2,
                        const svBitVecVal *payload_a_3,
                        const svBitVecVal *id_a,
                        const svBitVecVal *qos_a,
                        const svBit  	   valid_a,

                        const svBitVecVal *payload_b_0,
                        const svBitVecVal *payload_b_1,
                        const svBitVecVal *payload_b_2,
                        const svBitVecVal *payload_b_3,
                        const svBitVecVal *id_b,
                        const svBitVecVal *qos_b,
                        const svBit  	   valid_b,

                        const svBit  	   ready_c,

                        const svBit  	   conflict_c,
                        const svBit  	   release_c,
                        const svBitVecVal *releaseid_c,
                        //debug using
                        const svBitVecVal *tick
                        ){


    EnigmaBuf *sim = (EnigmaBuf *)app;

    sim->signal.payload_a_0  = *payload_a_0;
    sim->signal.payload_a_1  = *payload_a_1;
    sim->signal.payload_a_2  = *payload_a_2;
    sim->signal.payload_a_3  = *payload_a_3;
    sim->signal.id_a         = *id_a       & 0x1F;
    sim->signal.qos_a        = *qos_a      & 0x3;
    sim->signal.valid_a      = valid_a    ;
								              
    sim->signal.payload_b_0  = *payload_b_0;
    sim->signal.payload_b_1  = *payload_b_1;
    sim->signal.payload_b_2  = *payload_b_2;
    sim->signal.payload_b_3  = *payload_b_3;
    sim->signal.id_b         = *id_b       & 0x1F;
    sim->signal.qos_b        = *qos_b      & 0x3;
    sim->signal.valid_b      = valid_b    ;
								              
    sim->signal.ready_c      = ready_c    ;
								              
    sim->signal.conflict_c   = conflict_c ;
    sim->signal.release_c    = release_c  ;
    sim->signal.releaseid_c  = *releaseid_c & 0x3F;

    sim->signal.tick         = *tick  ;
}


