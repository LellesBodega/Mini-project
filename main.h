#ifndef MAIN_H
#define MAIN_H

#define IMG_WIDTH 64
#define IMG_HEIGHT 64

// Hardware Registers mapping
// Timer Registers
#define timer_status (*(volatile unsigned int *) 0x04000020)
#define timer_control (*(volatile unsigned int *) 0x04000024)
#define timer_periodl (*(volatile unsigned int *) 0x04000028)
#define timer_periodh (*(volatile unsigned int *) 0x0400002C)

// I/O Registers
#define toggle_reg (*(volatile unsigned int *) 0x04000010)
#define toggle_regOffset (*(volatile unsigned int *) 0x04000018)
#define btn_reg (*(volatile unsigned int *) 0x040000d0)
#define led_reg (*(volatile unsigned int *) 0x04000000)

// Functions 
void labinit(void);
void handle_interrupt(unsigned cause);

void set_leds(int led_mask);
void set_displays(int display_number, int value);
int get_sw(void);
int get_btn(void);

// UART functions
void uart_putchar(unsigned char c);
int uart_getchar(void);

extern unsigned char input_img[IMG_WIDTH * IMG_HEIGHT];
extern unsigned char output_img[IMG_WIDTH * IMG_HEIGHT];
extern int mytime;
extern char textstring[];
extern int prime;
extern int timeoutcount;

#endif