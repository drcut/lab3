#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "exec.h"

// ELF format
#define EI_NIDENT   (16)

struct elf32_head
{
    unsigned char e_ident[EI_NIDENT];   // the magic number  
    unsigned short e_type;              // identify the object file type 
    unsigned short e_machine;           // specify the machine architechture 
    unsigned int e_version;             // identify the object file version
    unsigned long e_entry;              // entry point address of the process 
    unsigned long e_phoff;              // start of program header 
    unsigned long e_shoff;              // start of section header 
    unsigned int e_flags;               // flags 
    unsigned short e_ehsize;            // size of the ELF header 
    unsigned short e_phentsize;         // size of program header 
    unsigned short e_phnum;             // number of program header 
    unsigned short e_shentsize;         // size of section header 
    unsigned short e_shnum;             // number of section header 
    unsigned short e_shstrndx;          // section header string table index 
} e;

struct sec_head
{
	unsigned name;
	unsigned type;
	unsigned long flags;
	unsigned long addr;			// base memory address
	unsigned long offset;		// file offset of the section
	unsigned long size;			// size in byte
	unsigned link;
	unsigned info;
	unsigned long align;
	unsigned long entsize;
} s;

struct prog_head
{
	unsigned type;
	unsigned flags;
	unsigned long offset;			// file offset of the segment
	unsigned long virtaddr;			// virtual address to map segment
	unsigned long physaddr;			// (no use)
	unsigned long filesize;			// segment size in file
	unsigned long memsize;			// segment size in memory
	unsigned long align;
} p;

void print_reg()
{
	printf("Registers:\n");
	int i;
	for(i = 0; i < Reg_number; i++)
	{
		printf("Reg %2d: %016llx", i, RegFile[i]);
		if(i % 4 == 3)
			printf("\n");
		else
			printf("\t");
	}
}

void print_mem(Addr addr, int n)	// n = the number of blocks(8 bytes) you want to check
{
	long long i;
	for(i = addr; i < addr + n; i += 8)
	{
		printf("Mem addr 0x%016llx:\t%016llx\n", i, *(unsigned long long*)memptr(i));
	}
}

void print_inst(int n)
{
	printf("Instructions:\n");
	long long i;
	for(i = PC; i < PC + n*4; i += 4)
	{
		printf("0x%08x\n", *(unsigned int*)memptr(i));
	}
}
	
int main(int argc, char **argv)
{
	char *exec_filename = argv[argc - 1];
	int debug = 0;
	if(argc == 3 && strcmp(argv[1], "-d") == 0)		// debug mode
	{
		debug = 1;
	}
	
	FILE *fp = fopen(exec_filename,"rb");
	FILE *fp2 = fopen(exec_filename, "rb");
	fread((void*)&e, sizeof(struct elf32_head), 1, fp);

	// visit & load program header (segments)
	fseek(fp, e.e_phoff, SEEK_SET);
	int i;
	for(i = 0; i < e.e_phnum; i++)
	{
		fread((void*)&p, sizeof(struct prog_head), 1, fp);						// fp: read program header
		fseek(fp2, p.offset, SEEK_SET);											// fp2: read segment content
		void *seg_start = memptr(p.virtaddr);
		fread(seg_start, 1, p.filesize, fp2);									// load segment content into Mem
		memset((void*)((Addr)seg_start + p.filesize), 0, p.memsize - p.filesize);		// clear bss area in Mem
	}
	
	init(e.e_entry);
	int stat = 0;
	int single_step = debug;	// If debug, turn on single step by default
	int run_until = 0;
	Addr until_addr = 0;
	int verbose = debug;
	
	long long instr_count = 0;
	
	while(1)
	{
		int IF = get_instruction();
		if(debug)
		{
			if(verbose || single_step)
			{
				printf("PC = 0x%016llx\n", PC-4);
				printf("IF = 0x%08x\n", IF);
			}
			if(run_until && PC-4 == until_addr)	// Arriving at run_until destination
			{
				run_until = 0;
				single_step = 1;
			}
		}
		instr_count++;
		
		// write to mem before 'read', 'write' or 'time'
		if(opcode(IF) == SYSTEM && (IF >> 20) == 0)
		{
			// only need to sync the top layer: just for faster
			llc.sync_to_mem(mem_data);
			l2.sync_to_mem(mem_data);
			l1.sync_to_mem(mem_data);
		}
		stat = decode_and_run(IF);
		// refresh cache after 'read' or 'time'
		if(opcode(IF) == SYSTEM && (IF >> 20) == 0)
		{
			// only need to sync the top layer: just for faster
			l1.sync_from_mem(mem_data);
			l2.sync_from_mem(mem_data);
			llc.sync_from_mem(mem_data);
		}
		
		if(stat != 0)
			break;
		
		if(single_step)							// In single step mode
		{
			print_reg();
			char in[64];
			while(1)
			{
				scanf("%s", in);
				if(strcmp(in, "r") == 0)		// Check registers
				{
					print_reg();
				}
				else if(strcmp(in, "m") == 0)	// Check memory
				{
					Addr addr;
					int n;
					scanf("%llx%d", &addr, &n);
					print_mem(addr, n);
				}
				else if(strcmp(in, "i") == 0)	// Check instructions
				{
					int n;
					scanf("%d", &n);
					print_inst(n);
				}
				else if(strcmp(in, "g") == 0)	// Run until
				{
					Addr addr;
					scanf("%llx", &addr);
					printf("Running until %016llx\n", addr);
					run_until = 1;
					until_addr = addr;
					single_step = 0;
					break;
				}
				else if(strcmp(in, "v") == 0)	// Verbose on/off
				{
					verbose = !verbose;
					if(verbose)
						printf("Verbose turned on\n");
					else
						printf("Verbose turned off\n");
				}
				else if(strcmp(in, "c") == 0)	// Continue
				{
					break;
				}
				else
				{
					printf("Command not understood\n");
				}
			}
		}
	}
	
	// Test...
	if(debug)
	{
		print_reg();
		printf("PC = %llx\n",PC-4);
	}
	/*
	printf("Instruction count = %lld\n", instr_count);
	printf("Load count = %lld\n", op_count[Load]);
	printf("Store count = %lld\n", op_count[Store]);
	printf("ALUR count = %lld\n", op_count[ALUR]);
	printf("ALUI count = %lld\n", op_count[ALUI]);
	printf("LUI count = %lld\n", op_count[LUI]);
	printf("AUIPC count = %lld\n", op_count[AUIPC]);
	printf("Branch count = %lld\n", op_count[Branch]);
	printf("JAL count = %lld\n", op_count[JAL]);
	printf("JALR count = %lld\n", op_count[JALR]);
	printf("SYSTEM count = %lld\n", op_count[SYSTEM]);
	printf("ALUI_64 count = %lld\n", op_count[ALUI_64]);
	printf("ALUR_64 count = %lld\n", op_count[ALUR_64]);
	printf("FLoad count = %lld\n", op_count[FLoad]);
	printf("FStore count = %lld\n", op_count[FStore]);
	printf("OP_FP count = %lld\n", op_count[OP_FP]);
	printf("FMADD count = %lld\n", op_count[FMADD]);
	printf("FMSUB count = %lld\n", op_count[FMSUB]);
	printf("FNMADD count = %lld\n", op_count[FNMADD]);
	printf("FNMSUB count = %lld\n", op_count[FNMSUB]);
	*/
	// Exit
	printf("Total time: %d cycles\n", time);
	fclose(fp2);
	fclose(fp);
	return 0;
}

