/* 
 * File:   newmain.c
 * Author: Bryan Shum
 * ECE 3301-02 Midterm 2 
 * Created on July 26, 2022, 9:32 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "LiquidCrystal.h"
#include "config.h"

/*
 * 
 */
# define _XTAL_FREQ 1000000
void __interrupt() adc_sample(void);

//setting up variables
    volatile int leftNum;
    volatile int rightNum;
    signed long long ans;
    int symbol;

int main() {    
    // Configure LCD Pins    
    
    // RS = RD0
    // RW = RD1
    // E  = RD2
    TRISD = 0x18;       // Input D3, D4 and output D0, D1, D2
    //TRISDbits.RD3 = 1;
    //TRISDbits.RD4 = 1;
    
    // Output pins connected to seven segment
    TRISB = 0x00;
        
//    INTCON = 0x00;
    // connect the LCD pins to the appropriate PORT pins
    pin_setup(&PORTB, &PORTD);
    
    // initialize the LCD to be 16x2 (this is what I have, yours might be different)
    begin(16, 2, LCD_5x8DOTS);
    
    
    //--------------------------------------------------------------------------
    // 1 - Configure the A/D Module

    // * Configure analog pins, voltage reference and digital I/O 
    // AN0 is connected to wipe of left potentiometer (leftNumber)
    // AN0 is connected to wipe of right potentiometer (rightNumber))
    // Reference voltages are VSS and VDD
    ADCON1 = 0x0E;
    TRISAbits.RA0 = 1;
    TRISAbits.RA1 = 1;

    // * Select A/D acquisition time
    // * Select A/D conversion clock
    // Right justified, ACQT = 2 TAD, ADCS = FOSC/2
    ADCON2bits.ADCS = 0; // FOSC/2
    ADCON2bits.ACQT = 1; // ACQT = 2 TAD
    ADCON2bits.ADFM = 1; // Right justified

    // * Select A/D input channel
    ADCON0bits.CHS = 0; // Channel 0 (AN0)

    // * Turn on A/D module
    ADCON0bits.ADON = 1;   
    
    // 2 - Configure A/D interrupt (if desired)
    // * Clear ADIF bit
    // * Set ADIE bit
    // * Select interrupt priority ADIP bit
    // * Set GIE bit
    
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;
    IPR1bits.ADIP = 1;
    RCONbits.IPEN = 0; // disable priority levels
    INTCONbits.PEIE = 1; // enable peripheral interrupts
    INTCONbits.GIE = 1;
    
    // 3 - Wait the required acquisition time (if required)
    // ---> using ACQT = 2 TAD, no need to manually specify a wait      
    
    while(1){
        // 4- Start conversion: Set GO/DONE(bar) bit
        ADCON0bits.GO = 1;   
                
        // checking status of dip switches
        if (PORTDbits.RD4 == 0 && PORTDbits.RD3 == 0){  // clear - case 1
         clear();
         continue;
        }
        if (PORTDbits.RD4 == 1 && PORTDbits.RD3 == 0){    // addition - case 2
            ans = leftNum + rightNum;
            symbol = 0x2B;
        }
        if (PORTDbits.RD4 == 0 && PORTDbits.RD3 == 1){    // subtraction - case 3
            ans = leftNum - rightNum;
            symbol = 0x2D;
        }
        if (PORTDbits.RD4 == 1 && PORTDbits.RD3 == 1){    // multiplication - case 4
            ans = (leftNum * rightNum);
            //ans = 100000;
            symbol = 0x2A;
        }       
        home();  
        print_int(leftNum);     // show left number
        write(symbol);          // show symbol
        //print(" : ");
        print_int(rightNum);    // show right number
        print("       ");
        setCursor(0,1);        
        print_int(ans);         //print answer on next line
        print("           ");  
    }
    
    return (EXIT_SUCCESS);
}
void __interrupt() adc_sample(void)
{
    // test which interrupt called this interrupt service routine
    
    // ADC Interrupt
    if (PIR1bits.ADIF && PIE1bits.ADIE)
    {
        // 5 Wait for A/D conversion to complete by either
        // * Polling for the GO/Done bit to be cleared
        // * Waiting for the A/D interrupt
  
        // 6 - Read A/D result registers (ADRESH:ADRESL); clear bit ADIF, if required
        
        // reset the flag to avoid recursive interrupt calls
        PIR1bits.ADIF = 0;
        
        if (ADCON0bits.CHS == 0) // channel AN0 (left potentiometer)
        {
            leftNum = (ADRESH << 8) | ADRESL;
            leftNum = leftNum - 512;
            ADCON0bits.CHS = 1;
        }
        else if (ADCON0bits.CHS == 1) // channel AN1 (right potentiometer))
        {
            rightNum = (ADRESH << 8) | ADRESL;
            rightNum = rightNum - 512;
            ADCON0bits.CHS = 0;
        }
    }
}
