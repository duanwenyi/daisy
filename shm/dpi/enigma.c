#include <cstdio>
#include <cstdlib>
#include <iostream> 
#include <sstream> 
#include <fstream>
#include "enigma.h"

#define ENIGMA_SIM_DEBUG

EnigmaSim::EnigmaSim(){
	portA.clear();
	portB.clear();
	dutActive.clear();
	dutPending.clear();
	portC.clear();
	
	conflictC.clear();
	releaseC.clear();

	release_delay.clear();
	stim_mode = 0;

    signal.pre_valid_a = 0;
    signal.pre_valid_a = 0;
    signal.pre_ready_c = 0;

	srand(0);

	loadStimulate();

	genOneFlitA( 5, LEVEL2);
	genOneFlitB( 5, LEVEL3);

	fprintf(stderr," Enigma Simulator initialed !\n");
	
}

void EnigmaSim::setRandomSeed(int seed){
	srand(seed);
}

void EnigmaSim::loadStimulate(){
	std::ifstream fin("stim.txt", std::ios::in); 
	char line[1024]={0}; 
	std::string port  = ""; 
	std::string id_c  = ""; 
	std::string qos_c = ""; 
	int id  = 0;
	int qos = 0;
	if(!fin){
		std::cout << " +No stim.txt file . Begin generate stimualte with auto random mode !\n" << std::endl;
	}else{
		std::cout << "Port ID Qos  --> loading " << std::endl;
		stim_mode = 1;

		while(fin.getline(line, sizeof(line))) 
			{ 
				std::stringstream word(line); 
				word >> port; 
				word >> id_c; 
				word >> qos_c; 

				id  = std::atoi( id_c.c_str());
				qos = std::atoi( qos_c.c_str());

				if(qos == -1){
					qos = rand() & 0x3;
				}
				
				if(port.compare("A") == 0){
					if(id == -1){
						id = rand() & 0x1F;
					}

					genOneFlitA( id, qos);
				}else if(port.compare("B") == 0){
					if(id == -1){
						id = rand() & 0x1F;
					}

					genOneFlitB( id, qos);
				}else if(port.compare("C") == 0){
					if(id == -1){
						id = rand() & 0x3F;
					}
					joinOneFlit( &conflictC, !!(id & 0x20) , id & 0x1F, qos);
				}else if(port.compare("R") == 0){
					if(id == -1){
						id = rand() & 0x3F;
					}
					joinOneFlit( &releaseC, !!(id & 0x20) , id & 0x1F, qos);
				}

                fprintf(stderr," %s\t %x\t %x \n", port.c_str(), id, qos);
				//std::cout << port << " " << id << " " << qos << std::endl; 
			} 
		fin.clear(); 
	}
	fin.close(); 

}

void EnigmaSim::joinOneFlit(vector<ENIGMA_FLIT> *group, int port, int id, int qos){
	
	ENIGMA_FLIT cell;
	cell.port = port;  // 0:A   1:B
	cell.id   = id  & 0x1F;
	cell.qos  = qos & 0x3;
	cell.payload[0] = id | (id<<8) |(port<<14);
	cell.payload[1] = qos;
	cell.payload[2] = port;
	cell.payload[3] = rand();   // for output check

	group->insert( group->begin(), cell );
}

void EnigmaSim::genOneFlitA(int id, int qos){
	joinOneFlit( &portA, 0, id, qos);
}

void EnigmaSim::genOneFlitB(int id, int qos){
	joinOneFlit( &portB, 1, id, qos);
}

