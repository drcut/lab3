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
	
	for(int trc = 0; trc <= 3; trc++)
	for(int st = 0; st <= 2; st++)
	{
		Memory m;

		CacheConfig cc;
		/*
		printf("Cache_Size(KB):");
		scanf("%d",&cc.size);
		cc.size = (cc.size<<10);
		printf("Block_Size(byte):");
		scanf("%d",&cc.block_size);
		printf("set_associativity(way num):");
		scanf("%d",&cc.associativity);
		*/
		
		cc.prefetch_strategy = st;
		cc.size = 32 << 10;
		cc.block_size = 64;
		cc.associativity = 8;
	
		cc.set_num = cc.size/(cc.associativity*cc.block_size);
		cc.write_allocate = 1;
		cc.write_through = 0;
		Cache l1(cc);
		l1.SetLower(&m);
	

		  StorageStats s;
		  s.access_counter = 0;
		  s.miss_num = 0;
		  s.access_time = 0;
		  s.replace_num = 0;
		  s.fetch_num = 0;
		  s.prefetch_num = 0;
		  m.SetStats(s);
		  l1.SetStats(s);

		  StorageLatency ml;
		  ml.bus_latency = 0;//6;
		  ml.hit_latency = 100;
		  m.SetLatency(ml);

		  StorageLatency ll;
		  ll.bus_latency =  0;//3;
		  ll.hit_latency = 4;//cache size 32768 line size 64 associativity 8
		  l1.SetLatency(ll);

/*
#ifdef PROG_SIM
  StorageLatency l2l;
  l2l.bus_latency =  0;
  l2l.hit_latency = 5;//cache size 262144 line size 64 associativity 8
  l2.SetLatency(l2l);
	
  StorageLatency llcl;
  llcl.bus_latency =  0;
  llcl.hit_latency = 11;//cache size 8388608 line size 64 associativity 8
  l1c.SetLatency(llcl);
#endif
*/

		l1.SetConfig(cc);
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
		while(fscanf(fp,"%c	%lx\n",&action,&addr)!=EOF)
		{
			if(action == 'r')
				l1.HandleRequest((uint64_t)addr, 0, 1, content, time);
			else if(action == 'w')
				l1.HandleRequest((uint64_t)addr, 0, 0, content, time);
		}
		printf("Trace file: %s\n", trace_name[trc]);
		printf("Prefetch strategy: %d\n", st);
		printf("Total time: %d cycles\n", time);
		l1.GetStats(s);
		printf("Total L1 access time: %d cycles\n", s.access_time);
		printf("Total L1 access count: %d\n", s.access_counter);
		printf("Total L1 miss count: %d\n", s.miss_num);
		printf("Total L1 miss rate: %f\n", (float)s.miss_num/(float)s.access_counter);
		printf("Total L1 replacement count: %d\n", s.replace_num);
		// printf("Total L1 writing back count: %d\n", s.fetch_num);
		// printf("Total L1 writing hit count: %d\n", s.prefetch_num);
		m.GetStats(s);
		printf("Total Memory access time: %d cycles\n", s.access_time);
		printf("Total Memory access count: %d\n\n", s.access_counter);
		fclose(fp);
	}
	return 0;
}
