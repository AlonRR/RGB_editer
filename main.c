#include <stdlib.h>
#include <stdio.h>
#include <xc.h>
#include "System/system.h"
#include "System/delay.h"
#include "oledDriver/oledC.h"
#include "oledDriver/oledC_colors.h"
#include "oledDriver/oledC_shapes.h"

void in_out_initialize(void)
{
    // TRISx 1-input 0-output
    TRISAbits.TRISA11 = 1; // button S1
    TRISAbits.TRISA12 = 1; // button s2
    TRISBbits.TRISB12 = 1; // potentiomitor
    // TRISAbits.TRISA0 = 0; //RED
    // TRISAbits.TRISA1 = 0; //GREEN
    // TRISCbits.TRISC7 = 0; //BLUE
    OC1CON1.
}

/*
     Main application
 */
int main(void)
{

    // initialize the system
    SYSTEM_Initialize();
    in_out_initialize();
    RPOR13bits.RP26R = 13; // connecting OC1 to RP26 aka RPOR13[0:7]
    OC1RS = 1023;
    OC1CON2bits.SYNCSEL = 0x1F; // self-sync
    OC1CON2bits.OCTRIG = 0;     // sync mode
    OC1CON1bits.OCTSEL = 0b111; // FOSC/2
    OC1CON1bits.OCM = 0b110;    // edge aligned
    OC1CON2bits.TRIGSTAT = 1;
    RPOR13bits.RP27R = 100;
    RPOR11bits.RP23R = 100;

    while (1)
    {
        if (PORTAbits.RA11 == 1)
        {
            //            PORTAbits.RA0 = 0; //on
            OC1R = 512;
        }
        else
        {
            //            PORTAbits.RA0 = 1; // off
            OC1R = 0;
        }
        if (PORTAbits.RA12 == 1)
        {
            PORTCbits.RC7 = 0;
        }
        else
        {
            PORTCbits.RC7 = 1;
        }
    }
    return 1;
}
/**
 End of File
 */