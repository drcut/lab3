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
	Memory m;

	CacheConfig cc;
	/*printf("Cache_Size(KB):");
	scanf("%d",&cc.size);
	cc.size = (cc.size<<10);
	printf("Block_Size(byte):");
	scanf("%d",&cc.block_size);
	printf("set_associativity(way num):");
	scanf("%d",&cc.associativity);*/
	cc.size = 32<<10;
	cc.associativity = 32;
	cc.block_size = 32;
	cc.set_num = cc.size/(cc.associativity*cc.block_size);
	cc.write_allocate = 0;
	Cache l1(cc);
	l1.SetLower(&m);

  StorageStats s;
  s.access_counter = 0;
  s.miss_num = 0;
  s.access_time = 0;
  s.replace_num = 0;
  s.fetch_num =0;
  s.prefetch_num = 0;
  m.SetStats(s);
  l1.SetStats(s);

  StorageLatency ml;
  ml.bus_latency = 6;
  ml.hit_latency = 100;
  m.SetLatency(ml);

  StorageLatency ll;
  ll.bus_latency = 3;
  ll.hit_latency = 10;
  l1.SetLatency(ll);

  FILE*fp=NULL;//需要注意
  if(!strcmp(argv[1],"-1"))
      fp=fopen("1.trace","r");
  else if(!strcmp(argv[1],"-2"))
      fp=fopen("2.trace","r");

  int hit, time;
  char content[64];
  uint64_t addr;
  char action;
  //printf("uint64: %" PRIu64 "\n", num);
  //while(fscanf(fp,"%c	%d \n",&action,&addr)!=EOF)
  while(fscanf(fp,"%c	%" PRIu64 " \n",&action,&addr)!=EOF)
  {
  	  //printf("%c  %" PRIu64 "\n",action,addr);
	  l1.HandleRequest((uint64_t)addr, 0, 1, content, hit, time);
	  //printf("Request access time: %dns\n", time);
  }
l1.GetStats(s);
printf("Total L1 access time: %dns\n", s.access_time);
printf("Total L1 access count: %d\n", s.access_counter);
printf("Total L1 miss count: %d\n", s.miss_num);
printf("Total L1 miss rate: %f\n", (float)s.miss_num/(float)s.access_counter);
printf("Total L1 replacement count: %d\n", s.replace_num);
m.GetStats(s);
printf("Total Memory access time: %dns\n", s.access_time);
printf("Total Memory access count: %d\n", s.access_counter);
  return 0;
}
