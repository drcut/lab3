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
#define F_PATH "/home/robin/study/trace/1.trace"
int main(void) {
  Memory m;
  Cache l1;
  l1.SetLower(&m);


CacheConfig cc;
/*printf("Cache_Size(KB):");
scanf("%d",&cc.size);
cc.size = (cc.size<<10);
printf("Block_Size(byte):");
scanf("%d",&cc.block_size);
printf("set_associativity(way num):");
scanf("%d",&cc.associativity);*/
cc.size = 32<<10;
cc.associativity = 8;
cc.block_size = 8;
cc.set_num = cc.size/(cc.associativity*cc.block_size);
l1.SetConfig(cc);

  StorageStats s;
  s.access_time = 0;
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
  fp=fopen(F_PATH,"r");

  int hit, time;
  char content[64];
  uint64_t addr;
  char action;
  //printf("uint64: %" PRIu64 "\n", num);
  //while(fscanf(fp,"%c	%d \n",&action,&addr)!=EOF)
  while(fscanf(fp,"%c	%" PRIu64 " \n",&action,&addr)!=EOF)
  {
  	  //printf("%c  %" PRIu64 "\n",action,addr);
	  l1.HandleRequest((uint64_t)addr, 0, (action=='r'), content, hit, time);
	  //printf("Request access time: %dns\n", time);
  }
l1.GetStats(s);
printf("Total L1 access time: %dns\n", s.access_time);
m.GetStats(s);
printf("Total Memory access time: %dns\n", s.access_time);
  return 0;
}
