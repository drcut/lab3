#include "stdio.h"
#include "cache.h"
#include "memory.h"
#include "def.h"
#include <stdio.h>
#include <fstream>
#include <cstdlib> 
#include <string>
#ifndef TYPE_H
#include <inttypes.h>
#include <cstring>
#define TYPE_H
#endif
//#define F_PATH "1.trace"
#define MAXN 110
unsigned int lat[MAXN];
int var_len;
const int Trace_num = 612200;
void ini_lat_dic()
{
	memset(lat,0,sizeof(lat));
	FILE*fp = NULL;	//需要注意
  	fp = fopen("hit_latency.txt","r");
  	//int t1,t2,t3;
  	char *t1 = new char[20];
  	char *t2 = new char[20];
  	char *t3 = new char[20];
  	var_len = 0;
  	while(fscanf(fp,"%s %s %s \n",t1,t2,t3)!=EOF)
  	{
  		//printf("%s %s %s\n var len = %d\n",t1,t2,t3,var_len);
  		if(t3[1]=='e')
  			t3[0]='0';
  		lat[var_len++]=atoi(t3);
  		//printf("lat:%u\n",lat[var_len-1]);
  	}
	fclose(fp);
	return;
}
int main(int argc, char **argv) {
	Memory m;
	CacheConfig cc;
	StorageLatency ml;
	  ml.bus_latency = 100;//6;
	  ml.hit_latency = 0;
	  m.SetLatency(ml);
	ini_lat_dic();
	//printf("var_len :%d\n",var_len);
	 /* printf("Cache_Size(KB):");
	  scanf("%d",&cc.size);
	  cc.size = (cc.size<<10);
	  printf("Block_Size(byte):");
	  scanf("%d",&cc.block_size);
	  printf("set_associativity(way num):");
	  scanf("%d",&cc.associativity);*/
	 cc.block_size = 64;
	 int small_time = 0x7FFFFFFF;
	 int small_index_1 = 0;
	 int small_index_2 = 0;
	 int small_index_3 = 0;
	for(int i =0;i<var_len-2;i++)
	{
	for(int j = i+1;j<var_len-1;j++)
	{
	for(int k = j+1;k<var_len;k++)
	{
	if(lat[i]==0)continue;
	if(lat[j]==0)continue;
	if(lat[k]==0)continue;
	if((i/8)==(j/8))continue;//two layer cache should not have the same size
	if((k/8)==(j/8))continue;//two layer cache should not have the same size
	if( ((32<<10)<<(k/8))>(128<<20))continue;
	if( ((32<<10)<<(j/8))>(16<<20))continue;
	if( ((32<<10)<<(i/8))>(2<<20))continue;

	cc.size = (32<<10)<<(k/8);
	if(cc.size>(128<<20))continue;
	cc.associativity = 1<<(k%8);
	cc.set_num = cc.size/(cc.associativity*cc.block_size);
	cc.write_allocate = 1;
	cc.write_through = 0;
	Cache l3(cc);
	l3.SetLower(&m);

	cc.size = (32<<10)<<(j/8);
	if(cc.size>(16<<20))continue;
	cc.associativity = 1<<(j%8);
	cc.set_num = cc.size/(cc.associativity*cc.block_size);
	cc.write_allocate = 1;
	cc.write_through = 0;
	Cache l2(cc);
	l2.SetLower(&l3);

	cc.size = (32<<10)<<(i/8);
	if(cc.size>(2<<20))continue;
	cc.associativity = 1<<(i%8);
	cc.set_num = cc.size/(cc.associativity*cc.block_size);
	cc.write_allocate = 1;
	cc.write_through = 0;
	Cache l1(cc);
	l1.SetLower(&l2);

	//printf("finish ini\n");
	//printf("i=%d j=%d\n",i,j);
	printf("i=%d j=%d k=%d\n",i,j,k);
	  
	   StorageLatency ll;
	  ll.bus_latency =  0;//3;
	  ll.hit_latency = lat[i];//cache size 32768 line size 64 associativity 8
	  l1.SetLatency(ll);

	  ll.bus_latency =  3;//3;
	  ll.hit_latency = lat[j];//cache size 32768 line size 64 associativity 8
	  l2.SetLatency(ll);
	  
	  ll.bus_latency =  6;//3;
	  ll.hit_latency = lat[k];//cache size 32768 line size 64 associativity 8
	  l3.SetLatency(ll);

  FILE*fp = NULL;	//需要注意
  fp = fopen("1.trace","r");

  int hit, time = 0;
  char content[64];
  uint64_t addr;
  char action;
  	int trace_count = 0;
  	while(fscanf(fp,"%c	%lx\n",&action,&addr) != EOF)
	{
		if(((Trace_num-trace_count)*lat[i]+time)>=small_time&&small_time!=0x7FFFFFFF)
		{
			time = small_time;
			break;
		}
		if(time>=small_time)break;
		if(action == 'r')
			l1.HandleRequest((uint64_t)addr, 0, 1, content, hit, time);
		else if(action == 'w')
			l1.HandleRequest((uint64_t)addr, 0, 0, content, hit, time);
		trace_count++;
	}
	//printf("Total time: %d cycles\n", time);
	if(time<small_time)
	{
		small_time = time;
		small_index_1 = i;
		small_index_2 = j;
		small_index_3 = k;
	}
	l1.fresh();
	l2.fresh();
	l3.fresh();
	fclose(fp);
}
}
}
	printf("small time = %d\nsmall index 1 = %d small_index_2=%d small_index_3=%d\n",small_time,small_index_1,small_index_2,small_index_3);
	//printf("small time = %d\nsmall index 1 = %d small_index 2 = %d\n",small_time,small_index_1,small_index_2);
  return 0;
}
