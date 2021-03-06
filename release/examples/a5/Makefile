# build software version of testbench (to check the "desired behaviour")
IOLIB_INCLUDE=../../iolib/include
IOLIB_LIB=../../iolib/lib
SOCKETLIB_INCLUDE=../../CtestBench/include
SOCKETLIB_LIB=../../CtestBench/lib
VHDL_LIB=../../vhdl
VHDL_VHPI_LIB=../../CtestBench/vhdl
#LLVM2AAOPTS=
LLVM2AAOPTS=-extract_do_while=true
all: SW HW 

PROGDEFS=-DLOOPPIPELINE
#PROGDEFS=

# compile with SW defined.
# note the use of IOLIB in building the testbench.
SW: prog.c  testbench.c 
	gcc -c -DSW -DRUN -I$(IOLIB_INCLUDE) prog.c
	gcc -c -DSW -DRUN -I$(IOLIB_INCLUDE) testbench.c
	gcc -o testbench_sw prog.o testbench.o -L$(IOLIB_LIB) -lio -lpthread

# five steps from C to vhdl simulator.
HW: c2llvmbc llvmbc2aa aa2vc vc2vhdl vhdlsim

# C to llvm byte-code.. use clang.
c2llvmbc: prog.c testbench.c
	clang -std=gnu89 $(PROGDEFS) -I$(IOLIB_INCLUDE) -emit-llvm -c prog.c
	#llvm-dis prog.o
	opt -O3	--instnamer --loopsimplify prog.o > prog.opt.o
	llvm-dis prog.opt.o

# llvm byte-code to Aa..
llvmbc2aa:  prog.opt.o
	llvm2aa $(LLVM2AAOPTS) --pipedepths=pipedepths prog.opt.o | vcFormat >  prog.opt.o.aa

# Aa to vC
aa2vc: prog.opt.o.aa
	AaOpt prog.opt.o.aa | vcFormat > prog.aa
	Aa2VC -O -C prog.aa | vcFormat > prog.vc

# vC to VHDL
vc2vhdl: prog.vc
	vc2vhdl -D -S 4 -O -I 8 -C -e ahir_system -w -s ghdl -t main_inner -f prog.vc
	vhdlFormat < ahir_system_global_package.unformatted_vhdl > ahir_system_global_package.vhdl
	vhdlFormat < ahir_system.unformatted_vhdl > ahir_system.vhdl
	vhdlFormat < ahir_system_test_bench.unformatted_vhdl > ahir_system_test_bench.vhdl

# build testbench and ghdl executable
# note the use of SOCKETLIB in building the testbench.
vhdlsim: ahir_system.vhdl ahir_system_test_bench.vhdl testbench.c vhdlCStubs.h vhdlCStubs.c
	gcc -c vhdlCStubs.c -I./ -I$(SOCKETLIB_INCLUDE)
	gcc -c testbench.c -I./ -I$(SOCKETLIB_INCLUDE)
	gcc -o testbench_hw testbench.o vhdlCStubs.o  -L$(SOCKETLIB_LIB) -lSocketLib -lpthread
	ghdl --clean
	ghdl --remove
	ghdl -i --work=GhdlLink  $(VHDL_LIB)/GhdlLink.vhdl
	ghdl -i --work=ahir  $(VHDL_LIB)/ahir.vhdl
	ghdl -i --work=ahir_ieee_proposed  $(VHDL_LIB)/aHiR_ieee_proposed.vhdl
	ghdl -i --work=work ahir_system_global_package.vhdl
	ghdl -i --work=work ahir_system.vhdl
	ghdl -i --work=work ahir_system_test_bench.vhdl
	ghdl -m --work=work -Wl,-L$(SOCKETLIB_LIB) -Wl,-lVhpi ahir_system_test_bench 

clean:
	rm -f *.o* *.cf *.*vhdl vhdlCStubs.* *.vcd in_data* out_port* testbench_sw testbench_hw ahir_system_test_bench vhpi.log

PHONY: all clean	
