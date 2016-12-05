#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
#include "storage.h"
#ifndef TYPE_H
#include <inttypes.h>
#define TYPE_H
#endif
#include <cstring>

#define dbg_printf

typedef struct CacheConfig_ {
  uint64_t size;
  uint64_t associativity;
  uint64_t set_num; // Number of cache sets
  int write_through; // 0|1 for back|through
  int write_allocate; // 0|1 for no-alc|alc
  int block_size;
} CacheConfig;
typedef struct CacheWay_{
	uint64_t tag;
	int last_visit_time;
	bool valid;
	char* data;
	bool have_write;
	CacheWay_()
	{
		data = NULL;
	}
	void init(CacheConfig cc)
	{
		data = new char[cc.block_size];
		valid = false;
		have_write = false;
	}
} CacheWay;
typedef struct CacheSet_{
	CacheWay* way;
	CacheSet_()
	{
		way = NULL;
	}
	void init(CacheConfig cc)
	{
		way = new CacheWay[cc.associativity];
		for(int i = 0; i < cc.associativity; i++)
		{
			way[i].init(cc);
		}
	}
} CacheSet;			
			

class Cache: public Storage {
 public:
  Cache(CacheConfig cc) 
  {
  	now_time = 0;
  	set = NULL;
  	lower_ = NULL;
  	
  	SetConfig(cc);
  }
  
  Cache()
  {
  	now_time = 0;
  	set = NULL;
  	lower_ = NULL;
  }

  // Sets & Gets
  void SetConfig(CacheConfig cc)
  {
  	config_ = cc;
  	
  	if(set != NULL)
  		delete(set);
  	set = new CacheSet[cc.set_num];
  	for(int i = 0; i < cc.set_num; i++)
  	{
  		set[i].init(cc);
  	}
  	dbg_printf("Init cache\n");
  }
  
	struct Pattern
	{
		struct
		{
			uint64_t last = 0;
			uint64_t d = 0;
			bool recent = false;
		} seq[2];
		uint64_t hst[16];
		uint64_t hst_p = 0;
	
		// addr: Last visited address
		// pref_addr: the array of addresses to prefetch
		// pref_cnt: how many addresses to prefetch
		void reg_and_prefetch(uint64_t addr, uint64_t* pref_addr, int& pref_cnt)
		{
			if(addr == seq[0].last + seq[0].d)
			{
				seq[0].recent = true;
				seq[1].recent = false;
				pref_cnt = 4;
				for(int i = 0; i < pref_cnt; i++)
					pref_addr[i] = seq[0].last + seq[0].d * (i+1);
				return;
			}
			else if(addr == seq[1].last + seq[1].d)
			{
				seq[1].recent = true;
				seq[0].recent = false;
				pref_cnt = 4;
				for(int i = 0; i < pref_cnt; i++)
					pref_addr[i] = seq[1].last + seq[1].d * (i+1);
				return;
			}
			else
			{
				hst[hst_p] = addr;
				
				int i;
				for(i = (hst_p + 15) % 16; i != hst_p; i = (i + 15) % 16)
				{
					uint64_t c_head = hst[i];
					uint64_t d = addr - hst[i];
					if(d == 0)
						continue;			// Do not consider sequence with d = 0
					
					int id = i;
					int rem = 2;
					while(id != hst_p && rem > 0)
					{
						if(c_head - hst[id] == d)
						{
							rem--;
							c_head = hst[id];
						}
						id = (id + 15) % 16;
					}
					
					if(rem == 0)			// Good sequence found
					{
						int r = seq[0].recent ? 1 : 0;
						seq[r].last = addr;
						seq[r].d = d;
						seq[r].recent = true;
						seq[r^1].recent = false;
						
						pref_cnt = 4;
						for(int i = 0; i < pref_cnt; i++)
							pref_addr[i] = seq[r].last + seq[r].d * (i+1);
						
						break;
					}
				}
				
				if(i == hst_p)				// No sequence found
					pref_cnt = 0;
				
				hst_p = (hst_p + 1) % 16;
				return;
			}
		}
	} pattern;
	
