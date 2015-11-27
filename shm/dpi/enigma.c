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
	
	stim_mode = 0;
    constraint_mode = 0;

    signal.pre_valid_a = 0;
    signal.pre_valid_a = 0;
    signal.pre_ready_c = 0;

    flit_nums = 0;
    idle_det  = 0;
    
	srand(0);
    
    loadTC();
    loadConstraint();

	//loadStimulate();

	//genOneFlitA( 5, LEVEL2);
	//genOneFlitB( 5, LEVEL3);

	fprintf(stderr," Enigma Simulator initialed !\n");
	
}

void EnigmaSim::setRandomSeed(int seed){
	srand(seed);
}

void EnigmaSim::showOCell(const char *info){
    fprintf(stderr," +OCELL[%s]--------------- <%2x - %d>-<%8x %8x %8x %8x > :: %s @%x\n",
            (ocell.id&0x20) ? "B":"A",
            ocell.id, ocell.qos, 
            ocell.payload[3], 
            ocell.payload[2], 
            ocell.payload[1], 
            ocell.payload[0],
            info,
            signal.tick
            );
}

void EnigmaSim::showFLIT( ENIGMA_FLIT cell ){
    fprintf(stderr," +GOLD [%s]+ID-Qos-Payload [%2x - %d]-[%8x %8x %8x %8x] <- \n",
            (cell.id&0x20) ? "B":"A",
            cell.id, cell.qos, 
            cell.payload[3], 
            cell.payload[2], 
            cell.payload[1], 
            cell.payload[0]
            );
}

bool EnigmaSim::checkOCell(){
    bool check = true;
    
    if(isIDExist(ocell.id)){
        if(isIDPending(ocell.id)){
            check = false;
            showOCell("+ERROR: the ID is locked when sent out");
        }else{

            if( isHighQos(ocell.qos) ){
                int p = getFirstPtr(ocell.id);
                if( (dutActive.at(p).id != ocell.id) ||
                    (dutActive.at(p).qos != ocell.qos) ||
                    (dutActive.at(p).payload[0] != ocell.payload[0]) ||
                    (dutActive.at(p).payload[1] != ocell.payload[1]) ||
                    (dutActive.at(p).payload[2] != ocell.payload[2]) ||
                    (dutActive.at(p).payload[3] != ocell.payload[3]) 
                    ){
                    check = false;
                }
                if(check == false){
                    if(isIDUnique(ocell.id)){
                        showOCell("+ERROR: FLIT INFO not correct when ID is unique");
                    }else{
                        showOCell("+ERROR: FLIT INFO not correct when ID is not unique");
                    }
                    showFLIT( dutActive.at(p) );
                }
                
            }else{
                showOCell("+ERROR: FLIT is not high Qos when invoke");
                check = false;
            }
        }
    }else{
        showOCell("+ERROR: the FLIT is not exist");
        check = false;
    }

    return check;
}

void EnigmaSim::showStatus(){
    fprintf(stderr,"..............................................................\n");
    vector<ENIGMA_FLIT_S>::iterator iter = dutActive.begin();

    if(dutActive.size() > 0){
        fprintf(stderr," |Active::  ");
        while( iter != dutActive.end()){
            fprintf(stderr," (%2x - %d)[%2x]", (*iter).id, (*iter).qos ,(*iter).payload[1]&0xFF);
            iter++;
        }
        fprintf(stderr,"\n");
    }

    iter = dutPending.begin();

    if(dutPending.size() > 0){
        fprintf(stderr," |Pending:: ");
        while( iter != dutPending.end()){
            fprintf(stderr," (%2x - %d)[%2x]", (*iter).id, (*iter).qos ,(*iter).payload[1]&0xFF);
            iter++;
        }
        fprintf(stderr,"\n");
    }
    fprintf(stderr,"..............................................................\n");

}

bool EnigmaSim::isIDActive(int id){
    vector<ENIGMA_FLIT_S>::iterator iter = dutActive.begin();

    while( iter != dutActive.end()){
        if( (*iter).id == id){
            return true;
        }
        iter++;
    }
    return false;
}

