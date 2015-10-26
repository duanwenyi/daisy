#include "svdpi.h"
#include <vector>

using namespace std;

typedef struct ENIGMA_FLIT_S {
	int port;  // 0: from port A   1: from port B 
	int id;
	int qos;
	int payload[4];
}ENIGMA_FLIT, *ENIGMA_FLIT_p;

typedef struct ENIGMA_PORT_SIGNAL_S{
	int ready_a; 	  
	int ready_b;	  
	int payload_c_0;
	int payload_c_1;
	int payload_c_2;
	int payload_c_3;
	int id_c;
	int qos_c;
	int valid_c;
    // debug using
	int tick;

	// pre-cycle singal value
	int pre_valid_a;
	int pre_valid_b;

	int pre_ready_c;
	int pre_conflict_c;
	int pre_release_c;
	int pre_releaseid_c;
	
}ENIGMA_PORT_SIGNAL, *ENIGMA_PORT_SIGNAL_p;

enum QOS_LEVEL{
	LEVEL0 = 0,
	LEVEL1 = 1,
	LEVEL2 = 2,
	LEVEL3 = 3
};

enum OUT_VALID{
	INVALID_OUT = 0,
	VALID_OUT   = 1
};

class EnigmaSim
{
 public:
	EnigmaSim();
	~EnigmaSim();
	
	void setRandomSeed(int seed);

	void loadStimulate();

	int                 stim_mode;  // 1: manual   0:auto random
	

	void joinOneFlit(vector<ENIGMA_FLIT> *group, int port, int id, int qos);

	void genOneFlitA(int id, int qos);
	void genOneFlitB(int id, int qos);

	
	vector<ENIGMA_FLIT> portA;
	vector<ENIGMA_FLIT> portB;

	vector<ENIGMA_FLIT> dutActive;
	vector<ENIGMA_FLIT> dutPending;

	vector<ENIGMA_FLIT> portC;

	vector<ENIGMA_FLIT> conflictC;
	vector<ENIGMA_FLIT> releaseC;

	ENIGMA_FLIT         ocell;             // output Flit of C port
	int                 pre_out_vld_mark;  // pre-cycle output valid
	vector<int>         release_delay;     // bind process to dutPending !

	ENIGMA_PORT_SIGNAL  signal;

};

extern "C" {

	void *enigma_init( int TEST_ID);

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
							  );
   
													 

	void enigma_port_A_sim( void *        app,
							svBitVecVal * payload_a_0,
							svBitVecVal * payload_a_1,
							svBitVecVal * payload_a_2,
							svBitVecVal * payload_a_3,
							svBitVecVal * id_a,
							svBitVecVal * qos_a,
							svBit * 	  valid_a
							);

	void enigma_port_B_sim( void *        app,
							svBitVecVal * payload_b_0,
							svBitVecVal * payload_b_1,
							svBitVecVal * payload_b_2,
							svBitVecVal * payload_b_3,
							svBitVecVal * id_b,
							svBitVecVal * qos_b,
							svBit * 	  valid_b
							);

	void enigma_port_C_sim( void *        app,
							svBit * 	  ready_c,

							svBit * 	  conflict_c,
							svBit * 	  release_c,
							svBitVecVal * releaseid_c,
												   
							svBit *		  error
							);
}


typedef struct ENIGMA_BUF_SIGNAL_S{

	// pre-cycle singal value
	int pre_ready_a; 	  
	int pre_ready_b;	  
	int pre_payload_c_0;
	int pre_payload_c_1;
	int pre_payload_c_2;
	int pre_payload_c_3;
	int pre_id_c;
	int pre_qos_c;
	int pre_valid_c;

	// cur-cycle singal value
	int ready_c;
	int conflict_c;
	int release_c;
	int releaseid_c;
	
	// port A
	int payload_a_0;
	int payload_a_1;
	int payload_a_2;
	int payload_a_3;
	int id_a;
	int qos_a;
	int valid_a;

	// port B
	int payload_b_0;
	int payload_b_1;
	int payload_b_2;
	int payload_b_3;
	int id_b;
	int qos_b;
	int valid_b;

    //debug using
    int tick;
}ENIGMA_BUF_SIGNAL, *ENIGMA_BUF_SIGNAL_p;

typedef struct ENIGMA_FLIT_U_S {
    int lock;
	int id;
	int qos;
	int next_ptr;    // 0 ~ 31
}ENIGMA_FLIT_U, *ENIGMA_FLIT_U_p;

#define ENIGMA_MEM_MAX_SIZE 24
#define ENIGMA_MEM_FULL_MARK 0xFFFFFF

typedef struct ENIGMA_BUF_MEM_S {
    int full;          // no empty address now
    int empty;         // no FLIT
    int nptr;          // next empty pointer for new coming FLIT
    int nnptr;         // next next empty pointer for new coming FLIT
    int vld_bits;      // echo bit for one address , map to 0 ~ 31
    int count;
    int lock_count;
	int payload[32][4];    // payload data
}ENIGMA_BUF_MEM, *ENIGMA_BUF_MEM_p;

typedef struct ENIGMA_QOS_S {
    int vld;      // echo bit for one address , map to 0 ~ 31
    int count;
}ENIGMA_QOS, *ENIGMA_QOS_p;

#define DIM_QOS_SEQ_THRESH 20
#define ENIGMA_ID_MSK      0x1F
#define ENIGMA_ID_EXP      0x20

#define PORT_SEL_A 0
#define PORT_SEL_B 1


class EnigmaBuf
{
 public:
	EnigmaBuf();
	~EnigmaBuf();
	
	vector<ENIGMA_FLIT_U> chain;

    int lptr;    // lowest Qos pointer
    int hptr;    // highest Qos pointer
    int head_ptr;  // chain header pointer
    int tail_ptr; // chain tail   pointer
    int update_tail_nptr_en;

    int portSel; // 0:Select A   1:Select B

    void lockAllId(int id);
    void unlockAllId(int id);
    void consumOneCell(int id);
    
    int  getHighestQos();

    void updateMemStatus();

    void showFlitInfo(int chain_id);
    void showFlitAll();

    ENIGMA_BUF_MEM_S    mem;
    
    bool isLowQosNotEmpty(int qos);
    bool isIDUnique(int chain_id);

    int  getlowestQos();
    bool isHighQosNotEmpty(int qos);

    int   qos_count[4];      //  count valid FLIT same qos numbers

	int   pre_chain_id;      // current chain selected cell id
	int   cur_chain_id;      // current chain selected cell id
	int   last_chain_id;     // last chain selected cell id
	int   pre_out_vld_mark;  // pre-cycle output valid

    int   pre_last_qos;
    int   last_qos;
    int   dim_qos_en;        // 
    int   seq_qos_count;     // squence output the same QOS counter

	ENIGMA_BUF_SIGNAL  signal;

};

extern "C" {

	void *enigma_buf_init();

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
							);
   
													 

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
							);
}
