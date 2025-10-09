// Jacob created the function of kernels through pseoducode from Wiki

// kernels.c
#include "dtekv-lib.h"
#include "kernels.h"
#include "menu.h"
#include <stddef.h>

// (3x3) och (5x5) områden

// Edge detection kernels
// Markerade kanter genom att förstärka skillnader mellan en pixel och dess grannar.
// Summan av alla vikter är 0, vilket gör att homogena områden blir svarta och kanter framhävs. 
// 5x5 = större område, kan ge bredare kanter.

// Box blur kernels
// Enkel medelvärdesutjämning. Alla pixlar i området får lika stor vikt.
// Dividera med 9 efteråt för att få medelvärdet.
// 5x5 område = Dividera med 25 efteråt.

// Gaussian blur kernels
// Viktar mittenpixeln högst och hörnen lägst, vilket ger en mjukare och mer naturlig suddighet än box blur.
// Dividera med 16 efteråt då det är summan av alla vikter.
// 5x5 Mjukare suddighet än 3x3-versionen. Dividera med 256 efteråt då det är summan av alla vikter.


// Sharpen kernels (3x3)
// Förstärker skillnader mellan mittenpixeln och dess grannar, vilket gör bilden skarpare.
// Summan av vikterna är 1, så ljusstyrkan bibehålls.
// 5


//3x3 kernels
const int edge_3x3[3][3] = {
    { -1, -1, -1 },
    { -1,  8, -1 },
    { -1, -1, -1 }
};

const int boxblur_3x3[3][3] = {
    { 1, 1, 1 },
    { 1, 1, 1 },
    { 1, 1, 1 }
};

const int gaussian_3x3[3][3] = {
    { 1, 2, 1 },
    { 2, 4, 2 },
    { 1, 2, 1 }
};

const int sharpen_3x3[3][3] = {
    {  0, -1,  0 },
    { -1,  5, -1 },
    {  0, -1,  0 }
};

// 5x5 kernels
const int edge_5x5[5][5] = {
    { -1, -1, -1, -1, -1 },
    { -1,  1,  2,  1, -1 },
    { -1,  2,  4,  2, -1 },
    { -1,  1,  2,  1, -1 },
    { -1, -1, -1, -1, -1 }
};

const int boxblur_5x5[5][5] = {
    { 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1 }
};

const int gaussian_5x5[5][5] = {
    { 1,  4,  6,  4, 1 },
    { 4, 16, 24, 16, 4 },
    { 6, 24, 36, 24, 6 },
    { 4, 16, 24, 16, 4 },
    { 1,  4,  6,  4, 1 }
};

const int sharpen_5x5[5][5] = {
    {  0,  0, -1,  0,  0 },
    {  0, -1, -2, -1,  0 },
    { -1, -2, 13, -2, -1 },
    {  0, -1, -2, -1,  0 },
    {  0,  0, -1,  0,  0 }
};

const int* get_selected_kernel(const menu_state_t* menu, int* divisor) {
    // 🔍 Debugutskrift för att se vilket kernel som valts via switchar
    print("Kernel select: ");
    print_dec(menu->kernel_selected);
    print("  size=");
    print_dec(menu->kernel_size);
    print("\n");
    
    if (menu->kernel_size == KERNEL_SIZE_3) {
        switch (menu->kernel_selected) {
            case KERNEL_EDGE:
                *divisor = 1; // Edge detection behöver ingen normalisering
                return (const int*)edge_3x3;
            case KERNEL_BOXBLUR:
                *divisor = 9; // Box blur 3x3
                return (const int*)boxblur_3x3;
            case KERNEL_GAUSSIAN:
                *divisor = 16; // Gaussian blur 3x3
                return (const int*)gaussian_3x3;
            case KERNEL_SHARPEN:
                *divisor = 1; // Sharpen behöver ingen normalisering
                return (const int*)sharpen_3x3;
        }
    } else if (menu->kernel_size == KERNEL_SIZE_5) {
        switch (menu->kernel_selected) {
            case KERNEL_EDGE:
                *divisor = 1; // Edge detection behöver ingen normalisering
                return (const int*)edge_5x5;
            case KERNEL_BOXBLUR:
                *divisor = 25; // Box blur 5x5
                return (const int*)boxblur_5x5;
            case KERNEL_GAUSSIAN:
                *divisor = 256; // Gaussian blur 5x5
                return (const int*)gaussian_5x5;
            case KERNEL_SHARPEN:
                *divisor = 1; // Sharpen behöver ingen normalisering
                return (const int*)sharpen_5x5;
        }
    }
    // Fallback, bör inte nås
    *divisor = 1;
    return NULL;
}
/*
 * Funktion: convolve
 * ------------------
 * Utför konvolution mellan en inmatningsbild och en kernel/filter.
 * - input: Pekare till indata (gråskale-bild, 1 byte per pixel)
 * - output: Pekare till utdata (samma format)
 * - width, height: Bildens storlek
 * - kernel: Pekare till kernelmatrisen (måste vara flattenad, dvs int*)
 * - ksize: Kernelns storlek (t.ex. 3 eller 5)
 * - divisor: Normaliseringsfaktor (t.ex. 9 för boxblur 3x3, 16 för gaussian 3x3)
 * - offset: Läggs till resultatet (ofta 0)
 *
 * För varje pixel summeras produkten av kernel och motsvarande bildområde.
 * Resultatet normaliseras och klipps till [0,255].
 */
void convolve(const unsigned char* input, unsigned char* output, int width, int height, const int* kernel, int ksize, int divisor, int offset) {
     print("Convolve started\n");
    int kcenter = ksize / 2;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int acc = 0;
            for (int ky = 0; ky < ksize; ky++) {
                for (int kx = 0; kx < ksize; kx++) {
                    int ix = x + kx - kcenter;
                    int iy = y + ky - kcenter;
                    // Kontrollera att vi är inom bildens gränser
                    if (ix >= 0 && ix < width && iy >= 0 && iy < height) {
                        acc += input[iy * width + ix] * kernel[ky * ksize + kx];
                    }
                }
            }
            acc = acc / divisor + offset;
            if (acc < 0) acc = 0;
            if (acc > 255) acc = 255;
            output[y * width + x] = (unsigned char)acc;
        }
    }
    print("Convolve done\n");
}