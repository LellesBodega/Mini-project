// menu.h
#ifndef MENU_H
#define MENU_H

#include "kernels.h"

// Menyval och status
typedef struct menu_state_t {
    kernel_type_t kernel_selected; // 0-3
    int kernel_size;               // 3 eller 5
    int run_mode;                  // 0 = single, 1 = chain
    int upload;                    // 1 = upload image
    int download;                  // 1 = download image
    int reset;                     // 1 = reset
} menu_state_t;

// Initiera meny
void menu_init(menu_state_t* state);

// Uppdatera meny baserat på toggles och knappar
void menu_update(menu_state_t* state, int switches, int btn);

// Visa meny/status på LED eller display
void menu_show(const menu_state_t* state);

#endif