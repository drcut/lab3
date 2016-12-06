#include "cache.h"
#include "def.h"
#include <cstring>

bool Cache::miss(uint64_t addr)
{
	uint64_t set_num = get_set_num(addr);
	uint64_t tag = get_tag(addr);

	for(int i = 0;i<config_.associativity;i++)
	{
		if(set[set_num].way[i].tag == tag && set[set_num].way[i].valid)
		{
			set[set_num].way[i].last_visit_time = now_time++;
			dbg_printf("hit\n");
			return false;
		}
	}
	dbg_printf("miss\n");
	return true;
}
void Cache::HandleRequest(uint64_t addr, int bytes, int read, char *content, int &time, bool prefetch)
{
	int last_visit;
	
	uint64_t set_num = get_set_num(addr);
	uint64_t tag = get_tag(addr);
	uint64_t offset = get_offset(addr);
		
	dbg_printf("Request type = %c, addr = 0x%016lx, len = %d\n", read?'r':'w', addr, bytes);
	dbg_printf("tag = 0x%lx, set_num = 0x%lx, offset = 0x%lx\n", tag, set_num, offset);
	
	if(!prefetch)		// Prefetching does not need time
	{
		stats_.access_counter++;
		time += latency_.hit_latency + latency_.bus_latency;
		stats_.access_time += latency_.hit_latency + latency_.bus_latency;
	}
	
	if(miss(addr))
	{
		if(!prefetch)
			stats_.miss_num++;
		
		if(BypassDecision(addr))
		{
			lower_->HandleRequest(addr, bytes, 0, content, time, prefetch);
			// time += latency_.bus_latency;
		}
		else
		{
			int last_visit = ReplaceDecision(get_set_num(addr));
			
			// Evicting old block, if not (write && write_non_allocate)
			if(set[set_num].way[last_visit].valid && !(!read && !config_.write_allocate))
			{
				stats_.replace_num ++;
				// If have_write, then need to write back
				if(set[set_num].way[last_visit].have_write)
		  		{
		  			uint64_t wb_addr = get_addr_by_cache(set_num, last_visit);
		  			lower_->HandleRequest(wb_addr, config_.block_size, 0, set[set_num].way[last_visit].data, time, prefetch);		// write back
		  			//update time for write back
		  			// time += latency_.bus_latency;
				}
			}
		
			// Loading new block
			if(!read)	// write
			{
				if(config_.write_allocate)
				{
					Load_block(addr ^ offset, now_time, set_num, last_visit, time, prefetch);		// write alloc: first read the block
				
					memcpy(set[set_num].way[last_visit].data + offset, content, bytes);	// write to cache block
					set[set_num].way[last_visit].have_write = true;
				}
				else	// write non_allocate
				{
					lower_->HandleRequest(addr, bytes, 0, content, time, prefetch);	// write into lower layer
					// time += latency_.bus_latency;
				}
			}
			else		// read
			{
				//read the data from lower level and then fill the block
				Load_block(addr ^ offset, now_time, set_num, last_visit, time, prefetch);

				memcpy(content, set[set_num].way[last_visit].data + offset, bytes);	// read from cache block
			}
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
				}
				else	// write hit: write into cache anyway
				{
					memcpy(set[set_num].way[i].data + offset, content, bytes);
					set[set_num].way[i].last_visit_time = now_time++;
					
					if(config_.write_through)
					{
						lower_->HandleRequest(addr, bytes, 0, content, time, prefetch);
						// time += latency_.bus_latency;
					}
					else	// write back
					{
						set[set_num].way[i].have_write = true;
					}
					
					//stats_.prefetch_num++;	// TEMP for write hit count
				}
			}
		}
	}
	
	PrefetchAlgorithm(addr, now_time, config_.prefetch_strategy);
}

bool Cache::BypassDecision(uint64_t addr) {
	return false;
}

int Cache::ReplaceDecision(uint64_t set_num) {
	
	int last_visit = 0;
	for(int i = 1; i < config_.associativity; i++)
	{
		if(set[set_num].way[i].last_visit_time < set[set_num].way[last_visit].last_visit_time && set[set_num].way[i].valid)			
			last_visit = i;
		if(set[set_num].way[i].valid == 0)	//prefer to use the empty block
		{
			last_visit = i;
			break;
		}
	}
	return last_visit;
}

void Cache::PrefetchAlgorithm(uint64_t addr, int now_time, int strategy)
{
	int time = -1;		// Temp
	if(strategy == 1)			// Algebra sequence detecting
	{
		uint64_t pref_addr[4];
		uint64_t read_addr[5] = {addr ^ get_offset(addr)};
		int pref_cnt;
		pattern.reg_and_prefetch(addr, pref_addr, pref_cnt);
		for(int i = 0; i < pref_cnt; i++)
		{
			uint64_t s = get_set_num(pref_addr[i]);
			uint64_t t = get_tag(pref_addr[i]);
			uint64_t off = get_offset(pref_addr[i]);
			read_addr[i+1] = pref_addr[i] ^ off;
			if(read_addr[i+1] == read_addr[i])			// Do not load one block repeatedly
				continue;
		
			// Evicting old block
			int last_visit = ReplaceDecision(s);
			if(set[s].way[last_visit].valid)
			{
				// If have_write, then need to write back
				if(set[s].way[last_visit].have_write)
			 	{
			 		uint64_t wb_addr = get_addr_by_cache(s, last_visit);
			 		lower_->HandleRequest(wb_addr, config_.block_size, 0, set[s].way[last_visit].data, time, true);		// write back
				}
			}
		
			dbg_printf("-- Prefetch %lx\n", read_addr[i+1]);
			Load_block(read_addr[i+1], now_time, s, last_visit, time, true);
		}
	}
	else if(strategy == 2)		// Always next
	{
		uint64_t read_addr = addr ^ get_offset(addr);
		uint64_t s = get_set_num(addr);
		
		// Evicting old block
		int last_visit = ReplaceDecision(s);
		if(set[s].way[last_visit].valid)
		{
			// If have_write, then need to write back
			if(set[s].way[last_visit].have_write)
		 	{
		 		uint64_t wb_addr = get_addr_by_cache(s, last_visit);
		 		lower_->HandleRequest(wb_addr, config_.block_size, 0, set[s].way[last_visit].data, time, true);		// write back
			}
		}
		
		Load_block(read_addr + config_.block_size, now_time, s, last_visit, time, true);
	}
}




