/* main.c
   This file written 2024 by Artur Podobas and Pedro Antunes
   For copyright and licensing, see file COPYING */

/* Below functions are external and found in other files. */

#include <stdio.h>
#include <string.h>
#include "dtekv-lib.h" // för uart
#include "main.h"
#include "menu.h"
#include "kernels.h"

// already defined in main.h (remove)?
unsigned char input_img[IMG_WIDTH * IMG_HEIGHT];
unsigned char output_img[IMG_WIDTH * IMG_HEIGHT];

// might be removed later ? --------
int mytime = 0x5957;
char textstring[] = "text, more text, and even more text!";
int prime = 1234567;
// ---------------------------------

int timeoutcount = 0;

//extern void print(const char*);
extern void print_dec(unsigned int);
extern void display_string(char*);
extern void time2string(char*,int);
extern void tick(int*);
extern void delay(int);
extern int nextprime(int);
extern void enable_interrupt();

/* Timer and interrupt initialization */
void labinit(void) {
  print("Initializing lab...\n");
  // initialize timer every 100ms (10 times a second), dtekv = 30MHz
  timer_control = 0x8;   // disable timer 
  timer_periodl = 3000000 & 0xFFFF; // load lower 16bits
  timer_periodh = (3000000 >> 16) & 0xFFFF; // load upper 16bits
  timer_status = 0;  // clear time out flag (1 == timer is done, reached zero)
  timer_control = 0x7;  // enable start + count(removed**) + interrupt request

  toggle_regOffset = 0x4;

  //enable_interrupt();
}

/* Below is the function that will be called when an interrupt is triggered. */
// Might be be needed if we are not using interrupts ----------------
void handle_interrupt(unsigned cause) {
  if (cause == 16) {

    if (timer_status & 1) {
      timer_status = 0; // clear flag
      timeoutcount++;
        // Kod för periodisk uppdatering, menyblink? status?
      }
      // Knapptryckningar och toggles här om vil ha snabb respons
    }
  }
  
/* LED- och displayfunktioner */
void set_leds(int led_mask) {
  // mask 10 lsb
  led_reg = led_mask & 0x3FF;                                
}

// get status of the switches 
int get_sw(void) {
  return toggle_reg & 0x3FF;
}

// get status of the second button
int get_btn(void) {
  return btn_reg & 0x1;
}

// might not be needed - if we are not using the displays --------
void set_displays(int display_number, int value) {
  int offset = 0x10;

  // invalid display_number
  if (display_number < 0 || display_number > 5) 
    return;  

  // map to the 7 segments
  volatile unsigned int *display_reg = (volatile unsigned int *) (0x04000050 + display_number * offset);

  unsigned char digit[10] = {
    0x40, // 0
    0x79,
    0x24,
    0x30,
    0x19,
    0x12,
    0x02,
    0x78,
    0x00,
    0x18  // 9
  };

  if (value < 0 || value > 9)
    return;

  *display_reg = digit[value];
}

void upload_image(void){
  print("Uploading image...\n");
  for (int y = 0; y < IMG_HEIGHT; y++) {
        for (int x = 0; x < IMG_WIDTH; x++) {
          int val = uart_getchar();
          if (val<0) val = 0; // Om inget mottaget, sätt till 0
          input_img[y * IMG_WIDTH + x] = (unsigned char) val;
        }
      }
      print("Image upload complete.\n");
}

void download_image(void){
  print("Downloading image...\n");
  for (int y = 0; y < IMG_HEIGHT; y++) {
    for (int x = 0; x < IMG_WIDTH; x++) {
        uart_putchar(output_img[y * IMG_WIDTH + x]);
        }
      }
      print("Download image complete.\n");
}
  
void reset_image(void){
    print("Resetting image buffers...\n");
    memset(input_img, 0, sizeof(input_img));
    memset(output_img, 0, sizeof(output_img));
    print("Image reset complete.\n");
}

//Main: Menu and image processing
int main ( void ) {
  labinit();
  delay(100000);
  print("Hello DTEKV!\n");

  unsigned int ctrl = *(volatile unsigned int*)0x04000044;
    if (ctrl & (1 << 10)) {
        print("JTAG host connected!\n");
    } else {
        print("No JTAG host detected!\n");
    }

  // Initiera meny
  menu_state_t menu;
  menu_init(&menu);

   // Förklaring till användaren
  print("=== Welcome to the DTEK Image Processor ===\n");
  print("Use the switches to select kernel and options.\n");
  print("[1:0] Kernel: 00=Edge, 01=Boxblur, 10=Gaussian, 11=Sharpen\n");
  print("[2] Kernel Size: 0=3x3, 1=5x5\n");
  print("[3] Run Mode: 0=Single, 1=Chain\n");
  print("[4] Hold for Upload\n");
  print("[5] Hold for Download\n");
  print("[6] Hold for Reset\n");
  print("Press button to confirm actions.\n");

  int last_btn = 0;
  int image_selected = 0;
  const char* image_files[3] = { "cat.raw", "squid.raw", "dog.raw" };  // Flagga om bildval är bekräftat

  while (1) {
    // Exempelvis:
    // 1. Läs switches/knappar
    // 2. Uppdatera meny/status på LED/displayer
    // 3. Kör vald kernel på bilddata
    // 4. Hantera upload/download/reset
    // Använd menu.kernel_selected, menu.kernel_size, menu.run_mode osv i din logik
    
    int switches = get_sw();
    int btn = get_btn();

// --- Bildval (innan menyuppdatering) ---
if (!image_selected) {
    print("Choose an image to process:\n");
    print("SW[1:0] selects image:\n");
    print("00 -> cat.raw\n");
    print("01 -> squid.raw\n");
    print("10 -> dog.raw\n");
    print("Press BTN[0] to confirm selection.\n");

    int index = switches & 0x3;
    if (index > 2) index = 2; // begränsa till 0–2
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
        // --- Slut på bildval ---

    // Uppdatera meny och visa status
    menu_update(&menu, switches, btn); // uppdatera endast meny baserat på knappstatus
    menu_show(&menu);         // uppdatera LED/display

    // Kör vald kernel på input_img → output_img
    if (menu.run_mode && image_selected) {
        int divisor;
        const int* kernel = get_selected_kernel(&menu, &divisor);
        if (kernel) {
            convolve(input_img, output_img, IMG_WIDTH, IMG_HEIGHT,
                     kernel, menu.kernel_size, divisor, 0);
        }
    }

    // Edge-detektering för knapp
    if (btn && !last_btn) {
        if(menu.upload) {
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
            image_selected = 0; // börja om från bildval
        }
      //delay(100);
    }
    last_btn = btn; // uppdatera för nästa iteration

    //menu_update(&menu, get_sw(), 0);
    //menu_show(&menu);
  }
  return 0;
}
