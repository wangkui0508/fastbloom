#include "../fastbloom.c"

// gcc -I.. -I./xxHash-0.8.0 -Wall main.c libxxhash.a 

static void check_param(size_t bits_per_entry, size_t probe_per_entry) {
	const int entry_count = 2500000;
	printf("bits_per_entry %zu probe_per_entry %zu", bits_per_entry, probe_per_entry);
	size_t slot_count = (bits_per_entry * entry_count + slot_bit_count - 1) / slot_bit_count;
	struct fastbloom* fb = fastbloom_new(slot_count, probe_per_entry, 0x111111);
	FILE* in_file = fopen("rand.dat", "rb");
	char buf[32];
	for(int i=0; i<entry_count; i++) {
		fread(buf, 32, 1, in_file);
		fastbloom_add(fb, buf, 32);
	}
	size_t positive_count = 0;
	for(int i=0; i<entry_count; i++) {
		fread(buf, 32, 1, in_file);
		if(fastbloom_has(fb, buf, 32)) {
			positive_count++;
		}
	}
	printf(" false positive ratio: %f\n", ((double)positive_count)/((double)entry_count));
	fclose(in_file);
	fastbloom_release(fb);
}

int main(void) {
	for(size_t bits_per_entry = 8; bits_per_entry <= 24; bits_per_entry++) {
		for(size_t probe_count = 3; probe_count <= bits_per_entry; probe_count++) {
			check_param(bits_per_entry, probe_count);
		}
	}
	return 0;
}

