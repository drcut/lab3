#include "memory.h"
#include <cstdlib>
#include <cstring>

void Memory::HandleRequest(uint64_t addr, int bytes, int read, char *content, int &time, bool prefetch)
{
	if(!prefetch)
	{
		dbg_printf("Main memory visited\n");
		time += latency_.hit_latency + latency_.bus_latency;
		stats_.access_time += latency_.hit_latency + latency_.bus_latency;
		stats_.access_counter++;
	}
  
#ifdef PROG_SIM
	if(read)
		memcpy(content, mem_data + addr, bytes);
	else
		memcpy(mem_data + addr, content, bytes);
	dbg_printf("type = %c, addr = 0x%016lx\n", read?'r':'w', addr);
#endif

}

