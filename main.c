#include <stdlib.h>
#include <stdio.h>
#include <xc.h>
#include "System/system.h"
#include "System/delay.h"
#include "oledDriver/oledC.h"
#include "oledDriver/oledC_colors.h"
#include "oledDriver/oledC_shapes.h"

typedef unsigned short u16;
typedef char u4;

typedef enum
{
    RED,
    GREEN,
    BLUE
} color_t;

typedef struct
{
    u16 red;
    u16 green;
    u16 blue;
    u16 prev_potentiometer_value;
    color_t current_color;
} RGB_t;

void in_out_initialize(void)
{
    // TRISx 1-input 0-output
    TRISAbits.TRISA11 = 1; // button S1
    TRISAbits.TRISA12 = 1; // button s2
    TRISBbits.TRISB12 = 1; // potentiomitor
    TRISAbits.TRISA8 = 0;  // LED1
    TRISAbits.TRISA9 = 0;  // LED2
    RPOR13bits.RP26R = 13; // connecting OC1 to RP26 aka RPOR13[0:7]
    RPOR13bits.RP27R = 14;
    RPOR11bits.RP23R = 15;
    ANSBbits.ANSB12 = 1; // potentiomitor as analog input
}

void a_to_d_convertor_initialize(void)
{
    AD1CON1bits.SSRC = 0;
    AD1CON1bits.MODE12 = 0;
    AD1CON1bits.ADON = 1;
    AD1CON1bits.FORM = 0;
    AD1CON2bits.SMPI = 0;
    AD1CON2bits.PVCFG = 0;
    AD1CON2bits.NVCFG0 = 0;
    AD1CON3bits.ADCS = 0xFF;
    AD1CON3bits.SAMC = 0B10000;
}

void RGB_initialize(void)
{
    OC1CON2bits.SYNCSEL = 0x1F; // self-sync
    OC1CON2bits.OCTRIG = 0;     // sync mode
    OC1CON1bits.OCTSEL = 0B111; // FOSC/2
    OC1CON1bits.OCM = 0B110;    // edge aligned
    OC1CON2bits.TRIGSTAT = 1;
    OC2CON2bits.SYNCSEL = 0x1F; // self-sync
    OC2CON2bits.OCTRIG = 0;     // sync mode
    OC2CON1bits.OCTSEL = 0B111; // FOSC/2
    OC2CON1bits.OCM = 0B110;    // edge aligned
    OC2CON2bits.TRIGSTAT = 1;
    OC3CON2bits.SYNCSEL = 0x1F; // self-sync
    OC3CON2bits.OCTRIG = 0;     // sync mode
    OC3CON1bits.OCTSEL = 0B111; // FOSC/2
    OC3CON1bits.OCM = 0B110;    // edge aligned
    OC3CON2bits.TRIGSTAT = 1;
}

void RGB_set_span()
{
    OC1RS = 1023;
    OC2RS = 1023;
    OC3RS = 1023;
}

void main_initialize(void)
{
    SYSTEM_Initialize();
    in_out_initialize();
    a_to_d_convertor_initialize();
    RGB_initialize();
    RGB_set_span();
}

u16 potentiometer_value()
{
    AD1CHSbits.CH0SA3 = 1; // select AN8 for A/D conversion ptoentiometer is connected to AN8
    AD1CON1bits.SAMP = 1;  // start sampling
    for (int i = 1; i < 1000; i++)
        ;
    AD1CON1bits.SAMP = 0;
    // for (int i = 1; i < 100; i++)
    //     ;
    while (AD1CON1bits.DONE == 0)
        ;
    return ADC1BUF0;
}

// mode = 0
// changes to the next color in the sequence e.g. red -> green -> blue -> red etc...
void set_RGB_color(RGB_t *rgb)
{
    switch (rgb->current_color)
    {
    case RED:
        OC1R = rgb->red;
        OC2R = 0;
        OC3R = 0;
        break;
    case GREEN:
        OC2R = rgb->green;
        OC1R = 0;
        OC3R = 0;
        break;
    case BLUE:
        OC3R = rgb->blue;
        OC1R = 0;
        OC2R = 0;
        break;
    }
}

void update_color(RGB_t *rgb)
{
    switch (rgb->current_color)
    {
    case RED:
        rgb->red = rgb->prev_potentiometer_value;
        break;
    case GREEN:
        rgb->green = rgb->prev_potentiometer_value;
        break;
    case BLUE:
        rgb->blue = rgb->prev_potentiometer_value;
        break;
    }
}

// mode = 1
void composite_mode(RGB_t *rgb)
{
    float relative_pot = (rgb->prev_potentiometer_value / 1023.0);
    OC1R = rgb->red * relative_pot;
    OC2R = rgb->green * relative_pot;
    OC3R = rgb->blue * relative_pot;
}

void swap_mode(u4 mode_select, RGB_t *rgb)
{
    switch (mode_select)
    {
    case 0:
        set_RGB_color(rgb);
        break;
    case 1:
        composite_mode(rgb);
        break;
    }
}

/*
     Main application
 */
int main(void)
{
    u4 mode_select = 0;
    u4 s1_pressed = 0;
    u4 s2_pressed = 0;
    u16 pot = 0;
    RGB_t rgb = {512, 512, 512, 0, RED}; // red, green, blue, potentiometer value, current color
    main_initialize();                   // initialize the system

    // main loop
    while (1)
    {
        pot = potentiometer_value();
        // if the potentiometer value changes by 25
        if ((pot + 25) % 1024 < rgb.prev_potentiometer_value || (pot - 25) % 1024 > rgb.prev_potentiometer_value)
        {
            rgb.prev_potentiometer_value = pot;
            // if mode_select is 0 (individual color), then update that color
            mode_select ? NULL : update_color(&rgb);
        }
        // if button S1 is pressed
        // check if the button s1 is held down
        // if it is held down, then only do this once
        // e.g. the value of pressed will be 1 after the first iteration.
        if (PORTAbits.RA11 == 0)
        {
            LATAbits.LATA8 = 1;
            if (s1_pressed == 0)
            {
                s1_pressed = 1;
                mode_select = (mode_select == 0) ? 1 : 0;
            }
        }
        else
        {
            s1_pressed = 0;
            LATAbits.LATA8 = 0;
        }

        if (mode_select == 0)
        {
            if (PORTAbits.RA12 == 0)
            {
                LATAbits.LATA9 = 1;
                if (s2_pressed == 0)
                {
                    s2_pressed = 1;
                    rgb.current_color = (rgb.current_color + 1) % 3;
                }
            }
            else
            {
                LATAbits.LATA9 = 0;
                s2_pressed = 0;
            }

            set_RGB_color(&rgb);
        }
        // button S2 is pressed and mode_select is individual

        if (mode_select == 1)
        {
            composite_mode(&rgb);
        }
    }
    return 1;
}
/**
 End of File
 */