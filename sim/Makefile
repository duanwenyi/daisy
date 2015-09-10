EVA_PRO_PATH = $(PWD)/..
EVA_PATH     = $(EVA_PRO_PATH)/shm/include
EVA_SRC      = $(wildcard $(EVA_PRO_PATH)/shm/dpi/*.c) $(EVA_PATH)/eva.c
#EVA_OBJ      = $(EVA_SRC:.c=.o) $(EVA_PRO_PATH)/shm/dpi/eva_hdl_drv.c
EVA_OBJ      = $(patsubst %.c,%.o,$(EVA_SRC))

CC          = gcc
CXX         = g++
DEFINES     = 
CFLAGS      = -pipe -O2 -Wall -W -m32 $(DEFINES) -lpthread
CXXFLAGS    = -pipe -O2 -Wall -W -m32 $(DEFINES) -lpthread
INCPATH     = -I$(EVA_PATH) -I$(EVA_PRO_PATH)/shm/dpi -I$(IES_HOME)/tools/include
#-I$(VCS_HOME)/include
LINK        = gcc
LFLAGS      = -Wl -lpthread
LIBS        = 

# HDL comple options when using VCS
VCS_SIM_EXEC   = ./simv_exec
VCS_CMPL_OPTS  = -sverilog +v2k +libext+.v+.sv +verilog2001ext+.v +vcs+lic+wait +plusarg_save +notimingcheck +nospecify -timescale=1ns/10ps -debug
VCS_CMPL_OPTS += +incdir+$(VCS_HOME)/etc/vmm +vpi
VCS_CMPL_OPTS += -Mdir=./output/vmm_test_scrc -o $(VCS_SIM_EXEC) -l ./output/vmm_test.cmp_log
VCS_CMPL_OPTS += +vpdfile+simv.vpd

VCS_RUN_OPTS   = -l ./simv.log -ucli -i vcs_sim.tcl



## HDL compile options
TOP_MODULE    = TH
DUT_CMPL_OPTS = -sv -update
DUT_ELAB_OPTS = -delay_mode zero -notimingchecks -noneg_tchk -access +rwc -timescale 1ns/10ps ${TOP_MODULE}
DUT_RUN_OPTS  = ${TOP_MODULE} -nontcglitch -errormax 1 -input wave.tcl

HDL_COMPILER = ncvlog


# VCS
#USER_RUN_OPTS += -ucli -i ./wave_fsdb.do

eva:
	make -f ../app/Makefile

dpi:
	$(CC) -Wall -g -fPIC -m32 $(EVA_SRC) $(INCPATH) -shared -o libdpi.so

svtb:
	$(CC) -Wall -g -fPIC -m32 $(EVA_SRC) $(INCPATH) -shared -o libdpi.so
	ncvlog -f project.f $(DUT_CMPL_OPTS)
	ncelab $(DUT_ELAB_OPTS)
run:
	ncsim $(DUT_RUN_OPTS)


sstb:
	$(CC) -Wall -g -fPIC -m32 $(EVA_SRC) $(INCPATH) -DUSING_VCS_COMPILER -shared -o libdpi.so
	@mkdir -p output
	vcs -f project.f libdpi.so $(VCS_CMPL_OPTS)

srun:
	$(VCS_SIM_EXEC) $(VCS_RUN_OPTS)


ipcs:
	ipcs -m |grep $$USER |awk '{print $$2}' | xargs ipcrm -m

clean:
	rm *.fsdb INCA_libs libdpi.so *.log daisy_demo *.shm *.key output simv_exec* ncsim.err -rf