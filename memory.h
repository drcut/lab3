#ifndef CACHE_MEMORY_H_
#define CACHE_MEMORY_H_

#include <stdint.h>
#include "storage.h"

#define dbg_printf printf

class Memory: public Storage {
 public:
  Memory() {}
  ~Memory() {}
  void set_mem_data(char* p)
  {
  	mem_data = p;
  }

  // Main access process
  void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &time);

 private:
  // Memory implement
  char* mem_data;
  DISALLOW_COPY_AND_ASSIGN(Memory);
};

#endif //CACHE_MEMORY_H_ 