bool EnigmaSim::isIDNotFull(int id){
    if( getSeqNums(id) < (ENIGMA_SEQ_ID_MAX -1) ){
        return true;
    }else{
        return false;
    }
}

int EnigmaSim::getSeqNums(int id){
    vector<ENIGMA_FLIT_S>::iterator iter = dutActive.begin();
    int count = 0;

    if(dutActive.size() > 0){
        while( iter != dutActive.end()){
            if( (*iter).id == id){
                count++;
            }
            iter++;
        }
    }

    if(dutActive.size() > 0){
        iter = dutPending.begin();
        while( iter != dutPending.end()){
            if( (*iter).id == id){
                count++;
            }
            iter++;
        }
    }


    return count;
}

bool EnigmaSim::isIDPending(int id){
    vector<ENIGMA_FLIT_S>::iterator iter = dutPending.begin();
    
    while( iter != dutPending.end()){
        if( (*iter).id == id){
            return true;
        }
        iter++;
    }
    
    return false;
}

bool EnigmaSim::isIDExist(int id){
    if( isIDActive(id) ||
        isIDPending(id) )
        return true;
    else
        return false;
}

int  EnigmaSim::getFirstPtr(int id){
    vector<ENIGMA_FLIT_S>::iterator iter = dutActive.begin();
    
    int ptr = 0;

    while( iter != dutActive.end()){
        if( (*iter).id == id){
            break;
        }
        iter++;
        ptr++;
    }

    return ptr;
}

bool EnigmaSim::isIDUnique(int id){
    vector<ENIGMA_FLIT_S>::iterator iter = dutActive.begin();
    
    int seq_count = 0;

    while( iter != dutActive.end()){
        if( (*iter).id == id){
            seq_count++;
        }
        iter++;
    }
    
    if(seq_count == 1)
        return true;
    else
        return false;
}

bool EnigmaSim::isHighQos(int qos){
    vector<ENIGMA_FLIT_S>::iterator iter = dutActive.begin();
    int count = 0;

    while( iter != dutActive.end()){
        if( (*iter).qos > qos ){
            if(getFirstPtr((*iter).id) == count){
                if( (signal.tick - (*iter).tick) > ENIGMA_SCHE_MAX  ){
                    if( (*iter).id == portC.back().id  && 
                        ((signal.tick - portC.back().tick ) < ENIGMA_SCHE_MAX)
                        ){
                        showOCell("+Warning: FLIT Qos may not the highest when a IDs group jump to high Qos");
                    }else{
                        showOCell("+Warning: FLIT Qos would not be the highest when a IDs group jump to high Qos");
                        return false;
                    }
                }
            }
        }
        count++;
        iter++;
    }

    return true;
}

void EnigmaSim::loadTC(){
	std::ifstream fin("tc_list", std::ios::in); 
	char line[1024]={0}; 
    std::string tc;

	if(!fin){
		std::cout << " File: tc_list is not found! exiting simulation ... \n" << std::endl;
        exit(0);
    }else{
		while(fin.getline(line, sizeof(line))) { 
            std::stringstream word(line); 
            word >> tc;

            if(!tc.empty()){
                tc_list.push_back(tc);
                std::cout << " +Load TC: " << tc << std::endl;
            }
        }
        std::cout << " Total " << tc_list.size() << " TC loaded ." << std::endl;
        fin.clear(); 
	}
	fin.close(); 

}

