#include "stdio.h"
#include "cache.h"
#include "memory.h"
#include "def.h"
#include <stdio.h>
#include <fstream>
#ifndef TYPE_H
#include <inttypes.h>
#define TYPE_H
#endif

//#define F_PATH "1.trace"
int main(int argc, char **argv) {
	char trace_name[4][100] = 
	{
		"../MyTraces/ackerman.txt",
		"../MyTraces/dr2_loop1.txt",
		"../MyTraces/matrix.txt",
		"../MyTraces/quicksort.txt"
	};
	
	char output_log[3][100] = 
	{
		"Never-prefetch.log",
		"Seq-prefetch.log",
		"Always-next.log",
	};
	
	for(int trc = 0; trc <= 3; trc++)
	for(int st = 0; st <= 2; st++)
	{
		Memory m;

		CacheConfig cc1, cc2;
		/*
		printf("Cache_Size(KB):");
		scanf("%d",&cc.size);
		cc.size = (cc.size<<10);
		printf("Block_Size(byte):");
		scanf("%d",&cc.block_size);
		printf("set_associativity(way num):");
		scanf("%d",&cc.associativity);
		*/
		cc2.prefetch_strategy = st;
		cc2.size = 256 << 10;
		cc2.block_size = 64;
		cc2.associativity = 8;
	
		cc2.set_num = cc2.size/(cc2.associativity*cc2.block_size);
		cc2.write_allocate = 1;
		cc2.write_through = 0;
		Cache l2(cc2);
		l2.SetLower(&m);
		
		cc1.prefetch_strategy = st;
		cc1.size = 32 << 10;
		cc1.block_size = 64;
		cc1.associativity = 8;
	
		cc1.set_num = cc1.size/(cc1.associativity*cc1.block_size);
		cc1.write_allocate = 1;
		cc1.write_through = 0;
		Cache l1(cc1);
		l1.SetLower(&l2);

		StorageStats s;
		s.access_counter = 0;
		s.miss_num = 0;
		s.access_time = 0;
		s.replace_num = 0;
		s.fetch_num = 0;
		s.prefetch_num = 0;
		m.SetStats(s);
		l1.SetStats(s);
		l2.SetStats(s);

		StorageLatency m_sl, l1_sl, l2_sl;
		m_sl.bus_latency = 0;
		m_sl.hit_latency = 100;
		m.SetLatency(m_sl);

		l1_sl.bus_latency = 0;
		l1_sl.hit_latency = 4;	//cache size 32K line size 64 associativity 8
		l1.SetLatency(l1_sl);
		
		l2_sl.bus_latency = 6;
		l2_sl.hit_latency = 5;	//cache size 256K line size 64 associativity 8
		l2.SetLatency(l2_sl);

		// freopen(output_log[st], "w", stdout);
		
		printf("Trace file: %s\n", trace_name[trc]);
		printf("Prefetch strategy: %d\n", st);
		
		FILE* fp = NULL;	// Be careful
		/*
		if(strcmp(argv[1],"-1") == 0)
			fp = fopen("1.trace","r");
		else if(strcmp(argv[1],"-2") == 0)
			fp = fopen("2.trace","r");
		*/
		fp = fopen(trace_name[trc], "r");

		int time;
		time = 0;
		char content[64];
		uint64_t addr;
		char action;
		while(fscanf(fp,"%c	%lx\n",&action,&addr) != EOF)
		{
			if(action == 'r')
				l1.HandleRequest((uint64_t)addr, 0, 1, content, time, false);
			else if(action == 'w')
				l1.HandleRequest((uint64_t)addr, 0, 0, content, time, false);
		}
		
		printf("Total time: %d cycles\n", time);
		l1.GetStats(s);
		printf("Total L1 access time: %d cycles\n", s.access_time);
		printf("Total L1 access count: %d\n", s.access_counter);
		printf("Total L1 miss count: %d\n", s.miss_num);
		printf("Total L1 miss rate: %f\n", (float)s.miss_num/(float)s.access_counter);
		// printf("Total L1 replacement count: %d\n", s.replace_num);
		// printf("Total L1 writing back count: %d\n", s.fetch_num);
		// printf("Total L1 writing hit count: %d\n", s.prefetch_num);
		
		l2.GetStats(s);
		printf("Total L2 access time: %d cycles\n", s.access_time);
		printf("Total L2 access count: %d\n", s.access_counter);
		printf("Total L2 miss count: %d\n", s.miss_num);
		printf("Total L2 miss rate: %f\n", (float)s.miss_num/(float)s.access_counter);
		
		m.GetStats(s);
		printf("Total Memory access time: %d cycles\n", s.access_time);
		printf("Total Memory access count: %d\n\n", s.access_counter);
		fclose(fp);
	}
	return 0;
}
