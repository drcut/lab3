#include "cache.h"
#include "def.h"
#include <cstring>

bool Cache::miss(uint64_t addr, int &last_visit)
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
			dbg_printf("hit\n");
			return false;
		}
		if(set[set_num].way[i].last_visit_time < set[set_num].way[last_visit].last_visit_time && set[set_num].way[last_visit].valid == 1 && set[set_num].way[i].valid == 1)			
			last_visit = i;
		if(set[set_num].way[i].valid == 0)	//prefer to use the empty block
			last_visit = i;
	}
	dbg_printf("miss\n");
	stats_.miss_num++;
	return true;
}
void Cache::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &time) {
	hit = 0;
	int last_visit;
	stats_.access_counter++;
	
	uint64_t set_num = get_set_num(addr);
	uint64_t tag = get_tag(addr);
	uint64_t offset = get_offset(addr);
	
	int lower_hit;			// TODO: Now the time may not be correctly counted.
	
	dbg_printf("Request type = %c, addr = 0x%016lx, len = %d\n", read?'r':'w', addr, bytes);
	dbg_printf("tag = 0x%lx, set_num = 0x%lx, offset = 0x%lx\n", tag, set_num, offset);
	
	if(miss(addr,last_visit))
	{
		// Evicting old block by LRU, if not (write && write_non_allocate)
		if(set[set_num].way[last_visit].valid && !(!read && !config_.write_allocate))
		{
			stats_.replace_num ++;
			// If have_write, then need to write back
			if(set[set_num].way[last_visit].have_write)
	  		{
	  			lower_->HandleRequest(get_addr_by_cache(set_num, last_visit), config_.block_size, 0, set[set_num].way[last_visit].data,
		                    		lower_hit, time);		// write back
	  			//update time for write back
	  			time += latency_.bus_latency;
	  			//stats_.access_time += latency_.bus_latency;
	  			stats_.fetch_num++;		// TEMP for write back count
			}
		}
		
		// Loading new block
		if(!read)	// write
		{
			if(config_.write_allocate)
			{
				lower_->HandleRequest(addr ^ offset, config_.block_size, 1, set[set_num].way[last_visit].data,
		                    		lower_hit, time);		// write alloc: first read the block
		        time += latency_.bus_latency;
				//stats_.access_time += latency_.bus_latency;
				
				memcpy(set[set_num].way[last_visit].data + offset, content, bytes);	// write to cache block
				set[set_num].way[last_visit].valid = true;
				set[set_num].way[last_visit].have_write = true;
				set[set_num].way[last_visit].tag = tag;
				set[set_num].way[last_visit].last_visit_time = now_time++;
			}
			else	// write non_allocate
			{
				lower_->HandleRequest(addr, bytes, 0, content, lower_hit, time);		// write into lower layer
				time += latency_.bus_latency;
				//stats_.access_time += latency_.bus_latency;
			}
			
    		hit = 0;
		}
		else		// read
		{
			//read the data from lower level and then fill the block
    		lower_->HandleRequest(addr ^ offset, config_.block_size, 1, set[set_num].way[last_visit].data,
                        		lower_hit, time);
            time += latency_.bus_latency;
			//stats_.access_time += latency_.bus_latency;
			
			set[set_num].way[last_visit].valid = true;
			set[set_num].way[last_visit].have_write = false;
			set[set_num].way[last_visit].tag = tag;
			set[set_num].way[last_visit].last_visit_time = now_time++;
			memcpy(content, set[set_num].way[last_visit].data + offset, bytes);	// read from cache block
			hit = 0;
		}
	}
	else	// cache hit
	{
		for(int i = 0; i < config_.associativity; i++)
		{
			if(set[set_num].way[i].tag == tag && set[set_num].way[i].valid)
			{
				if(read)
				{
					memcpy(content, set[set_num].way[i].data + offset, bytes);
					time += latency_.bus_latency + latency_.hit_latency;
					stats_.access_time += latency_.hit_latency;
				}
				else	// write hit: write into cache anyway
				{
					memcpy(set[set_num].way[i].data + offset, content, bytes);
					set[set_num].way[i].last_visit_time = now_time++;
					time += latency_.bus_latency + latency_.hit_latency;
					stats_.access_time += latency_.hit_latency;
					
					if(config_.write_through)
					{
						lower_->HandleRequest(addr, bytes, 0, content, lower_hit, time);
					}
					else	// write back
					{
						set[set_num].way[i].have_write = true;
					}
					
					stats_.prefetch_num++;	// TEMP for write hit count
				}
			}
		}

	  	hit = 1;
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