extern "C" {

	void *enigma_init( int TEST_ID){
	
		EnigmaSim *app = new EnigmaSim();

		return (void *)app;
	}

	void enigma_port_com_sim( void *             app,
							  const svBit        ready_a, 	  
							  const svBit        ready_b,	  

							  const svBitVecVal *payload_c_0,
							  const svBitVecVal *payload_c_1,
							  const svBitVecVal *payload_c_2,
							  const svBitVecVal *payload_c_3,
							  const svBitVecVal *id_c,
							  const svBitVecVal *qos_c,
							  const svBit        valid_c,
							  const svBitVecVal *tick
							  ){

		EnigmaSim *sim = (EnigmaSim *)app;

		sim->signal.ready_a       = ready_a    ;
		sim->signal.ready_b    	  = ready_b    ;
		sim->signal.payload_c_0	  = *payload_c_0;
		sim->signal.payload_c_1	  = *payload_c_1;
		sim->signal.payload_c_2	  = *payload_c_2;
		sim->signal.payload_c_3	  = *payload_c_3;
		sim->signal.id_c       	  = *id_c       & 0x3F;  // 6 bits
		sim->signal.qos_c      	  = *qos_c      & 0x3;   // 2 bits
		sim->signal.valid_c    	  = valid_c    ;
		sim->signal.tick    	  = *tick      ;
	}
   
													 

	void enigma_port_A_sim( void *        app,
							svBitVecVal * payload_a_0,
							svBitVecVal * payload_a_1,
							svBitVecVal * payload_a_2,
							svBitVecVal * payload_a_3,
							svBitVecVal * id_a,
							svBitVecVal * qos_a,
							svBit * 	  valid_a
							){
		EnigmaSim *sim = (EnigmaSim *)app;

        ENIGMA_FLIT cell;

		if(sim->portA.size() > 0){
            cell = sim->portA.back();

			*valid_a = rand()%2;
		
			if(sim->signal.pre_valid_a && sim->signal.ready_a){
			
				sim->portA.pop_back();

				sim->dutActive.push_back( cell );  // port A
#ifdef ENIGMA_SIM_DEBUG				
				fprintf(stderr," +POST PORT[A] : ID[%2x] QOS[%d]! Payload: %8x %8x %8x %8x  :: remain %d items @%x\n",
						cell.id, cell.qos, 
						cell.payload[0], 
						cell.payload[1], 
						cell.payload[2], 
						cell.payload[3],
						sim->portA.size(),
                        sim->signal.tick
						);
#endif
                if(sim->portA.size() > 0){
                    cell = sim->portA.back();
                }else{
                    *valid_a = 0;
                }
			}
            

            if(*valid_a){
                *id_a   = cell.id;
                *qos_a  = cell.qos;
                *payload_a_0  = cell.payload[0];
                *payload_a_1  = cell.payload[1];
                *payload_a_2  = cell.payload[2];
                *payload_a_3  = cell.payload[3];
            }

		}else{
			*valid_a = 0;
		}

		// backup current cycle value
		sim->signal.pre_valid_a    	  = *valid_a;
	}

	void enigma_port_B_sim( void *        app,
							svBitVecVal * payload_b_0,
							svBitVecVal * payload_b_1,
							svBitVecVal * payload_b_2,
							svBitVecVal * payload_b_3,
							svBitVecVal * id_b,
							svBitVecVal * qos_b,
							svBit * 	  valid_b
							){
		EnigmaSim *sim = (EnigmaSim *)app;

        ENIGMA_FLIT cell;

		if(sim->portB.size() > 0){
            cell = sim->portB.back();

			*valid_b = rand()%2;
		
			if(sim->signal.pre_valid_b && sim->signal.ready_b){
			
				sim->portB.pop_back();

				sim->dutActive.push_back( cell );  // port B
#ifdef ENIGMA_SIM_DEBUG				
				fprintf(stderr," +POST PORT[B] : ID[%2x] QOS[%d]! Payload: %8x %8x %8x %8x  :: remain %d items @%x\n",
						cell.id, cell.qos, 
						cell.payload[0], 
						cell.payload[1], 
						cell.payload[2], 
						cell.payload[3],
						sim->portB.size(),
                        sim->signal.tick
						);
#endif
                if(sim->portB.size() > 0){
                    cell = sim->portB.back();
                }else{
                    *valid_b = 0;
                }

                if(sim->portB.size() == 0){
                    *valid_b = 0;
                }
			}
            

            if(*valid_b){
                *id_b   = cell.id;
                *qos_b  = cell.qos;
                *payload_b_0  = cell.payload[0];
                *payload_b_1  = cell.payload[1];
                *payload_b_2  = cell.payload[2];
                *payload_b_3  = cell.payload[3];
            }

		}else{
			*valid_b = 0;
		}

		// backup current cycle value
		sim->signal.pre_valid_b    	  = *valid_b;
	}

	void enigma_port_C_sim( void *        app,
							svBit * 	  ready_c,

							svBit * 	  conflict_c,
							svBit * 	  release_c,
							svBitVecVal * releaseid_c,
												   
							svBit *		  error
							){

		EnigmaSim *sim = (EnigmaSim *)app;

		*error      = 0;

		*conflict_c = 0;
        *ready_c    = rand()%2;
#if 0
		// process Realease
		if(sim->dutPending.size() > 0){
			vector<int>::iterator iterA = sim->release_delay.begin();
			vector<ENIGMA_FLIT>::iterator iterB	= sim->dutPending.begin();
			for(; iterB != sim->dutPending.end();){

				if( (*iterA) > 0)
					(*iterA)--;

				if((*iterA) == 0){
					*release_c  = 1;
					*releaseid_c = (*iterB).id | ((*iterB).port << 6);

					// remove one conflicted pending item
					sim->dutPending.erase(iterB);
					sim->release_delay.erase(iterA);


					fprintf(stderr, " +Release Flit :Port[%s], ID[%2x], QOS[%d], Payload[%8x %8x %8x %8x] -- remain pending %d FLIT\n",
							(*iterB).port? "B":"A", (*iterB).id, (*iterB).qos,
							(*iterB).payload[3], 
							(*iterB).payload[2], 
							(*iterB).payload[1], 
							(*iterB).payload[0],
							sim->dutPending.size()
							);
					break;
				}

				iterA++;
				iterB++;
			}
			

		}else{
			sim->release_delay.clear();

			*release_c   = 0;
			*releaseid_c = rand() & 0x3F;
		}


		// process Conflict
		if( (sim->pre_out_vld_mark == VALID_OUT) ){

			if(*conflict_c){

				sim->dutPending.push_back( sim->ocell );
				sim->release_delay.push_back( rand()%64 + 1 );

			}else{
				sim->portC.push_back( sim->ocell );

				// remove output Flit from Active group
				vector<ENIGMA_FLIT>::iterator iter	= sim->dutActive.begin();
				for(; iter != sim->dutActive.end();){
					if( (*iter).port == sim->ocell.port &&
						(*iter).id   == sim->ocell.id
						){

					fprintf(stderr, " +OUT Flit :Port[%s], ID[%2x], QOS[%d], Payload[%8x %8x %8x %8x] -- remain active %d FLIT\n",
							(*iter).port? "B":"A", (*iter).id, (*iter).qos,
							(*iter).payload[3], 
							(*iter).payload[2], 
							(*iter).payload[1], 
							(*iter).payload[0],
							sim->dutActive.size() - 1
							);

						// same ID must be the first !
						sim->dutActive.erase(iter);
						break;
					}
				}
			}
		}


		if(sim->signal.pre_ready_c && sim->signal.valid_c){
			sim->ocell.port = !!(sim->signal.id_c & 0x20);
			sim->ocell.id   = sim->signal.id_c & 0x1F;
			sim->ocell.qos  = sim->signal.id_c;

			sim->ocell.payload[0] = sim->signal.payload_c_0;
			sim->ocell.payload[1] = sim->signal.payload_c_1;
			sim->ocell.payload[2] = sim->signal.payload_c_2;
			sim->ocell.payload[3] = sim->signal.payload_c_3;

			sim->pre_out_vld_mark = VALID_OUT;
#if 0
			// simple check payload
			
#endif
			// check whether ID is conflicted
			for(int cc=0; cc< sim->dutPending.size(); cc++){
				if(sim->ocell.id == sim->dutPending.at(cc).id &&
				   sim->ocell.port == sim->dutPending.at(cc).port
				   ){
					fprintf(stderr, " +Error: Conflicted Flit found to be sent :Port[%s], ID[%2x], QOS[%d], Payload[%8x %8x %8x %8x]\n",
							sim->ocell.port? "B":"A", sim->ocell.id, sim->ocell.qos,
							sim->ocell.payload[3], 
							sim->ocell.payload[2], 
							sim->ocell.payload[1], 
							sim->ocell.payload[0] 
							);
					*error = 1;
					break;
				}

			}

		}else{
			
			sim->pre_out_vld_mark = INVALID_OUT;
		}
#endif
		// backup current cycle value
		sim->signal.pre_ready_c    	  = *ready_c;
		sim->signal.pre_conflict_c	  = *conflict_c;
		sim->signal.pre_release_c 	  = *release_c;
		sim->signal.pre_releaseid_c	  = *releaseid_c;
	}

}
