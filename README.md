# CS389 HW5
author: Zhengyao Gu, Albert Ji

## Workload Generation
The workload generator has three functions in its public interface. `std::string request_type_dist()` returns 
a type of request randomly. In our workload, there are only three types of requests: `get`, `set` and `del`.
We strictly set the probability of the function returning `get` at 67%. The probability ratio between `set` and `del`
requests is left for users to specify with the constructor parameter `set_del_ratio`.

`key_type random_key()` returns users a key randomly chosen from the key pool `key_pool`. `key_pool` is generated
at the construction of the workload generator with a size `key_num`, specified by users through constructor.
To generate the key pool, the constructor ask for a randomly generated size following the generalized extreme
value distribution with parameters from the memcache paper. The workload generator then generates a string of
the size based on a set pool of around 80 characters. To simulate the temporal locality identified in the
memcache paper, we break the key pool into `n` segments, each given a probablistic weight. Each segment is two times
as likely to be chosen than the next one. After choosing a segment, a key is chosen uniformly randomly
from the segment and returned through `key_type random_key()`.

`const Cache::val_type random_val()` returns a randomly generated array of characters. It is prompted a random size
following the general Pareto distribution with parameters in the memcache table. Then a string of that size
is uniformly generated based on a set pool of characters as in `random_key`. The string is returned as in `char*` type
through the function.

### Calibration
Here is a list of parameters that affects the hit rate during our calibration, and how they affects the hit rate (positively or negatively)
To attain a stable hit rate, we warm up the server with 100 thousand requests during which the hit rate is not recorded.
Then, we compute the hit rate for the next 1 million requests as our metric.
We fixed our maxmem to be 4MB.
 Parameter | Definition | Effect 
--- | --- | ---
 set_del_ratio | How many times a set request is likely to be prompted | positive 
 key_num | Number of keys in the key pool | negative 
 n_seg | number of segments (see last section. We did not make this as a constructor parameter since we decided on the number 100 pretty quickly) | positive
| 
A bigger `set_del_ratio` means less `del` requests, which means less invalidation miss. Thus a bigger ratio increases hit rate.
More keys in the key pool means more keys can be requested by `get`'s, thus more compulsory misses.
Recall that the a segment is always twice as likely to be chosen than the next one. More segments then concentrates a greater
probability in the smaller first segment, thus increasing the temporality overall.

We found that when `set/del = 10`, `key_num = 10,000`, and `n_seg = 100`, we achieve a hit rate around 82% consistently.

### Benchmark
Our performance is very stable when the cache's maxmem is extremely high. More than 95% requests always take less than 2 milliseconds, and most of time, 1 millisecond. Nevertheless, we set the request numbers to 10^5. This is the `cumulative distribution function` of our result. \
![My Graph](https://github.com/zhengyaogu/cs389hw5/blob/master/cdf.png)\
Most requests take less than 1 millisecond, where the requests that take more than 8 milliseconds should be the ones that change the size of our hash table(unordered set) in the cache. \
We ran `benchmark_performance` with the same parameters, and the result `95th-percentile latency` is 0, which means more than 95% requests take less than 1 millisecond to process. The `mean throughput` is 24950.1, so our program can process about 25k requests per second.
|maxmem | set_del_ratio | Compilation Option | key_pool_size| 95th_latency | Mean Throughput |
| --- | --- | --- | --- | --- | --- |
|1000000 | 10| -O3 | 10000 | 0 | 1e+8|

### Sensitivity Testing
The first thing that we need to test is the size of maximum cache memory, i.e. the `maxmem` variable of our cache. The guess is we kept get those 0 and 1 millisecond requests because our cache is so large that it doesn't need to evict anything.  \
The second aspect we want to alter is the `set_del_ratio`. We want to see which request takes longer time to process with different mamximum cache memory. \
The third aspect we want to test is the compilation option. We used command `-O3` in all of the previous sections, and we want to see what would happen if we change it. \
The last aspect we want to test is the  `key_pool_size`. This variable directly controls the number of all possible keys we might generate in the benchmark. The guess is increasing  `key_pool_size` should have a similar effect to decreasing `maxmem`, because they both make the cache evict more keys. 
|maxmem | set_del_ratio | Compilation Option | key_pool_size| 95th_latency | Mean Throughput |
| --- | --- | --- | --- | --- | --- |
|10000 | 10| -O3 | 10000 | 0 | 42973.8|

By changing the `maxmem` from 10^6 to 10^4, though the `95th_latency` doesn't change, the `mean throughput` decreases dramatically. This confirms our guess that smaller `maxmem` will cause the cache to evict more things and increase latency. 
|maxmem | set_del_ratio | Compilation Option | key_pool_size| 95th_latency | Mean Throughput |
| --- | --- | --- | --- | --- | --- |
|10000 | 1000| -O3 | 10000 | 0 | 109649 |

After changing the `set_del_ratio` from 10 to 1000, the `mean throughput` increases nearly 3 times. This original guess is when `maxmem` is limited, eviction is extremely expensive. A higher `set_del_ratio` should force the cache to evict many more times and result in a lower `mean throughput`. However, the data shows the exact opposite. When `maxmem` is limited, the `del` operation is actually more expensive than the `set` operation.

|maxmem | set_del_ratio | Compilation Option | key_pool_size| 95th_latency | Mean Throughput |
| --- | --- | --- | --- | --- | --- |
|10000 | 1000 | -O0 | 10000 | 0 | 1.11e+7 |

Surprisingly, when we turn off the optimization flag for the server and benchmark, our program runs faster. This is probably because most requests take 0 millisecond (less than 1 millisecond), and the `-O0` flag somehow reduces the number of extreme values (requests that take more than 10 milliseconds).

|maxmem | set_del_ratio | Compilation Option | key_pool_size| 95th_latency | Mean Throughput |
| --- | --- | --- | --- | --- | --- |
|10000 | 10 | -O0 | 10000 | 0 | 5e+7 |

It seems the pattern of `set_del_ratio` gets reversed when `-O3` gets changed to `-O0`. When the compilation flag is `-O3`, the mean throughput increases as  `set_del_ratio` increases; when the compilation flag is `-O0`, the mean throughput decreases as  `set_del_ratio` increases. I can only guess that this is caused by the optimization of the implementation of `std::unordered_set`.

|maxmem | set_del_ratio | Compilation Option | key_pool_size| 95th_latency | Mean Throughput |
| --- | --- | --- | --- | --- | --- |
|10000 | 10 | -O0 | 100000 | 0 | 5e+7 |

When the compilation flag is `-O0`, the increase of `key_pool_size` doesn't seem to affect the mean throughput. Keep increasing the 
`key_pool_size`.

|maxmem | set_del_ratio | Compilation Option | key_pool_size| 95th_latency | Mean Throughput |
| --- | --- | --- | --- | --- | --- |
|10000 | 10 | -O0 | 1000000 | 0 | 1e+8 |

When the compilation flag is `-O0`, the increase of `key_pool_size` would increase the mean throughput. 

|maxmem | set_del_ratio | Compilation Option | key_pool_size| 95th_latency | Mean Throughput |
| --- | --- | --- | --- | --- | --- |
|10000 | 10 | -O3 | 100000 | 0 | 36941.3 |

When the compilation flag is `-O3`, the increase of `key_pool_size` would decrease the mean throughput. \

The optimization flag greatly affects the performance. For reason unknow, it would reverse the effect of `key_pool_size` and `set_del_ratio` toward the program. 
