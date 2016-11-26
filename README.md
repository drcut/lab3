# lab3
part 1: single cache layer
Use LRU in cache.cc miss(addr). Miss function decides if the addr is in cache or not; if not, use LRU to replace.

## Usage:
Just use 'make' to build up the project. You will get 'sim' and 'prog_sim'.

'sim' is the simulator for traces. Use './sim -x' to run on trace_x.
e.g. ./sim -1	--> run trace1

'prog_sim' is the simulator for riscv-programs. Use './prog_sim [exec_file_name]' to run [exec_file_name].
e.g. ./prog_sim dry2	--> run dhrystone