void EnigmaSim::loadConstraint(){
    // clear status first
    error  = 0;
    out_constraint.clear();
    in_constraint.clear();
    conflict_op.clear();

	portA.clear();
	portB.clear();
	dutActive.clear();
	dutPending.clear();
	portC.clear();

    if(tc_list.size() > 0){
        std::ifstream fin(tc_list.at(0).c_str(), std::ios::in); 

        char line[1024]={0}; 
        std::string op  = ""; 
        std::string par0_char = ""; 
        std::string par1_char = ""; 
        std::string par2_char = ""; 
    
        int par0, par1,par2;


        if(!fin){
            std::cout << " +Error Open file : " << tc_list.at(0) << std::endl;
        }else{
            std::cout << " --> loading Constraint from : " << tc_list.at(0) << std::endl;
            constraint_mode = 1;

            while(fin.getline(line, sizeof(line))) 
                { 
                    std::stringstream word(line); 
                    word >> op; 
                    if( op.compare("#") != 0){

                        word >> par0_char; 
                        par0  = std::atoi( par0_char.c_str());

                        word >> par1_char; 
                        par1  = std::atoi( par1_char.c_str());

                        if(op.compare("A") == 0){
                        
                            genOneFlitA( par0, par1);  // (id , qos)

                        }else if(op.compare("AM") == 0){
                            word >> par2_char; 
                            par2  = std::atoi( par2_char.c_str());  // repeat times
                            for(int cc=0; cc < par2; cc++){
                                genOneFlitA( par0, par1);  // (id , qos)
                            }

                        }else if(op.compare("B") == 0){

                            genOneFlitB( par0, par1);  // (id , qos)

                        }else if(op.compare("BM") == 0){
                            word >> par2_char; 
                            par2  = std::atoi( par2_char.c_str());  // repeat times
                            for(int cc=0; cc < par2; cc++){
                                genOneFlitB( par0, par1);  // (id , qos)
                            }

                        }else if(op.compare("IN") == 0){
                            IN_CONSTRAINT in;
                            in.port_A_num = par0;
                            in.port_B_num = par1;
                            in_constraint.push_back( in );


                        }else if(op.compare("CC") == 0){
                            word >> par2_char; 
                            par2  = std::atoi( par2_char.c_str());

                            CONFLICT_OP cfl;
                            cfl.hit  = 0;
                            cfl.id   = par0;
                            cfl.seq  = par1;
                            cfl.nums = par2;

                            conflict_op.push_back( cfl );

                        }else if(op.compare("CS") == 0){
                            OUT_CONSTRAINT out;
                            out.pending_nums  = par0;
                            out.transfer_nums = par1;

                            out_constraint.push_back( out );

                        }

                    }
                } 
            
            fin.clear(); 
        }
        fin.close(); 

        showLoadStatus();

        tc_list.erase( tc_list.begin() );

    }
}

int  EnigmaSim::getSeqNumInOut(int id){
    int nums;
    vector<ENIGMA_FLIT>::iterator iter	= portC.begin();
    for(; iter != portC.end();){
        if( (*iter).id == id ){
            nums++;
        }
        iter++;
    }
    
    return nums;
}


int EnigmaSim::checkConflict(){
    int mark_hit = 0;

    if(conflict_op.size() > 0){
        vector<CONFLICT_OP>::iterator iter	= conflict_op.begin();
        // reduce FLIT nums in all hited cell
        for(; iter != conflict_op.end();){
            if( (*iter).hit ){
                if( (*iter).nums > 0 )
                    (*iter).nums--;

                // mark it need release now !
                if((*iter).nums == 0 )
                    (*iter).hit++;
            }
            iter++;
        }
        
        // check whether thd id & seq is hit
        iter	= conflict_op.begin();
        for(; iter != conflict_op.end();){
            if( (*iter).id == ocell.id && 
                !(*iter).hit  &&
                ((*iter).seq = getSeqNumInOut(ocell.id) )
                ){
                (*iter).hit = 1;
                mark_hit =  1;
                
                fprintf(stderr, " +Conflict -------------> {%2x - %d}-[%8x %8x %8x %8x]  @%x\n",
                        ocell.id, ocell.qos,
                        ocell.payload[3], 
                        ocell.payload[2], 
                        ocell.payload[1], 
                        ocell.payload[0], 
                        signal.tick
                        );
                
                showStatus();
                dutPending.push_back( ocell );

                break;
            }
            iter++;
        }
    }

    return mark_hit;
}

