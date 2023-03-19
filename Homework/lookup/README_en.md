# lookup

## Problem Description

This problem requires you to implement routing table updates and lookups, including the following four functions.

```cpp
// See code for function comments
void update(bool insert, RoutingTableEntry entry) {
  // TODO
}

// See code for function comments
bool prefix_query(in6_addr addr, in6_addr *nexthop, uint32_t *if_index) {
  // TODO
  *nexthop = 0;
  *if_index = 0;
  return false;
}

// See code for function comments
int mask_to_len(in6_addr mask) {
  // TODO
  return -1;
}

// See code for function comments
in6_addr len_to_mask(int len) {
  // TODO
  return 0;
}
```

where `RoutingTableEntry` is defined in the `lookup.h` file.

You need to implement.

1. insertion and deletion of routing tables
2. lookup of routing table, return whether it is found or not, if it is found you also need to write nexthop and if_index
3. implement `mask_to_len` and `len_to_mask` functions to convert prefix length and address mask

An address mask is a special kind of IPv6 address that is characterized by a series of 1's followed by a series of 0's if viewed from a binary perspective, so the number of 1's is the prefix length. For example, the prefix length 0 corresponds to `::`, 1 corresponds to `8000::`, 4 corresponds to `f000::`, 16 corresponds to `ffff::`, 120 corresponds to `ffff:fff:fff:fff:fff:fff:fff:fff:ff00`, 128 corresponds to `ffff:fff:fff:fff:fff :fff:fff:fff:fff`.

The data used for the evaluation is provided in the `data` directory.

The input data consists of $n$ lines, each line representing an operation. The first character is `I` followed by four space-separated strings corresponding to `addr`, `len`, `if_index` and `nexthop`, indicating insertion; the first character is `D` followed by two space-separated strings corresponding to `addr` and `len`, indicating deletion. If the first character is `Q`, an IPv6 address follows to indicate the IPv6 address to be queried.

Each line of the output data corresponds to each line of the input. For `I` or `D` operations, the code will first determine whether `addr` and `len` are legal in the input information, and output `Invalid` if they are not legal, or `Valid` if they are legal and perform the actual insert or delete operation. For the input `Q` operation, the routing table is queried, and if it is, `nexthop` and `if_index` are printed, otherwise `Not Found` is printed. You don't need to process the input and output in your code.

In terms of the amount of data in this question, implementing a linear lookup table means that you can get all the scores. We believe that an online course is not an algorithm course and should not be overly basic in terms of performance. The idea of implementing a linear lookup table is as follows.

1. maintain an array of routing table entries
2. when inserting/deleting, linearly scan each item in the array and check if it exists.
3. when the longest prefix matches, linearly scan the array, find a number of items that match, and return the result of the one with the longest prefix.

However, there may be performance differences between linear table lookup and better table lookup algorithms in the later stages of the actual router. Other implementations available for reference are as follows.

1. treat the IPv6 address as a string of letters, each of which is 0/1, and implement it with Trie.
2. Do one level of Trie per bit may have more query steps and can be compressed. 3.
3. Lulea route lookup algorithm.

## Sample 4

See *lookup_input4.txt* and *lookup_answer4.txt* in the data directory.

## Explanation of Example 4

You can see that the first two lines of input data construct a routing table like this.

```text
fd00::0102:0300/120 via if9 nexthop fe80::c0a8:0302
fd00::0102:0304/128 via if10 nexthop fe80::c0a8:0901
```

The first three queries matched the second, the first, and could not be found; after deleting the second entry of the routing table, the first two queries matched the first entry, and the third could not be found; after the routing table was deleted, all three could not be found.