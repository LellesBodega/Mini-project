/**
 * main.c - Huvudprogram för inbyggd bildbehandling på DTEK-V
 *
 * Denna version använder en bild som är inbakad direkt i programmet
 * från en header-fil (t.ex. 'cat_image.h'). Detta eliminerar behovet
 * av 'dtekv-upload' under körning.
 *
 * Logik:
 * 1. Vid start laddas den inbyggda bilden automatiskt in i 'input_img'.
 * 2. Användaren väljer filter och storlek med switchar (SW[1:0] och SW[2]).
 * 3. En knapptryckning (BTN[0]) bekräftar en handling:
 * - Om SW[3] är på: Bearbeta bilden (kör convolve).
 * - Om SW[4] är på: Chain commands (två filter i följd).
 * 4. Den bearbetade bilden i 'output_img' kan laddas ner med 'dtekv-download'.
 */

// Created by Yannsze from lab3, main modified by Jacob

#include <string.h>
#include "dtekv-lib.h"
#include "main.h"
#include "menu.h"
#include "kernels.h"

// Inkludera headern med bild-arrayen
#include "cat_image.h"

// ===========================================================
// Globala bildbuffertar
// ===========================================================

// Definierade i main.h, men allokerade här.

unsigned char output_img[IMG_HEIGHT][IMG_WIDTH];
unsigned char input_img[IMG_HEIGHT][IMG_WIDTH];
unsigned char temp_img[IMG_HEIGHT][IMG_WIDTH];

// ===========================================================
// Globala variabler 
// ===========================================================

int timeoutcount = 0; // Används av interrupt-hanteraren

// ===========================================================
// Externa symboler (från andra filer)
// ===========================================================

extern void enable_interrupt();
extern void delay(int);

// ===========================================================
// Timer- och systeminitiering
// ===========================================================

void labinit(void) {

    print("Initializing system...\n");

    // Sätt upp timer för att generera en interrupt varje 100ms (vid 30 MHz)

    timer_control = 0x8; // Stäng av timer medan vi konfigurerar
    timer_periodl = 3000000 & 0xFFFF;
    timer_periodh = (3000000 >> 16) & 0xFFFF;
    timer_status = 0;    // Nollställ status
    timer_control = 0x7; // Aktivera timer, starta, och tillåt interrupt

}

// ===========================================================
// Interrupt-hantering
// ===========================================================

void handle_interrupt(unsigned cause) {

    // Kontrollera om det var en timer-interrupt

    if (cause == 16) {
        if (timer_status & 1) {
            timer_status = 0; // Rensa interrupt-flaggan
            timeoutcount++;
        }
    }
}



// ===========================================================
// Hjälpfunktioner för I/O
// ===========================================================

void set_leds(int led_mask) {
    led_reg = led_mask & 0x3FF; // Använd endast de 10 lägsta bitarna
}

int get_sw(void) {
    return toggle_reg & 0x3FF;
}

int get_btn(void) {
    return btn_reg & 0x1;
}

// Jacob
// Laddar den inbyggda bilden till input_img bufferten.
void load_initial_image(void) {
    print("Loading embedded cat image into input buffer...\n");
    memcpy(input_img, cat_img, sizeof(cat_img));
    print("Image loaded.\n");
}

// Återställer bilden till originalet och rensar output.
void reset_images(void) {
    print("Resetting images...\n");
    load_initial_image(); // Åter-ladda originalbilden
    memset(output_img, 0, sizeof(output_img));
    print("Images reset to initial state.\n");
}