void EnigmaSim::pendingRemoveID(int id){
    if(dutPending.size() > 0){
        vector<ENIGMA_FLIT>::iterator iter	= dutPending.begin();
        for(; iter != dutPending.end();){
            if( (*iter).id == id ){
                dutPending.erase(iter);
                fprintf(stderr, " +Release~~~~~~~~~~~~~~~> {%2x - %d}-[%8x %8x %8x %8x]  @%x\n",
                        (*iter).id, (*iter).qos,
                        (*iter).payload[3], 
                        (*iter).payload[2], 
                        (*iter).payload[1], 
                        (*iter).payload[0],
                        signal.tick
                        );
                showStatus();
                break;
            }
            iter++;
        }
    }
}

int  EnigmaSim::checkRealse(){
    int release_id = -1;
    if(conflict_op.size() > 0){
        vector<CONFLICT_OP>::iterator iter	= conflict_op.begin();
        // reduce FLIT nums in all hited cell
        for(; iter != conflict_op.end();){
            if( (*iter).hit > 1 ){
                release_id = (*iter).id;

                // remove pending item
                pendingRemoveID(release_id);
                // remove current constraint
                conflict_op.erase(iter);
                break;
            }
            iter++;
        }
    }

    return release_id;
}

void EnigmaSim::showLoadStatus(){
    fprintf(stderr,"-----------------------------------------------\n");
    fprintf(stderr," +A\t ID\t Qos\t [%3d]\t | +B\t ID\t Qos\t [%3d]\n", portA.size(), portB.size() );
    int max_in_nums = portA.size();
    if( max_in_nums < portB.size() ){
        max_in_nums = portB.size();
    }

    for(int cc=0; cc< max_in_nums; cc++ ){
        if(cc < portA.size() ){
            fprintf(stderr," +A\t %2x\t %d\t      \t      ", portA.at(cc).id, portA.at(cc).qos );
        }else{
            fprintf(stderr,"   \t   \t  \t      \t      " );

        }

        if(cc < portB.size() ){
            fprintf(stderr," +B\t %2x\t %d", portB.at(cc).id, portB.at(cc).qos );
        }
        
        fprintf(stderr,"\n");
    }

    if(in_constraint.size() > 0){
        fprintf(stderr,"-----------------------------------------------\n");
        fprintf(stderr," IN Port FLIT Constraint :\n");
        for(int cc=0; cc< in_constraint.size(); cc++ ){
            fprintf(stderr," Group Transfer A [%3d] & B [%3d]\n", in_constraint.at(cc).port_A_num, in_constraint.at(cc).port_B_num );
        }
    }

    if(out_constraint.size() > 0){
        fprintf(stderr,"-----------------------------------------------\n");
        fprintf(stderr," C Port FLIT Constraint :\n");
        for(int cc=0; cc< out_constraint.size(); cc++ ){
            fprintf(stderr," Pending[%3d] --> Transfer[%3d]\n", out_constraint.at(cc).pending_nums, out_constraint.at(cc).transfer_nums );
        }
    }

    if(conflict_op.size() > 0){
        fprintf(stderr,"-----------------------------------------------\n");
        fprintf(stderr," C Port Conflict Constraint :\n");
        for(int cc=0; cc< conflict_op.size(); cc++ ){
            fprintf(stderr," Conflict ID[%2x] @seq [%3d] then release it after [%3d] FLITs\n", 
                    conflict_op.at(cc).id, conflict_op.at(cc).seq, conflict_op.at(cc).nums );
        }
    }
    fprintf(stderr,"-----------------------------------------------\n");

}

void EnigmaSim::joinOneFlit(vector<ENIGMA_FLIT> *group, int port, int id, int qos){
	
	ENIGMA_FLIT cell;
	//cell.port = port;  // 0:A   1:B
    if(port) // port B
        cell.id   = (id & 0x1F) | 0x20;
    else
        cell.id   = id  & 0x1F;
        

	cell.qos  = qos & 0x3;
	cell.payload[0] = rand();   // for output check
	cell.payload[1] = (port<<31) | (flit_nums & 0xFF);
	cell.payload[2] = qos;
	cell.payload[3] = id; 

    flit_nums++;

    group->push_back( cell );
	//group->insert( group->begin(), cell );
}

