#include "cache.h"
#include "def.h"
#include <cstring>
bool Cache::miss(uint64_t addr,int &last_visit)
{
	uint64_t set_num= get_set_num(addr);
	uint64_t tag = get_tag(addr);
	bool have = 0;
	last_visit = 0;
	for(int i = 0;i<config_.associativity;i++)
	{
		if(set[set_num].way[i].tag == tag && set[set_num].way[i].valid == 1)
		{
			set[set_num].way[i].last_visit_time = now_time++;
			printf("not miss\n");
			return 0;
		}
		if(set[set_num].way[i].last_visit_time < set[set_num].way[last_visit].last_visit_time && set[set_num].way[last_visit].valid == 1 && set[set_num].way[last_visit].valid == 1)
			last_visit = i;
		if(set[set_num].way[i].valid == 0)//prefer to use the empty block
			last_visit = i;
	}
	printf("miss\n");
	return 1;
}
void Cache::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &time) {
  hit = 0;
  time = 0;
  int last_visit;
  if(miss(addr,last_visit))
  {
  	uint64_t set_num= get_set_num(addr);
	uint64_t tag = get_tag(addr);
  	//if still have empty block
  	if(set[set_num].way[last_visit].valid == 0)//fill an empty block
	{
		set[set_num].way[last_visit].valid = 1;
		set[set_num].way[last_visit].tag = tag;
		set[set_num].way[last_visit].last_visit_time = now_time++;
		set[set_num].way[last_visit].have_write = 0;
		if(!read)
		{
			set[set_num].way[last_visit].data = new char[bytes];
			memcpy(set[set_num].way[last_visit].data,content,bytes);
		}
		else
		{
			//read the data from lower level and then fill the block
		}
	}
  	//else replace by LRU
  	else
  	{
  		//if old block's have_write == 1,it shold be write to mem before it is replaced
  	}
  	int lower_hit, lower_time;
    	lower_->HandleRequest(addr, bytes, read, content,
                          lower_hit, lower_time);
   	 hit = 0;
    	time += latency_.bus_latency + lower_time;
    	stats_.access_time += latency_.bus_latency;
  }
  else
  {
  	hit = 1;
     	time += latency_.bus_latency + latency_.hit_latency;
      	stats_.access_time += time;
      	return;
  }
}

int Cache::BypassDecision() {
  return FALSE;
}

void Cache::PartitionAlgorithm() {
}

int Cache::ReplaceDecision() {
  return TRUE;
  //return FALSE;
}

void Cache::ReplaceAlgorithm(){
}

int Cache::PrefetchDecision() {
  return FALSE;
}

void Cache::PrefetchAlgorithm() {
}

