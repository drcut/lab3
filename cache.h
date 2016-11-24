#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
#include "storage.h"
#ifndef TYPE_H
#include <inttypes.h>
#define TYPE_H
#endif
typedef struct CacheConfig_ {
  int size;
  int associativity;
  int set_num; // Number of cache sets
  int write_through; // 0|1 for back|through
  int write_allocate; // 0|1 for no-alc|alc
  int block_size;
} CacheConfig;
typedef struct Cacheway_{
	uint64_t tag;
	int last_visit_time;
	bool valid;
	char* data;
	bool have_write;
	Cacheway_()
	{
		valid = 0;
		data = NULL;
		have_write = 0;
	}
} Cacheway;
typedef struct CacheSet_{
	Cacheway way[32];
}CacheSet;
class Cache: public Storage {
 public:
  Cache() 
  {
  	now_time = 0;
  	//set = new CacheSet[config_.set_num];
  	for(int i = 0;i<4096;i++)
  		for(int j = 0;j<32;j++)
  		set[i].way[j].valid = 0;
  	printf("init cache\n");
  }

  // Sets & Gets
  void SetConfig(CacheConfig cc){ config_ = cc; }
  void GetConfig(CacheConfig &cc){ cc = config_; }
  void SetLower(Storage *ll) { lower_ = ll; }
  bool miss(uint64_t addr,int& last_visit);
  // Main access process
  void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &time);
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
  	printf("addr %" PRIu64 " set %" PRIu64 "\n",addr,res);
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
  	printf("addr %" PRIu64 " tag %" PRIu64 "\n",addr,res);
  	return res;
  }
uint64_t get_addr_by_cache(int set_index,int way_index)
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
 private:
  int now_time;
  // Bypassing
  int BypassDecision();
  // Partitioning
  void PartitionAlgorithm();
  // Replacement
  int ReplaceDecision();
  void ReplaceAlgorithm();
  // Prefetching
  int PrefetchDecision();
  void PrefetchAlgorithm();

  CacheSet set[4096];
  CacheConfig config_;
  Storage *lower_;
  DISALLOW_COPY_AND_ASSIGN(Cache);
};

#endif //CACHE_CACHE_H_ 
