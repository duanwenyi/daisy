#include <cstdio>
#include <cstdlib>
#include <iostream> 
#include <sstream> 
#include <fstream>
#include "enigma.h"

#define ENIGMA_DEBUG

EnigmaBuf::EnigmaBuf(){
	chain.clear();

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


	fprintf(stderr," Enigma Buffer initialed !\n");
	
}


void EnigmaBuf::consumOneCell(int id){
    if(id < chain.size()){
        vector<ENIGMA_FLIT_U>::iterator iter = chain.begin();

        iter += id;


        mem.vld_bits  &= ~(1 << iter->addr);  // unmask the bit
        mem.count--;

#ifdef ENIGMA_DEBUG
        fprintf(stderr, " +C FLIT: " );
        showFlitInfo( id );
#endif
        if(qos_count[iter->qos] > 0)  // forbiden error
            qos_count[iter->qos]--;
        else
            fprintf(stderr," +Error: @consumOneCell : Qos %d is 0 when reduce counter! @%x\n", iter->qos, signal.tick);
            

        chain.erase( iter );
    }else{
        fprintf(stderr," +Error: @consumOneCell : ID %d not in Chain max size %d ! @%x\n", id, (int)chain.size(), signal.tick);
    }
}

bool EnigmaBuf::isMemFull(){
    return ( mem.count == ENIGMA_MEM_MAX_SIZE );
}

bool EnigmaBuf::isMemNearFull(){
    return ( mem.count == (ENIGMA_MEM_MAX_SIZE - 1) );
}

bool EnigmaBuf::isMemEmpty(){
    return ( mem.count == 0 );

}

int  EnigmaBuf::getFreeAddr(){
    int cc = 0;
    for( cc=0; cc< ENIGMA_MEM_MAX_SIZE; cc++ ){
        if( (mem.vld_bits & (1 << cc)) == 0 ){
            mem.vld_bits |= (1 << cc);
            break;
        }
    }
    
    return cc;
}

