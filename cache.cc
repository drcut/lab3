#include "cache.h"
#include "def.h"
bool Cache::miss(uint64_t addr)
{
	uint64_t set_num= get_set_num(addr);
	uint64_t tag = get_tag(addr);
	bool have = 0;
	int last_visit = 0;
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
		if(set[set_num].way[i].valid == 0)
			last_visit = i;
	}
	//replace the last_visit
	set[set_num].way[last_visit].tag = tag;
	set[set_num].way[last_visit].valid = 1;
	set[set_num].way[last_visit].last_visit_time = now_time++;
	printf("miss\n");
	return 1;
}
void Cache::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &time) {
  hit = 0;
  time = 0;
  if(miss(addr))
  {
  	//already replace by LRU
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

