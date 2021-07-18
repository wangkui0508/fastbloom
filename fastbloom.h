#include <stdint.h>
#include <stdbool.h>

struct fastbloom;

void fastbloom_get_optimal_params(double target_false_positive_ration, size_t entry_count, size_t* slot_count, size_t* probe_per_entry);

struct fastbloom* fastbloom_new(size_t slot_count, size_t probe_per_entry, size_t seed);

void fastbloom_release(struct fastbloom* fb);

void fastbloom_reset(struct fastbloom* fb);

void fastbloom_add(struct fastbloom* fb, const void* data_ptr, size_t data_size);

bool fastbloom_has(struct fastbloom* fb, const void* data_ptr, size_t data_size);