void EnigmaBuf::showFlitInfo(int chain_id){
    if(chain_id < chain.size()){
        fprintf(stderr," +ChainID[%2d]  : [%s]ID[%2x]-[%2x] QOS[%d]  Mem Addr[%2x][%8x][%2d-%2d] Payload:[%8x %8x %8x %8x] +lastQos[%d->%d:%2d] +->%d items @%x\n", 
                chain_id,
                (chain.at(chain_id).id & ENIGMA_ID_EXP ) ? "B" : "A",
                chain.at(chain_id).id,
                chain.at(chain_id).id & ENIGMA_ID_MSK,
                chain.at(chain_id).qos,
                chain.at(chain_id).addr,
                mem.vld_bits,
                mem.count,
                mem.lock_count,
                mem.payload[chain.at(chain_id).addr][0],
                mem.payload[chain.at(chain_id).addr][1],
                mem.payload[chain.at(chain_id).addr][2],
                mem.payload[chain.at(chain_id).addr][3],
                pre_last_qos,
                last_qos,
                seq_qos_count,
                chain.size(),
                signal.tick
                );
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

int EnigmaBuf::getValidCellId(){
    int qos = getHighestQos();
    int cc;
    for(cc=0; cc < chain.size(); cc++){
        if( (chain.at(cc).qos == qos) && 
            (chain.at(cc).lock == 0)
            ){
            break;
        }
    }

    if(cc == chain.size()){
        return -1;
    }else{
        return cc;
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
                        svBit       * dim_qos_en
                        ){
    EnigmaBuf *sim = (EnigmaBuf *)app;

    // port C
    if( sim->pre_out_vld_mark ){
        if( sim->signal.conflict_c ){
            sim->lockAllId( sim->last_chain_id );
        }else{

            // gen dim qos
            if( !sim->isMemEmpty() &&
                !sim->dim_qos_en &&
                (sim->pre_last_qos == sim->last_qos) && 
                sim->isLowQosNotEmpty( sim->last_qos )
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

            sim->consumOneCell( sim->last_chain_id );
        }
    }

    if(sim->dim_qos_en && sim->isMemEmpty()){
        sim->dim_qos_en = 0;
    }
        
    if( sim->signal.release_c ){
        sim->unlockAllId( sim->signal.releaseid_c );
    }

    // *************** INNER FUNCTIONS *******************
    // invoke one valid FLIT to sending !
    if(!sim->isMemEmpty() &&
       (sim->mem.lock_count < sim->mem.count) &&
       (sim->getValidCellId() != -1)
       ){
            
        sim->cur_chain_id = sim->getValidCellId();
        
        if(sim->cur_chain_id < sim->chain.size()){
            *id_c    = sim->chain.at(sim->cur_chain_id).id;
            *qos_c   = sim->chain.at(sim->cur_chain_id).qos;
            
            int addr = sim->chain.at(sim->cur_chain_id).addr;

            *payload_c_0 = sim->mem.payload[addr][0];
            *payload_c_1 = sim->mem.payload[addr][1];
            *payload_c_2 = sim->mem.payload[addr][2];
            *payload_c_3 = sim->mem.payload[addr][3];

            *valid_c = 1;
        }else{
            *valid_c = 0;
            fprintf(stderr," +Error: @C : invoke illegal chain ID %d when size is %d ! @%x\n", 
                    sim->cur_chain_id , (int)sim->chain.size(), sim->signal.tick);
        }

#ifdef ENIGMA_DEBUG
        fprintf(stderr, " +S FLIT: " );
        sim->showFlitInfo( sim->cur_chain_id );
#endif

    }else{
        sim->cur_chain_id = 0;
        *valid_c = 0;
    }
        
    // *************** INNER FUNCTIONS *******************

    ENIGMA_FLIT_U cellA;
    ENIGMA_FLIT_U cellB;

    // port A
    if(sim->signal.pre_ready_a && sim->signal.valid_a ){ 
        cellA.lock       = 0;
        cellA.id         = sim->signal.id_a;
        cellA.qos        = sim->signal.qos_a;

        if(sim->isMemFull()){
            fprintf(stderr," +Error: @PORT A IN, memory FULL is detected when recieve FLIT. check design !!! @%x\n", sim->signal.tick);
            return ;
        }
        cellA.addr       = sim->getFreeAddr();
            
        sim->mem.vld_bits              |= (1 << cellA.addr);
        sim->mem.count++;
        sim->mem.payload[cellA.addr][0] = sim->signal.payload_a_0;
        sim->mem.payload[cellA.addr][1] = sim->signal.payload_a_1;
        sim->mem.payload[cellA.addr][2] = sim->signal.payload_a_2;
        sim->mem.payload[cellA.addr][3] = sim->signal.payload_a_3;

        sim->qos_count[cellA.qos]++;
			
        sim->chain.push_back( cellA );

#ifdef ENIGMA_DEBUG
        fprintf(stderr, " +A FLIT: " );
        sim->showFlitInfo( sim->chain.size() - 1 );
#endif
    }

    *ready_a = !sim->isMemFull()  && !sim->dim_qos_en;
    *ready_b = !(sim->isMemNearFull() && sim->signal.pre_ready_a) && !sim->dim_qos_en;

    // port B
    if(sim->signal.pre_ready_b && sim->signal.valid_b ){
        cellB.lock       = 0;
        cellB.id         = sim->signal.id_b | ENIGMA_ID_EXP;
        cellB.qos        = sim->signal.qos_b;

        if(sim->isMemFull()){
            fprintf(stderr," +Error: @PORT B IN, memory FULL is detected when recieve FLIT. check design !!! @%x\n",sim->signal.tick);
            return ;
        }
        cellB.addr       = sim->getFreeAddr();

        sim->mem.vld_bits              |= (1 << cellB.addr);
        sim->mem.count++;
        sim->mem.payload[cellB.addr][0] = sim->signal.payload_b_0;
        sim->mem.payload[cellB.addr][1] = sim->signal.payload_b_1;
        sim->mem.payload[cellB.addr][2] = sim->signal.payload_b_2;
        sim->mem.payload[cellB.addr][3] = sim->signal.payload_b_3;

        sim->qos_count[cellB.qos]++;

        sim->chain.push_back( cellB );

#ifdef ENIGMA_DEBUG
        fprintf(stderr, " +B FLIT: " );
        sim->showFlitInfo( sim->chain.size() - 1 );
#endif
    }


		
    sim->pre_out_vld_mark       = sim->signal.pre_valid_c && sim->signal.ready_c;
    if(sim->pre_out_vld_mark)
        sim->last_chain_id      = sim->cur_chain_id;

    // debug using
    *chain_id    = sim->cur_chain_id;
    *chain_size  = sim->chain.size();
    *pre_out_vld = sim->pre_out_vld_mark;
    *max_qos     = sim->getHighestQos();
    *dim_qos_en  = sim->dim_qos_en;

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


