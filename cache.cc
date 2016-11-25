#include "cache.h"
#include "def.h"
#include <cstring>
bool Cache::miss(uint64_t addr,int &last_visit)
{
	stats_.access_counter++;
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
	stats_.miss_num++;
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
		if(!read)//write
		{
			int lower_hit, lower_time;
    			lower_->HandleRequest(addr, bytes, read, content,
                        		lower_hit, lower_time);//write alloc
			set[set_num].way[last_visit].data = new char[bytes];
			memcpy(set[set_num].way[last_visit].data,content,bytes);
    			hit = 0;
    			time += latency_.bus_latency + lower_time;
    			stats_.access_time += latency_.bus_latency;
		}
		else//read
		{
			//read the data from lower level and then fill the block
			int lower_hit, lower_time;
    			lower_->HandleRequest(addr, bytes, read, content,
                        		lower_hit, lower_time);
    			set[set_num].way[last_visit].data = new char[bytes];
			memcpy(set[set_num].way[last_visit].data,content,bytes);//update the cache data
			hit = 0;
			time += latency_.bus_latency + lower_time;
    			stats_.access_time += latency_.bus_latency;
		}
	}
  	//else replace by LRU
  	else
  	{
  		//if old block's have_write == 1,it shold be write to mem before it is replaced
  		if(set[set_num].way[last_visit].have_write)
  		{
  			int lower_hit, lower_time;
  			lower_->HandleRequest(get_addr_by_cache(set_num, last_visit), bytes, 0 , set[set_num].way[last_visit].data,
                        		lower_hit, lower_time);//write back
  			//update time for write back
  			time += latency_.bus_latency + lower_time;
    			stats_.access_time += latency_.bus_latency;

    			lower_->HandleRequest(addr, bytes, read, content,lower_hit, lower_time);
			memcpy(set[set_num].way[last_visit].data,content,bytes);//update the cache data
			//update time(should add bus_latency twice?)
			hit = 0;
			time += lower_time;
    			//update block
  			set[set_num].way[last_visit].valid = 1;
			set[set_num].way[last_visit].tag = tag;
			set[set_num].way[last_visit].last_visit_time = now_time++;
			set[set_num].way[last_visit].have_write = 0;
  		}
  		else
  		{
  			set[set_num].way[last_visit].valid = 1;
			set[set_num].way[last_visit].tag = tag;
			set[set_num].way[last_visit].last_visit_time = now_time++;
			set[set_num].way[last_visit].have_write = 0;
  			int lower_hit, lower_time;
    			lower_->HandleRequest(addr, bytes, read, content,
                        		lower_hit, lower_time);//read data
			memcpy(set[set_num].way[last_visit].data,content,bytes);//update the cache data
			hit = 0;
			time += latency_.bus_latency + lower_time;
    			stats_.access_time += latency_.bus_latency;
  		}
  	}
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