  void GetConfig(CacheConfig &cc){ cc = config_; }
  void SetLower(Storage *ll) { lower_ = ll; }
  bool miss(uint64_t addr);

	// Main access process
	void HandleRequest(uint64_t addr, int bytes, int read, char *content, int &time);
	
	void Load_block(uint64_t addr, int &now_time, uint64_t set_num, uint64_t line_id, int &time)
	{
		CacheWay* cw = &set[set_num].way[line_id];
		lower_->HandleRequest(addr, config_.block_size, 1, cw->data, time);
		cw->valid = true;
		cw->have_write = false;
		cw->tag = get_tag(addr);
		cw->last_visit_time = now_time++;
		
		time += latency_.bus_latency;
	}	

  uint64_t get_set_num(uint64_t addr)
  {
  	int offset = 0;
  	int tmp = config_.block_size;
  	while(tmp!=1)
  	{
  		offset++;
  		tmp>>=1;
  	}
  	uint64_t res = (addr>>offset)&((uint64_t)config_.set_num-1);
  	//printf("addr %" PRIu64 " set %" PRIu64 "\n",addr,res);
  	//getchar();
  	return res;
  }
  
  uint64_t get_tag(uint64_t addr)
  {
  	int offset = 0;
  	int tmp = config_.block_size;
  	while(tmp!=1)
  	{
  		offset++;
  		tmp>>=1;
  	}
  	int offset_set = 0;
  	tmp = config_.set_num;
  	while(tmp!=1)
  	{
  		offset_set++;
  		tmp>>=1;
  	}
  	uint64_t res = (addr>>(offset_set+offset));
  	//printf("addr %" PRIu64 " tag %" PRIu64 "\n",addr,res);
  	return res;
  }
  
  uint64_t get_offset(uint64_t addr)
  {
       return addr & ((uint64_t)config_.block_size - 1);
  	/*int offset = 0;
  	int tmp = config_.block_size;
  	while(tmp!=1)
  	{
  		offset++;
  		tmp>>=1;
  	}
  	
  	uint64_t res = addr & ((1 << offset) - 1);
  	//printf("addr %" PRIu64 " offset %" PRIu64 "\n",addr,res);
  	return res;*/
  }
  
	uint64_t get_addr_by_cache(int set_index, int way_index)
	{
		int offset = 0;//last bit
	  	int tmp = config_.block_size;
	  	while(tmp!=1)
	  	{
	  		offset++;
	  		tmp>>=1;
	  	}
	  	int offset_set = 0;//middle bit
	  	tmp = config_.set_num;
	  	while(tmp!=1)
	  	{
	  		offset_set++;
	  		tmp>>=1;
	  	}
	  	return (set[set_index].way[way_index].tag<<(offset_set+offset))+(set_index<<offset);
	}
	
	void sync_from_mem(char* mem)
	{
		for(int i = 0; i < config_.set_num; i++)
			for(int j = 0; j < config_.associativity; j++)
				if(set[i].way[j].valid)
				{
					uint64_t addr = get_addr_by_cache(i, j);
					memcpy(set[i].way[j].data, mem + addr, config_.block_size);
				}
	}
	
	void sync_to_mem(char* mem)
	{
		for(int i = 0; i < config_.set_num; i++)
			for(int j = 0; j < config_.associativity; j++)
				if(set[i].way[j].valid && set[i].way[j].have_write)
				{
					uint64_t addr = get_addr_by_cache(i, j);
					memcpy(mem + addr, set[i].way[j].data, config_.block_size);
				}
	}
	
 private:
  int now_time;
  // Bypassing
  bool BypassDecision(uint64_t addr);
  // Partitioning
  // void PartitionAlgorithm();
  // Replacement
  int ReplaceDecision(uint64_t set_num);
  // void ReplaceAlgorithm();
  
  // Prefetching
  // int PrefetchDecision();
  void PrefetchAlgorithm(uint64_t addr, int now_time);

  CacheSet* set;
  CacheConfig config_;
  Storage *lower_;
  DISALLOW_COPY_AND_ASSIGN(Cache);
};

#endif //CACHE_CACHE_H_ 
