// Jacob
// menu.c
#include "menu.h"
#include "main.h"
#include "dtekv-lib.h" // för print/debug

// Meny med standardvärden
void menu_init(menu_state_t* state) {
    state->kernel_selected = KERNEL_EDGE;
    state->kernel_size = 3;
    state->run_mode = 0;
    state->reset = 0;
    state->chain_mode = 0;

}

// Uppdatera meny baserat på switches och knapp
// switches[0-1]=kernel, switches[2]=size, switches[3]=run_mode, btn=action

void menu_update(menu_state_t* state, int switches, int btn) {
    // Kernelval: switches 0-1 (två bitar)
    state->kernel_selected = (kernel_type_t)(switches & 0x3);

    // Kernelstorlek: switches 2 (0=3x3, 1=5x5)
    state->kernel_size = (switches & 0x4) ? 5 : 3;   // (switches & 0x4) ? 5 : 3; betyder: 
                                                    // Om toggle 2 värde är satt till 1, dvs nedtryckt, så blir kernel_size 5.
                                                    // Om toggle 2 värde är satt till 0, dvs INTE nedtryckt, så blir kernel_size 3.

    // Run mode: switches 3 (1=process image)
    state->run_mode = (switches & 0x8) ? 1 : 0;

    // Kedjeläge: SW[4] (0 = Single, 1 = Chain)
    state->chain_mode = (switches & 0x10) ? 1 : 0;

    // Reset: switches 6 (håll nere för reset)
    state->reset = (switches & 0x40) ? 1 : 0;

    // Använda btn för att "bekräfta" val? eller starta bearbetning?
    // Typ om btn trycks och upload är valt, trigga upload
}

// Visa meny/status på LED (kerneltyp på LED 0-1, storlek på LED 2, run_mode på LED 3)
void menu_show(const menu_state_t* state) {
    int led_mask = 0;
    led_mask |= (state->kernel_selected & 0x3);      // LED 0-1: kerneltyp
    led_mask |= (state->kernel_size == 5) << 2;      // LED 2: kernelstorlek
    led_mask |= (state->run_mode) << 3;              // LED 3: run mode
    led_mask |= (state->chain_mode) << 4;                // LED 4: upload
    //led_mask |= (state->download) << 5;              // LED 5: download
    led_mask |= (state->reset) << 6;                 // LED 6: reset

    // Anropa LED-funktion
    set_leds(led_mask);
}