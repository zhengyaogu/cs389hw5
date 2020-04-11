#include <iostream>
#include <cassert>
#include "cache.hh"
#include "workload_generator.hh"

int main()
{
    Cache my_cache("127.0.0.1", "10002");
    WorkloadGenerator wkld(2);
    unsigned int warm_up_iters = 10000;
    unsigned int test_iters = 10000;
    unsigned int count = 0;
    unsigned int hits = 0;

    for (unsigned int i = 0; i < warm_up_iters + test_iters; i++)
    {
        std::string request_type = wkld.request_type_dist();
        if (request_type.compare("get") == 0)
            {
                Cache::size_type s = 0;
                Cache::size_type key_size = wkld.key_size_dist();
                key_type key = wkld.random_key(key_size);
                Cache::val_type val = my_cache.get(key, s);
                if (i >= warm_up_iters)
                {
                    count++;
                    if (val != nullptr) {hits++;}
                }
            }
        else if (request_type.compare("get") == 0)
            {
                Cache::size_type key_size = wkld.key_size_dist();
                key_type key = wkld.random_key(key_size);
                Cache::size_type val_size = wkld.val_size_dist();
                const Cache::val_type val = wkld.random_val(key_size);
                my_cache.set(key, val, val_size + 1);
            }
        else if (request_type.compare("del") == 0)
            {
                Cache::size_type key_size = wkld.key_size_dist();
                key_type key = wkld.random_key(key_size);
                my_cache.del(key);
            }
    }

    std::cout << "The hit rate is: " << double(hits) / double(count) << std::endl;
}