#ifndef BIO_INTERPOLATION_MULTI_IN_PIC_H
#define BIO_INTERPOLATION_MULTI_IN_PIC_H

#include <Arduino.h>

#define SRC_W_PIC 32
#define SRC_H_PIC 32
#define DST_W_PIC 96  // Adjusted to 96x96 grid
#define DST_H_PIC 96  // Adjusted to 96x96 grid

// Define interpolation methods
enum InterpolationMethod {
    NEAREST_NEIGHBOR = 0,  // Nearest Neighbor
    BILINEAR_1BIT,         // 1-bit precision
    BILINEAR_2BIT,         // 2-bit precision
    BILINEAR_4BIT          // 4-bit precision
};

// Shared tables for all interpolation methods
static uint8_t common_x0[DST_W_PIC];
static uint8_t common_x1[DST_W_PIC];
static uint8_t common_y0[DST_H_PIC];
static uint8_t common_y1[DST_H_PIC];
static bool common_tables_initialized = false;

// Function to initialize common interpolation tables
void init_common_tables() {
    if (common_tables_initialized) return;
    
    // Calculate common integer coordinates for all methods (scale factor 3 for 32x32 to 96x96)
    for (int dx = 0; dx < DST_W_PIC; dx++) {
        int src_x = (dx * 3) / 3; // Integer-based scaling (dx * 3) / 3
        common_x0[dx] = src_x;
        common_x1[dx] = src_x + 1;
    }

    for (int dy = 0; dy < DST_H_PIC; dy++) {
        int src_y = (dy * 3) / 3; // Integer-based scaling (dy * 3) / 3
        common_y0[dy] = src_y;
        common_y1[dy] = src_y + 1;
    }

    common_tables_initialized = true;
}

// Interpolation functions (declarations)
uint16_t bio_nearest_neighbor(int dst_x, int dst_y, uint16_t src_data[32][32]);
uint16_t bio_bilinear_1bit(int dst_x, int dst_y, uint16_t src_data[32][32]);
uint16_t bio_bilinear_2bit(int dst_x, int dst_y, uint16_t src_data[32][32]);
uint16_t bio_bilinear_4bit(int dst_x, int dst_y, uint16_t src_data[32][32]);

// Function pointer type for interpolation functions
typedef uint16_t (*InterpolationFunc)(int, int, uint16_t[32][32]);

// Array of interpolation functions
static const InterpolationFunc interpolation_funcs[] = {
    bio_nearest_neighbor,   // Index 0
    bio_bilinear_1bit,      // Index 1
    bio_bilinear_2bit,      // Index 2
    bio_bilinear_4bit       // Index 3
};

// Unified interface for calling interpolation
inline uint16_t bio_interpolate_PIC(int method, int dst_x, int dst_y, uint16_t src_data[32][32]) {
    return interpolation_funcs[method](dst_x, dst_y, src_data);
}

// Nearest Neighbor interpolation (optimized)
static uint8_t nn_x_map[DST_W_PIC];
static uint8_t nn_y_map[DST_H_PIC];
static bool nn_tables_initialized_PIC = false;

void init_nearest_neighbor_tables_PIC() {
    if (nn_tables_initialized_PIC) return;
    
    for (int dx = 0; dx < DST_W_PIC; dx++) {
        nn_x_map[dx] = (dx * 3) / 3;
    }
    
    for (int dy = 0; dy < DST_H_PIC; dy++) {
        nn_y_map[dy] = (dy * 3) / 3;
    }
    
    nn_tables_initialized_PIC = true;
}

uint16_t bio_nearest_neighbor_PIC(int dst_x, int dst_y, uint16_t src_data[32][32]) {
    if (!nn_tables_initialized_PIC) init_nearest_neighbor_tables_PIC();
    return src_data[nn_y_map[dst_y]][nn_x_map[dst_x]];
}

// Bilinear interpolation with 1-bit precision (optimized)
uint16_t bio_bilinear_1bit_PIC(int dst_x, int dst_y, uint16_t src_data[32][32]) {
    if (!common_tables_initialized_PIC) init_common_tables_PIC();
    
    uint8_t x0 = common_x0[dst_x];
    uint8_t x1 = common_x1[dst_x];
    uint8_t y0 = common_y0[dst_y];
    uint8_t y1 = common_y1[dst_y];
    
    uint16_t v00 = src_data[y0][x0];
    uint16_t v01 = src_data[y0][x1];
    uint16_t v10 = src_data[y1][x0];
    uint16_t v11 = src_data[y1][x1];
    
    uint16_t top = (v00 + v01) >> 1;
    uint16_t bottom = (v10 + v11) >> 1;
    
    return (top + bottom) >> 1;
}

// Bilinear interpolation with 2-bit precision (optimized)
uint16_t bio_bilinear_2bit_PIC(int dst_x, int dst_y, uint16_t src_data[32][32]) {
    if (!common_tables_initialized_PIC) init_common_tables_PIC();
    
    uint8_t x0 = common_x0[dst_x];
    uint8_t x1 = common_x1[dst_x];
    uint8_t y0 = common_y0[dst_y];
    uint8_t y1 = common_y1[dst_y];
    
    uint16_t v00 = src_data[y0][x0];
    uint16_t v01 = src_data[y0][x1];
    uint16_t v10 = src_data[y1][x0];
    uint16_t v11 = src_data[y1][x1];
    
    uint16_t top = v00 * 3 + v01 * 2;
    uint16_t bottom = v10 * 3 + v11 * 2;
    
    return (top + bottom) / 6;
}

// Bilinear interpolation with 4-bit precision (optimized)
uint16_t bio_bilinear_4bit_PIC(int dst_x, int dst_y, uint16_t src_data[32][32]) {
    if (!common_tables_initialized_PIC) init_common_tables_PIC();
    
    uint8_t x0 = common_x0[dst_x];
    uint8_t x1 = common_x1[dst_x];
    uint8_t y0 = common_y0[dst_y];
    uint8_t y1 = common_y1[dst_y];
    
    uint16_t v00 = src_data[y0][x0];
    uint16_t v01 = src_data[y0][x1];
    uint16_t v10 = src_data[y1][x0];
    uint16_t v11 = src_data[y1][x1];
    
    uint16_t top = v00 * 5 + v01 * 3;
    uint16_t bottom = v10 * 5 + v11 * 3;
    
    return (top + bottom) / 8;
}

#endif
