# fastbloom
A fast bloomfilter in C.

Fastbloom boosts bloom filter's speed dramatically, while increasing the DRAM consumption a little.

Fastbloom is actually a hash map to normal bloom filters. These bloom filters are all 512 bits long such that each can fit into a cache line. Given a key, we first look up the hash map and fetch a 512-bit bloom filter, then we looks up the key in it. Each query to fastbloom only need to access one cache line, so its speed is very fast.

However, fastbloom suffers from larger false positive ratio than normal bloom filters, with the same amount of DRAM. To keep the same false positive ratio, fastbloom needs to consume a little more DRAM.

There is also a [golang version of fastbloom](https://github.com/wangkui0508/go-fastbloom).

Here is an [example](https://github.com/wangkui0508/fastbloom/falsepos_demo) demonstrating how to use fastbloom, which is a small program calculating its false negative ratio for different settings.

The following tables compares two implementations of normal bloom filters and two implementations of fastbloom. The bench for comparison can be found in [bloombench](https://github.com/wangkui0508/bloombench). The compared implementations are:

- C++ bloom filter: https://github.com/ArashPartow/bloom
- C fastbloom: https://github.com/wangkui0508/fastbloom
- Go bloom filter: https://github.com/bits-and-blooms/bloom
- Go fastbloom: https://github.com/wangkui0508/go-fastbloom

The DRAM consumption of the implementations (bits per entry):

| False positive ratio | C++ bloom filter | C fastbloom | Go bloom filter | Go fastbloom |
| -------------------- | ---------------- | ----------- | --------------- | ------------ |
| 1%                   | 9.59             | 10          | 9.59            | 10           |
| 0.5%                 | 11.03            | 12          | 11.03           | 12           |
| 0.1%                 | 14.38            | 16          | 14.38           | 16           |
| 0.05%                | 15.82            | 18          | 15.82           | 18           |
| 0.01%                | 19.17            | 23          | 19.17           | 23           |

The time used for inserting 10 million entries and then querying 10 million entries.

| False positive ratio | C++ bloom filter | C fastbloom | Go bloom filter | Go fastbloom |
| -------------------- | ---------------- | ----------- | --------------- | ------------ |
| 1%                   | 16.17            | 7.31        | 13.40           | 9.47         |
| 0.5%                 | 17.16            | 7.31        | 15.13           | 10.01        |
| 0.1%                 | 20.47            | 7.07        | 19.52           | 10.90        |
| 0.05%                | 22.44            | 7.29        | 23.31           | 10.73        |
| 0.01%                | 26.36            | 8.44        | 29.03           | 11.52        |