void EnigmaSim::genOneFlitA(int id, int qos){
	joinOneFlit( &portA, 0, id, qos);
}

void EnigmaSim::genOneFlitB(int id, int qos){
	joinOneFlit( &portB, 1, id, qos);
}


void EnigmaSim::reducePortA(){
    //portA.pop_back();
    portA.erase(portA.begin());
    
    if(portA.empty() && (in_constraint.size()>0)){
        in_constraint.clear();
        fprintf(stderr," ---->IN Port A is empty now ! Discard all IN Port Constraint!\n");
        showLoadStatus();
    }
}

void EnigmaSim::reducePortB(){
    //portB.pop_back();
    portB.erase(portB.begin());
    
    if(portB.empty() && (in_constraint.size()>0)){
        in_constraint.clear();
        fprintf(stderr," ---->IN Port B is empty now ! Discard all IN Port Constraint!\n");
        showLoadStatus();
    }
}

void EnigmaSim::increasePortC(){

    if(out_constraint.size() > 0){
        if( (out_constraint.at(0).pending_nums == 0) &&
            (out_constraint.at(0).transfer_nums > 0) )
            out_constraint.at(0).transfer_nums--;
        
        if(out_constraint.at(0).transfer_nums == 0){
            out_constraint.erase( out_constraint.begin() );
            fprintf(stderr," C Port FLIT Constraint finished ONE ! remaim %d @%x\n", out_constraint.size(), signal.tick);
            
        }

        if( portA.empty() && portB.empty() &&
            dutActive.empty() && dutPending.empty()
            ){
            out_constraint.clear();
            fprintf(stderr," ---->TC stimulation is over ! Discard all C Port FLIT Constraint now ! @%x\n", signal.tick );
        }
    }

    if( portA.empty() && portB.empty() &&
        dutActive.empty() && dutPending.empty()
        ){
        if(error == 0){
            fprintf(stderr," ---->TC PASS\n\n");
        }else{
            fprintf(stderr," ---->TC FAIL with %d ERROR\n", error);
        }
        if(tc_list.size() > 0){
            loadConstraint();
        }
    }
    
}

bool EnigmaSim::genReadyPortC(){
    if(out_constraint.size() > 0){
        if ( out_constraint.at(0).pending_nums > (dutActive.size() + dutPending.size() )){
            return false;
        }else{
            out_constraint.at(0).pending_nums = 0;
            return true;
        }
    }else{
        return true;
    }
}

bool EnigmaSim::genValidPortA(int ready){
    if(in_constraint.size() > 0){
        if ( in_constraint.at(0).port_A_num > 0 ){
            if(ready)
                in_constraint.at(0).port_A_num--;

            if( in_constraint.at(0).port_A_num == 0 && 
                in_constraint.at(0).port_B_num == 0){
                in_constraint.erase( in_constraint.begin() );
                fprintf(stderr," ---->IN Port Constraint finished ONE by A!  --> remain %d @%x\n" , in_constraint.size(), signal.tick);
            }

            return true;
        }else{
            return false;
        }
    }else{
        return true;
    }
}

