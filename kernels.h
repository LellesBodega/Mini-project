// kernels.h
#ifndef KERNELS_H
#define KERNELS_H

#define KERNEL_SIZE_3 3
#define KERNEL_SIZE_5 5

typedef enum {
    KERNEL_EDGE,
    KERNEL_BOXBLUR,
    KERNEL_GAUSSIAN,
    KERNEL_SHARPEN
} kernel_type_t;

// Forward declaration av menu_state_t
typedef struct menu_state_t menu_state_t;   // OBS: "struct menu_state_t", ej typedef än

// 3x3 kernels
extern const int edge_3x3[3][3];
extern const int boxblur_3x3[3][3];
extern const int gaussian_3x3[3][3];
extern const int sharpen_3x3[3][3];

// 5x5 kernels
extern const int edge_5x5[5][5];
extern const int boxblur_5x5[5][5];
extern const int gaussian_5x5[5][5];
extern const int sharpen_5x5[5][5];

// Funktion för att få valda kernel baserat på typ och storlek
const int* get_selected_kernel(const menu_state_t* menu, int* divisor);

// Convolution function
void convolve(const unsigned char* input, unsigned char* output, int width, int height, const int* kernel, int ksize, int divisor, int offset);

#endif