# lab3
part 1:single cache layer
Use LRU in cache.cc miss(addr),miss function decide if the addr is in cache or not,if not,use LRU to replace
Cache::HandleRequest first call miss to decide miss or not(in my code,HandleRequest only have to deal with the time,
it does not need to active with cache,for miss() has do it instead 
