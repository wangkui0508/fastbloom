#include <stdlib.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include "xxhash.h"
#include "fastbloom.h"

struct fastbloom {
	atomic_ullong* slot_ptr;
	size_t         slot_count;
	size_t         probe_per_entry;
	XXH64_hash_t   seed;
};

const size_t slot_byte_count = 64; //Intel CPUs have 64-byte cache lines
const size_t slot_bit_count = slot_byte_count * 8;
const size_t slot_w64_count = slot_byte_count / 8;

struct fastbloom* fastbloom_new(size_t slot_count, size_t probe_per_entry, size_t seed) {
	struct fastbloom* bf = (struct fastbloom*)malloc(sizeof(struct fastbloom));
	bf->slot_ptr = (atomic_ullong*)aligned_alloc(slot_byte_count, slot_count*slot_byte_count);
	bf->slot_count = slot_count;
	memset(bf->slot_ptr, 0, slot_count*slot_byte_count);
	bf->probe_per_entry = probe_per_entry;
	bf->seed = seed;
	fastbloom_reset(bf);
	return bf;
}

void fastbloom_reset(struct fastbloom* bf) {
	memset(bf->slot_ptr, 0, bf->slot_count*slot_byte_count);
}

void fastbloom_release(struct fastbloom* bf) {
	free(bf->slot_ptr);
	free(bf);
}

static bool fastbloom_op(struct fastbloom* bf, const void* data_ptr, size_t data_size, bool is_add) {
	XXH128_hash_t hash = XXH3_128bits_withSeed(data_ptr, data_size, bf->seed);
	size_t bit_offset_in_slot = hash.low64 % slot_bit_count; //low64's lowest 9 bits as first bit_offset
	size_t slot_idx = (hash.low64 / slot_bit_count) % bf->slot_count; //low64's high bits
	atomic_ullong* curr_slot = bf->slot_ptr + (slot_w64_count*slot_idx);
	size_t probe_count = 1;
	while(1) {
		size_t w64_idx = bit_offset_in_slot / 64;
		size_t bit_offset_in_w64 = bit_offset_in_slot % 64;

		unsigned long long mask = 1;
		mask <<= bit_offset_in_w64;
		if(is_add) {
			curr_slot[w64_idx] |= mask;
		} else if((curr_slot[w64_idx]&mask) == 0) {
			return false; // do not has it
		}

		probe_count++;
		if(probe_count == bf->probe_per_entry) {
			break;
		}
		bit_offset_in_slot = hash.high64 % slot_bit_count; //consume bits in high64 as bit_offset
		if(probe_count == 8) { //generate more random bits
			hash = XXH3_128bits_withSeed(data_ptr, data_size, bf->seed + 0x5A5A5A5A);
		} else if(probe_count == 15) { //copy random bits from low64 to high64 for future use
			hash.high64 = hash.low64;
		} else { //shift out the consumed bits
			hash.high64 = hash.high64 / slot_bit_count;
		}
	}
	return true;
}

void fastbloom_add(struct fastbloom* bf, const void* data_ptr, size_t data_size) {
	fastbloom_op(bf, data_ptr, data_size, true);
}

bool fastbloom_has(struct fastbloom* bf, const void* data_ptr, size_t data_size) {
	return fastbloom_op(bf, data_ptr, data_size, false);
}

void fastbloom_get_optimal_params(double target_false_positive_ration, size_t entry_count, size_t* slot_count, size_t* probe_per_entry) {
	size_t bits_per_entry;
	if(target_false_positive_ration > 0.023220) {
		bits_per_entry=8;  *probe_per_entry=6; 
	} else if(target_false_positive_ration > 0.014895) {
		bits_per_entry=9;  *probe_per_entry=7; 
	} else if(target_false_positive_ration > 0.009646) {
		bits_per_entry=10; *probe_per_entry=8; 
	} else if(target_false_positive_ration > 0.006303) {
		bits_per_entry=11; *probe_per_entry=8; 
	} else if(target_false_positive_ration > 0.004118) {
		bits_per_entry=12; *probe_per_entry=9; 
	} else if(target_false_positive_ration > 0.002753) {
		bits_per_entry=13; *probe_per_entry=9; 
	} else if(target_false_positive_ration > 0.001856) {
		bits_per_entry=14; *probe_per_entry=10;
	} else if(target_false_positive_ration > 0.001236) {
		bits_per_entry=15; *probe_per_entry=10;
	} else if(target_false_positive_ration > 0.000841) {
		bits_per_entry=16; *probe_per_entry=10;
	} else if(target_false_positive_ration > 0.000575) {
		bits_per_entry=17; *probe_per_entry=11;
	} else if(target_false_positive_ration > 0.000377) {
		bits_per_entry=18; *probe_per_entry=11;
	} else if(target_false_positive_ration > 0.000271) {
		bits_per_entry=19; *probe_per_entry=12;
	} else if(target_false_positive_ration > 0.000206) {
		bits_per_entry=20; *probe_per_entry=13;
	} else if(target_false_positive_ration > 0.000134) {
		bits_per_entry=21; *probe_per_entry=13;
	} else if(target_false_positive_ration > 0.000108) {
		bits_per_entry=22; *probe_per_entry=13;
	} else if(target_false_positive_ration > 0.000068) {
		bits_per_entry=23; *probe_per_entry=13;
	} else if(target_false_positive_ration > 0.000050) {
		bits_per_entry=24; *probe_per_entry=13;
	} else {
		bits_per_entry=25; *probe_per_entry=14;
	}
	*slot_count = (bits_per_entry * entry_count + slot_bit_count - 1) / slot_bit_count;
}