// ===========================================================
// Huvudprogram
// ===========================================================
int main(void) {
    labinit();
    delay(100000); // Liten fördröjning för att systemet ska stabiliseras

    // --- Steg 1: Skriv ut viktig information vid start ---
    print("\n=== DTEK-V Embedded Image Processor ===\n");
    // Ladda den inbyggda bilden direkt vid start
    load_initial_image();

    // Skriv ut minnesadresser för JTAG-överföring (endast för nedladdning nu)
    print("\n--> Use this address with dtekv-download:\n");
    print("output_img address: ");
    print_hex32((unsigned int)output_img);
    print("\nImage size in bytes: 65536\n\n");

    // Skriv ut uppdaterade instruktioner
    print("--- Instructions ---\n");
    print("The cat image is pre-loaded.\n");
    print("1. Use switches to select operation:\n");
    print("   SW[1:0]: Kernel Type (00=Edge, 01=Box, 10=Gauss, 11=Sharp)\n");
    print("   SW[2]:   Kernel Size (0=3x3, 1=5x5)\n");
    print("   SW[3]:   Set to 1 to enable 'Process Image' action\n");
    print("   SW[4]:   Set to 1 to enable 'Chain Process Image' action\n");
    print("   SW[6]:   Set to 1 to enable 'Reset Image' action\n");
    print("2. Press BTN[0] to execute the selected action.\n");
    print("3. Download result from host: dtekv-download <out.raw> <output_addr> 65536\n\n");

    print("\n--- Menu loop starting ---\n");

    // Initiera meny och knappstatus
    menu_state_t menu;
    menu_init(&menu);
    int last_btn = 0;

    // =======================================================
    // Huvudloop
    // =======================================================
    while (1) {
        int switches = get_sw();
        int btn = get_btn();

        // Uppdatera menystatus baserat på switchar och visa på lysdioder
        menu_update(&menu, switches, btn);
        menu_show(&menu);

        // Kontrollera om en knapptryckning precis har skett (stigande flank)
        if (btn && !last_btn) {

            // KONTROLL 1: Är "Process Image"-läget (SW[3]) aktivt?
            if (menu.run_mode) {
                int divisor;
                const int* kernel = get_selected_kernel(&menu, &divisor);

                if (kernel) {
                    // Kontrollera om vi är i kedjeläge (SW[4])
                    if (menu.chain_mode) {
                        print("Processing image in CHAIN mode...\n");

                        // --- Första kerneln ---
                        print("Applying first kernel...\n");
                        const int* kernel1 = get_selected_kernel(&menu, &divisor);
                        convolve((unsigned char*)input_img, (unsigned char*)temp_img,
                                IMG_WIDTH, IMG_HEIGHT, kernel1, menu.kernel_size, divisor, 0);

                        // --- Vänta på användarval av nästa filter ---
                        print("\nFirst filter done. Select second kernel with switches and press BTN[0] again.\n");
                        print("SW[1:0]: Kernel Type, SW[2]: Kernel Size.\n");
                        print("Waiting for button press...\n");

                        // Vänta tills användaren trycker igen
                        while (get_btn() == 0);
                        while (get_btn() == 1); // vänta tills knappen släpps
                        
                        //Läs in switcharna på nytt och uppdatera menyn
                        int switches2 = get_sw();
                        menu_update(&menu, switches2, 1); // simulera nytt knapptryck i menyn

                        // --- Andra kerneln ---
                        int divisor2;
                        const int* kernel2 = get_selected_kernel(&menu, &divisor2);
                        print("Applying second kernel...\n");
                        convolve((unsigned char*)temp_img, (unsigned char*)output_img,
                                IMG_WIDTH, IMG_HEIGHT, kernel2, menu.kernel_size, divisor2, 0);

                        print("Processing complete. Image is ready for download.\n");
                    } else {
                        //Den vanliga single-filter-processen
                        print("Processing image in SINGLE mode...\n");
                        convolve((unsigned char*)input_img, (unsigned char*)output_img,
                                IMG_WIDTH, IMG_HEIGHT, kernel, menu.kernel_size, divisor, 0);
                        print("Processing complete. Image is ready for download.\n");
                    }
                } else {
                    print("Error: Could not get selected kernel.\n");
    }
}
            
            // KONTROLL 2: Om INTE process-läget var aktivt, är "Reset" (SW[6]) det?
            else if (menu.reset) {
                reset_images();
            }

        } // Slut på if(btn && !last_btn)
        // Spara knappens nuvarande tillstånd för att kunna detektera nästa tryck
        last_btn = btn;
        
    } // Slut på while(1)
}
