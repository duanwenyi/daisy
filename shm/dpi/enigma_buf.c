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


	fprintf(stderr," Enigma Buffer initialed !\n");
	
}


void EnigmaBuf::consumOneCell(int id){

    vector<ENIGMA_FLIT_U>::iterator iter = chain.begin();

    iter += id;


    mem.vld_bits  &= ~(1 << iter->addr);  // unmask the bit
    mem.count--;

    chain.erase( iter );
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
    fprintf(stderr," +ChainID[%2d]  : ID[%2d]-[%2d] QOS[%d] OrignalPort[%s] Mem Addr[%2d][%8x] Payload:[%8x %8x %8x %8x] --> %d items\n", 
            chain_id,
            chain.at(chain_id).id,
            chain.at(chain_id).id & ENIGMA_ID_MSK,
            chain.at(chain_id).qos,
            (chain.at(chain_id).id & ENIGMA_ID_EXP ) ? "B" : "A",
            chain.at(chain_id).addr,
            mem.vld_bits,
            mem.payload[chain.at(chain_id).addr][0],
            mem.payload[chain.at(chain_id).addr][1],
            mem.payload[chain.at(chain_id).addr][2],
            mem.payload[chain.at(chain_id).addr][3],
            chain.size()
            );

}

void EnigmaBuf::lockAllId(int id){
    vector<ENIGMA_FLIT_U>::iterator iter;

    for(iter = chain.begin(); iter != chain.end(); iter++){  
        if(iter->id == id){
            if(iter->lock == 1){
                fprintf(stderr," +Error: @lockAllId : re-lock of ID %d [0x%x] is detected !\n", id, id);
            }
            iter->lock = 1;
            mem.lock_count++;

            qos_count[iter->qos]--;
        }  
    }
}

void EnigmaBuf::unlockAllId(int id){
    vector<ENIGMA_FLIT_U>::iterator iter;

    for(iter = chain.begin(); iter != chain.end(); iter++){  
        if(iter->id == id){
            if(iter->lock == 0){
                fprintf(stderr," +Error: @unlockAllId : re-unlock of ID %d [0x%x] is detected !\n", id, id);
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

    return cc;
}

extern "C" {

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
							svBit       * valid_c
							){
		EnigmaBuf *sim = (EnigmaBuf *)app;

        // *************** INNER FUNCTIONS *******************
        // invoke one valid FLIT to sending !
        if(!sim->isMemEmpty() &&
           (sim->mem.lock_count < sim->mem.count)
           ){
            
            sim->cur_chain_id = sim->getValidCellId();

            *id_c    = sim->chain.at(sim->cur_chain_id).id;
            *qos_c   = sim->chain.at(sim->cur_chain_id).qos;
            
            int addr = sim->chain.at(sim->cur_chain_id).addr;

            *payload_c_0 = sim->mem.payload[addr][0];
            *payload_c_1 = sim->mem.payload[addr][1];
            *payload_c_2 = sim->mem.payload[addr][2];
            *payload_c_3 = sim->mem.payload[addr][3];

            *valid_c = 1;

#ifdef ENIGMA_DEBUG
            fprintf(stderr, " +S FLIT: " );
            sim->showFlitInfo( sim->cur_chain_id );
#endif

        }else{
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
                fprintf(stderr," +Error: @PORT A IN, memory FULL is detected when recieve FLIT. check design !!!\n");
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
        *ready_b = !(sim->isMemNearFull() | sim->signal.pre_ready_a) && !sim->dim_qos_en;

		// port B
		if(sim->signal.pre_ready_b && sim->signal.valid_b ){
            cellB.lock       = 0;
			cellB.id         = sim->signal.id_b | ENIGMA_ID_EXP;
			cellB.qos        = sim->signal.qos_b;

            if(sim->isMemFull()){
                fprintf(stderr," +Error: @PORT B IN, memory FULL is detected when recieve FLIT. check design !!!\n");
                return ;
            }
            cellB.addr       = sim->getFreeAddr();

            sim->mem.vld_bits              |= (1 << cellB.addr);
            sim->mem.count++;
            sim->mem.payload[cellB.addr][0] = sim->signal.payload_a_0;
            sim->mem.payload[cellB.addr][1] = sim->signal.payload_a_1;
            sim->mem.payload[cellB.addr][2] = sim->signal.payload_a_2;
            sim->mem.payload[cellB.addr][3] = sim->signal.payload_a_3;

            sim->qos_count[cellB.qos]++;

			sim->chain.push_back( cellB );

#ifdef ENIGMA_DEBUG
            fprintf(stderr, " +B FLIT: " );
            sim->showFlitInfo( sim->chain.size() - 1 );
#endif
		}


		// port C
        if( sim->pre_out_vld_mark ){
            if( sim->signal.conflict_c ){
                sim->lockAllId( sim->last_chain_id );
            }else{

                // gen dim qos
                if( !sim->dim_qos_en &&
                    (sim->chain.at(sim->last_chain_id).qos == sim->last_qos) && 
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


                sim->last_qos = sim->chain.at(sim->last_chain_id).qos;

                sim->consumOneCell( sim->last_chain_id );
            }
        }

        if(sim->dim_qos_en && sim->isMemEmpty()){
            sim->dim_qos_en = 0;
        }
        
		if( sim->signal.release_c ){
            sim->unlockAllId( sim->signal.releaseid_c );
        }
		

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

        sim->pre_out_vld_mark       = sim->signal.pre_valid_c && sim->signal.ready_c;
        sim->last_chain_id          = sim->cur_chain_id;
		
	}
   
													 

	void enigma_buf_port_i( void *             app,
							const svBitVecVal  payload_a_0,
							const svBitVecVal  payload_a_1,
							const svBitVecVal  payload_a_2,
							const svBitVecVal  payload_a_3,
							const svBitVecVal  id_a,
							const svBitVecVal  qos_a,
							const svBit  	   valid_a,

							const svBitVecVal  payload_b_0,
							const svBitVecVal  payload_b_1,
							const svBitVecVal  payload_b_2,
							const svBitVecVal  payload_b_3,
							const svBitVecVal  id_b,
							const svBitVecVal  qos_b,
							const svBit  	   valid_b,

							const svBit  	   ready_c,

							const svBit  	   conflict_c,
							const svBit  	   release_c,
							const svBitVecVal  releaseid_c
							){


		EnigmaBuf *sim = (EnigmaBuf *)app;

		sim->signal.payload_a_0  = payload_a_0;
		sim->signal.payload_a_1  = payload_a_1;
		sim->signal.payload_a_2  = payload_a_2;
		sim->signal.payload_a_3  = payload_a_3;
		sim->signal.id_a         = id_a       & 0x1F;
		sim->signal.qos_a        = qos_a      & 0x3;
		sim->signal.valid_a      = valid_a    ;
								              
		sim->signal.payload_b_0  = payload_b_0;
		sim->signal.payload_b_1  = payload_b_1;
		sim->signal.payload_b_2  = payload_b_2;
		sim->signal.payload_b_3  = payload_b_3;
		sim->signal.id_b         = id_b       & 0x1F;
		sim->signal.qos_b        = qos_b      & 0x3;
		sim->signal.valid_b      = valid_b    ;
								              
		sim->signal.ready_c      = ready_c    ;
								              
		sim->signal.conflict_c   = conflict_c ;
		sim->signal.release_c    = release_c  ;
		sim->signal.releaseid_c  = releaseid_c & 0x3F;

	}

}
