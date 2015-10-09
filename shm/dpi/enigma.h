#include "svdpi.h"

void *engima_init( int TEST_ID);

void engima_port_com_sim( void *            app,
						  const svBit       ready_a, 	  
						  const svBit       ready_b,	  

						  const svBitVecVal payload_c_0,
						  const svBitVecVal payload_c_1,
						  const svBitVecVal payload_c_2,
						  const svBitVecVal payload_c_3,
						  const svBitVecVal id_c,
						  const svBitVecVal qos_c,
						  const svBit       valid_c
						  );
   
													 

void engima_port_A_sim( void *        app,
						svBitVecVal * payload_a_0,
						svBitVecVal * payload_a_1,
						svBitVecVal * payload_a_2,
						svBitVecVal * payload_a_3,
						svBitVecVal * id_a,
						svBitVecVal * qos_a,
						svBit * 	  valid_a
						);

void engima_port_B_sim( void *        app,
						svBitVecVal * payload_b_0,
						svBitVecVal * payload_b_1,
						svBitVecVal * payload_b_2,
						svBitVecVal * payload_b_3,
						svBitVecVal * id_b,
						svBitVecVal * qos_b,
						svBit * 	  valid_b
						);

void engima_port_C_sim( void *        app,
						svBit * 	  ready_c,

						svBit * 	  conflict_c,
						svBit * 	  release_c,
						svBitVecVal * releaseid_c,
												   
						svBit *		  error
						);
