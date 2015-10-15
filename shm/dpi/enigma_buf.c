#include <cstdio>
#include <cstdlib>
#include <iostream> 
#include <sstream> 
#include <fstream>
#include "enigma.h"

EnigmaBuf::EnigmaBuf(){
	portA.clear();
	portB.clear();
	Pending.clear();


	fprintf(stderr," Enigma Buffer initialed !\n");
	
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

		ENIGMA_FLIT cellA;
		ENIGMA_FLIT cellB;

		// port A
		if(sim->signal.pre_ready_a && sim->signal.valid_a ){
			cellA.port = 0;
			cellA.id         = sim->signal.id_a;
			cellA.qos        = sim->signal.qos_a;
			cellA.payload[0] = sim->signal.payload_a_0;
			cellA.payload[1] = sim->signal.payload_a_1;
			cellA.payload[2] = sim->signal.payload_a_2;
			cellA.payload[3] = sim->signal.payload_a_3;
			
			sim->portA.push_back( cellA );
		}

		// port B
		if(sim->signal.pre_ready_b && sim->signal.valid_b ){
			cellB.port = 0;
			cellB.id         = sim->signal.id_b;
			cellB.qos        = sim->signal.qos_b;
			cellB.payload[0] = sim->signal.payload_b_0;
			cellB.payload[1] = sim->signal.payload_b_1;
			cellB.payload[2] = sim->signal.payload_b_2;
			cellB.payload[3] = sim->signal.payload_b_3;
			
			sim->portB.push_back( cellB );
		}


		// port C
		


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