bool EnigmaSim::genValidPortB(int ready){
    if(in_constraint.size() > 0){
        if ( in_constraint.at(0).port_B_num > 0 ){
            if(ready)
                in_constraint.at(0).port_B_num--;

            if( in_constraint.at(0).port_A_num == 0 && 
                in_constraint.at(0).port_B_num == 0){
                in_constraint.erase( in_constraint.begin() );
                fprintf(stderr," ---->IN Port Constraint finished ONE by B!  --> remain %d @%x\n" , in_constraint.size(), signal.tick);
            }

            return true;
        }else{
            return false;
        }
    }else{
        return true;
    }
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
            //cell = sim->portA.back();
            cell = sim->portA.at(0);
            cell.tick = sim->signal.tick;
            
            if( sim->isIDNotFull(cell.id) )
                *valid_a = (rand()%4 != 0) && sim->genValidPortA(sim->signal.ready_a);
            else
                *valid_a = 0;
		
			if(sim->signal.pre_valid_a && sim->signal.ready_a){

                sim->reducePortA();

				sim->dutActive.push_back( cell );  // port A
#ifdef ENIGMA_SIM_DEBUG				
				fprintf(stderr," +IN   [A]:ID-Qos-Payload [%2x - %d]-[%8x %8x %8x %8x]  :: remain %d items @%x\n",
						cell.id, cell.qos, 
						cell.payload[3], 
						cell.payload[2], 
						cell.payload[1], 
						cell.payload[0],
						sim->portA.size(),
                        sim->signal.tick
						);
#endif
                if(sim->portA.size() > 0){
                    //cell = sim->portA.back();
                    cell = sim->portA.at(0);
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
            //cell = sim->portB.back();
            cell = sim->portB.at(0);
            cell.tick = sim->signal.tick;

            if( sim->isIDNotFull(cell.id) )
                *valid_b = (rand()%4 != 0) && sim->genValidPortB(sim->signal.ready_b);
            else
                *valid_b = 0;

			if(sim->signal.pre_valid_b && sim->signal.ready_b){
			
                sim->reducePortB();

				sim->dutActive.push_back( cell );  // port B
#ifdef ENIGMA_SIM_DEBUG				
				fprintf(stderr," +IN   [B]:ID-Qos-Payload [%2x - %d]-[%8x %8x %8x %8x]  :: remain %d items @%x\n",
						cell.id, cell.qos, 
						cell.payload[3], 
						cell.payload[2], 
						cell.payload[1], 
						cell.payload[0],
						sim->portB.size(),
                        sim->signal.tick
						);
#endif
                if(sim->portB.size() > 0){
                    //cell = sim->portB.back();
                    cell = sim->portB.at(0);
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

        *ready_c    = ((rand()%6) != 0)  && sim->genReadyPortC();

		// process Realease
        int rel_id  = sim->checkRealse();
        if( rel_id != -1 ){
            *release_c   = 1;
            *releaseid_c = rel_id;
        }else{
            *release_c   = 0;
            *releaseid_c = rand() & 0x3F;
        }

		// process !Conflict
		if( (sim->pre_out_vld_mark == VALID_OUT) ){

			if(!sim->signal.pre_conflict_c){
				sim->portC.push_back( sim->ocell );

				// remove output Flit from Active group
				vector<ENIGMA_FLIT>::iterator iter	= sim->dutActive.begin();
				for(; iter != sim->dutActive.end();iter++){
					if( (*iter).id   == sim->ocell.id){
                        sim->showOCell("");
						// same ID must be the first !
						sim->dutActive.erase(iter);
						break;
					}
				}

                sim->increasePortC();
			}
		}

        *conflict_c = 0;
		if(sim->signal.pre_ready_c && sim->signal.valid_c){
			sim->ocell.id   = sim->signal.id_c;
			sim->ocell.qos  = sim->signal.qos_c;

			sim->ocell.payload[0] = sim->signal.payload_c_0;
			sim->ocell.payload[1] = sim->signal.payload_c_1;
			sim->ocell.payload[2] = sim->signal.payload_c_2;
			sim->ocell.payload[3] = sim->signal.payload_c_3;
            sim->ocell.tick       = sim->signal.tick;

			sim->pre_out_vld_mark = VALID_OUT;

            // check out 
            if(sim->checkOCell() == false)
                *error = 1;

            // procese Cconflict
            *conflict_c = sim->checkConflict();

            sim->idle_det = 0;

		}else{
			
			sim->pre_out_vld_mark = INVALID_OUT;

            sim->idle_det++;
		}

		// backup current cycle value
		sim->signal.pre_ready_c    	  = *ready_c;
		sim->signal.pre_conflict_c	  = *conflict_c;
		sim->signal.pre_release_c 	  = *release_c;
		sim->signal.pre_releaseid_c	  = *releaseid_c;

        if(*error){
            sim->showStatus();

            sim->error++;
        }

        if(sim->idle_det > 200){
            sim->showStatus();
            sim->idle_det = 0;
        }
	}

}
