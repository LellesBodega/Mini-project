/* main.c
   Written 2024 by Artur Podobas and Pedro Antunes
   Adapted 2025 for image selection and processing on DTEK-V board
   Author: JW (KTH)
*/

#include <stdio.h>
#include <string.h>
#include "dtekv-lib.h"   // UART, I/O
#include "main.h"
#include "menu.h"
#include "kernels.h"

// ===========================================================
// Globala bildbuffertar
// ===========================================================
unsigned char input_img[IMG_WIDTH * IMG_HEIGHT];
unsigned char output_img[IMG_WIDTH * IMG_HEIGHT];

// ===========================================================
// Globala variabler
// ===========================================================
int timeoutcount = 0;
int mytime = 0x5957;

// ===========================================================
// Externa symboler
// ===========================================================
extern void print(const char*);
extern void print_dec(unsigned int);
extern void display_string(char*);
extern void time2string(char*, int);
extern void tick(int*);
extern void delay(int);
extern int nextprime(int);
extern void enable_interrupt();

// ===========================================================
// Timer- och systeminitiering
// ===========================================================
void labinit(void) {
    print("Initializing lab...\n");

    // Timer varje 100ms (≈10 Hz vid 30 MHz)
    timer_control = 0x8; // disable
    timer_periodl = 3000000 & 0xFFFF;
    timer_periodh = (3000000 >> 16) & 0xFFFF;
    timer_status = 0;
    timer_control = 0x7; // enable timer + start + interrupt

    toggle_regOffset = 0x4;
    // enable_interrupt(); // valfritt
}

// ===========================================================
// Interrupt-hantering (om används)
// ===========================================================
void handle_interrupt(unsigned cause) {
    if (cause == 16) {
        if (timer_status & 1) {
            timer_status = 0;
            timeoutcount++;
            // Här kan du lägga kod för periodisk statusuppdatering
        }
    }
}

// ===========================================================
// Hjälpfunktioner för I/O
// ===========================================================
void set_leds(int led_mask) {
    led_reg = led_mask & 0x3FF;  // endast 10 LSB
}

int get_sw(void) {
    return toggle_reg & 0x3FF;
}

int get_btn(void) {
    return btn_reg & 0x1;
}

void set_displays(int display_number, int value) {
    int offset = 0x10;
    if (display_number < 0 || display_number > 5)
        return;

    volatile unsigned int *display_reg =
        (volatile unsigned int *)(0x04000050 + display_number * offset);

    unsigned char digit[10] = {
        0x40, 0x79, 0x24, 0x30, 0x19,
        0x12, 0x02, 0x78, 0x00, 0x18
    };

    if (value < 0 || value > 9)
        return;

    *display_reg = digit[value];
}

// ===========================================================
// Bildhanteringsfunktioner
// ===========================================================
void upload_image(void) {
    print("Uploading image...\n");
    for (int y = 0; y < IMG_HEIGHT; y++) {
        for (int x = 0; x < IMG_WIDTH; x++) {
            int val = uart_getchar();
            if (val < 0)
                val = 0;
            input_img[y * IMG_WIDTH + x] = (unsigned char)val;
        }
    }
    print("Image upload complete.\n");
}

void download_image(void) {
    print("Downloading image...\n");
    for (int y = 0; y < IMG_HEIGHT; y++) {
        for (int x = 0; x < IMG_WIDTH; x++) {
            uart_putchar(output_img[y * IMG_WIDTH + x]);
        }
    }
    print("Download image complete.\n");
}

void reset_image(void) {
    print("Resetting image buffers...\n");
    memset(input_img, 0, sizeof(input_img));
    memset(output_img, 0, sizeof(output_img));
    print("Image reset complete.\n");
}

// ===========================================================
// Huvudprogram: Meny, bildval och bildbehandling
// ===========================================================
int main(void) {
    labinit();
    delay(100000);
    print("Hello DTEK-V!\n");

    unsigned int ctrl = *(volatile unsigned int*)0x04000044;
    if (ctrl & (1 << 10)) {
        print("JTAG host connected!\n");
    } else {
        print("No JTAG host detected!\n");
    }

    // Initiera meny
    menu_state_t menu;
    menu_init(&menu);

    // Startmeddelande
    print("=== Welcome to the DTEK-V Image Processor ===\n");
    print("Use SW[1:0] to select kernel:\n");
    print("00=Edge  01=Boxblur  10=Gaussian  11=Sharpen\n");
    print("SW[2] = Kernel size (0=3x3, 1=5x5)\n");
    print("SW[3] = Run mode (0=Single, 1=Chain)\n");
    print("SW[4]=Upload  SW[5]=Download  SW[6]=Reset\n");
    print("BTN[0] = Confirm actions / selections\n\n");

    // --- Bildvalsförberedelser ---
    int last_btn = 0;
    int image_selected = 0;
    const char* image_files[3] = { "cat.raw", "squid.raw", "dog.raw" };

    // =======================================================
    // Huvudloop
    // =======================================================
    while (1) {
        int switches = get_sw();
        int btn = get_btn();

        // --- Steg 1: Bildval ---
        if (!image_selected) {
            print("Choose an image to process:\n");
            print("SW[1:0] selects image:\n");
            print("00=cat.raw, 01=squid.raw, 10=dog.raw\n");
            print("Press BTN[0] to confirm.\n");

            int index = switches & 0x3;
            if (index > 2) index = 2;
            const char* filename = image_files[index];

            if (btn && !last_btn) {
                FILE *f = fopen(filename, "rb");
                if (f) {
                    fread(input_img, 1, IMG_WIDTH * IMG_HEIGHT, f);
                    fclose(f);
                    print("Image loaded successfully: ");
                    print(filename);
                    print("\n");
                    image_selected = 1;
                } else {
                    print("Error: could not open ");
                    print(filename);
                    print("\n");
                }
            }

            last_btn = btn;
            if (!image_selected)
                continue; // vänta tills bild laddats
        }

        // --- Steg 2: Menyuppdatering ---
        menu_update(&menu, switches, btn);
        menu_show(&menu);

        // --- Steg 3: Kör vald kernel ---
        if (menu.run_mode && image_selected) {
            int divisor;
            const int* kernel = get_selected_kernel(&menu, &divisor);
            if (kernel) {
                convolve(input_img, output_img, IMG_WIDTH, IMG_HEIGHT,
                        kernel, menu.kernel_size, divisor, 0);

                // ✅ Spara automatiskt processad bild till SD-kort
                char proc_filename[32];
             int index = switches & 0x3;
                if (index > 2) index = 2;
                snprintf(proc_filename, sizeof(proc_filename), "%s_proc.raw", image_files[index]);

                FILE *f = fopen(proc_filename, "wb");
                if (f) {
                   fwrite(output_img, 1, IMG_WIDTH * IMG_HEIGHT, f);
                   fclose(f);
                    print("Processed image saved to: ");
                   print(proc_filename);
                 print("\n");
             } else {
                   print("Error: could not save processed image: ");
                   print(proc_filename);
                   print("\n");
        }
    }

        // --- Steg 4: Hantera knapptryck ---
        if (btn && !last_btn) {
            if (menu.upload) {
                print("Upload triggered.\n");
                upload_image();
            } 
            else if (menu.download) {
                print("Download triggered.\n");
                download_image();
            } 
            else if (menu.reset) {
                print("Reset triggered.\n");
                reset_image();
                image_selected = 0; // gå tillbaka till bildval
            }
        }

        last_btn = btn;
    }

    return 0;
}
