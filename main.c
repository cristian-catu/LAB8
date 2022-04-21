/* 
 * File:   main.c
 * Author: Cristian Catú
 *
 * Potenciometro en AN0, ADRESH -> PORTC, ADRESL -> PORTD
 * 
 * Created on 15 april 2022, 21:31
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#define _XTAL_FREQ 4000000
#include "setup.h"

int8_t i = 1; //se establecen todas las variables a utilizar
int8_t unidades = 0;
int8_t decenas = 0;
int8_t centenas = 0;
int8_t unidades2 = 0;
int8_t decenas2 = 0;
int8_t centenas2 = 0;
int8_t decenas3 = 0;
int8_t centenas3 = 0;
int8_t display_unidades = 0;
int8_t display_decenas = 0;
int8_t display_centenas = 0;
uint8_t valor = 0;
int8_t remainder = 0;

void __interrupt() isr (void){
    if(PIR1bits.ADIF){              // Fue interrupción del ADC?
        if(ADCON0bits.CHS == 0){    // Verificamos sea AN0 el canal seleccionado
            PORTB = ADRESH;         // Mostramos ADRESH en PORTC
        }
        else if (ADCON0bits.CHS == 1){
            valor = ADRESH;
            centenas2 = valor/100; //Se obtienen las centenas al divir dentro de 100
            remainder = valor%100;
            decenas2 = remainder/10; //Se obtienen las decenas al divir el remainder dentro de 10
            unidades2 = remainder%10;  //Se obtienen las unidades al sacar el remainder del remainder
            centenas3 = 0;
            decenas3 = 0;
            unidades = unidades2*2;
            if (unidades > 9){
                unidades = unidades - 10;
                decenas3 = 1;
            }
            decenas = decenas2*2;
            decenas = decenas + decenas3;
            if (decenas > 9){
                decenas = decenas - 10;
                centenas3 = 1;
            }
            centenas = centenas2*2;
            centenas = centenas + centenas3;
            if (valor > 250){
                unidades = 0;
                decenas = 0;
                centenas = 5;
            }
        }
        PIR1bits.ADIF = 0;          // Limpiamos bandera de interrupción
    }
    if(INTCONbits.T0IF){  //interrupción cada 2ms
        switch(i)         //se hacen los estados para el multiplexado
        {        
            case 1:  //mostramos centenas en display 1
                PORTD = 0b00000001;
                PORTC = display_centenas;
                i = 2;
                break;
            case 2: //mostramos decenas en display 2
                PORTD = 0b00000010;
                PORTC = display_decenas;
                i = 3;
                break;
            case 3:  //mostramos unidades en display 2
                PORTD = 0b00000100;
                PORTC = display_unidades;
                i = 1;
                break;
        }
        INTCONbits.T0IF = 0; //se limpia la bandera del TMR0
        TMR0 = 248; //se pone el valor inicial del TMR0
    }
    return;
}

void main(void) {
    setup();
    int tabla[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111}; //Tabla para mostrar el valor en el display
    while(1){
        PORTCbits.RC7 = 1;
        if(ADCON0bits.GO == 0){             // No hay proceso de conversion
            // *usando mas de un canal analógico
            if(ADCON0bits.CHS == 0b0000)    
                ADCON0bits.CHS = 0b0001;    // Cambio de canal
            else if(ADCON0bits.CHS == 0b0001)
                ADCON0bits.CHS = 0b0000;    // Cambio de canal
            __delay_us(40);                 // Tiempo de adquisición
            ADCON0bits.GO = 1;              // Iniciamos proceso de conversión
        }
        
        display_unidades = tabla[unidades]; //llamamos a la tabla para obener los valores a poner en el display de 7-seg
        display_decenas = tabla[decenas];
        display_centenas = tabla[centenas];
    }
    return;
}