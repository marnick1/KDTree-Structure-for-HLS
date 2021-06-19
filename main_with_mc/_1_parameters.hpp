#ifndef MY_HARDWARE_PARAMETERS
#define MY_HARDWARE_PARAMETERS

// Node & KD-Tree Dimensions
static const int Dims = 2;

// Space Partitioning Hardware Specifications
// 1. Number of units per operation
static const int reset_units  = 1;    // Always 1 unit, can't reset twice
static const int insert_units = 1;
static const int remove_units = 1;
static const int search_units = 1;
static const int range_units  = 1;
static const int nn_units     = 5;    // Parallel nearest neighbor queries

static const int stack_units = insert_units + range_units + nn_units;
static const int total_units = stack_units + reset_units + remove_units + search_units;

// 2. Completion Buffer Size
static const int cbuf_size = 20;

// 3. Memory Specifications
static const int memPorts = 10;

#endif
