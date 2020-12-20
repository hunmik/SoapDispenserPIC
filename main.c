/* 
 * File:   main.c
 * Author: hunmik
 *
 * Created on 2020. december 5., 15:09
 */

#include <stdio.h>
#include <stdlib.h>
#include <pic16f677.h>

// CONFIG
#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select bit (MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Selection bits (BOR enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

#define _XTAL_FREQ 4000000 //default is 4MHz
#define hand RA2
#define door_open RA3
#define wheel_mid RA4 // wheel is in middle (not parking)
#define RedLED RA5
#define GreenLED RC7
#define MotorFW RB4
#define MotorRW RB5
#define Battery_voltage RA0


#define IO_input 1
#define IO_output 0
#define Digital_IO 0
#define Analog_IO 1
#define ON 1
#define OFF 0
#define LED_OFF 1
#define LED_ON 0
/*
 * 
 */
//COnstants
const int MotorONMaxTicks = 300; //300*10ms 3 sec
const int FlashLEDOnTicks = 5;//5*10ms ;
const int FlashLEDOffTicks = 500 ;//500*10ms;
const int Timer1Count = 1250;
const int AlarmVoltage = 689; //ADC reading for alarm 
//5.1V alarm voltage requied, with a 0.475 resistor divider it is 2.422V
//Supply voltage is 3.6V, therefore 2.422/3.6*1024 =689

//Variables
int FlashLEDTicks; //Timer counter for flasing led
bit FlashLEDbit; //If ON LED will be ON
int MotorONTicks; //Timer counter for motor ON
bit MotorError; //if motor runs too long

//Declarations
void Init_PIC(void);
void interrupt InterruptHandler(void);



//main
int main(int argc, char** argv) {
    Init_PIC();
    char i;
           for (i = 1; i < 4; ++i){  
            GreenLED=LED_ON;
            __delay_ms(100);
            GreenLED=LED_OFF;
             __delay_ms(100);
           }
    //set initial position, should have effect only after disassembly
                MotorRW=ON;
                while (!wheel_mid) {}; 
                MotorRW=OFF;
                 
    static bit wheel_prev;      //stores previous value of wheel position
    while (1){
        
        if (hand){
            //Flash led
            if (door_open){ //door_open
                RedLED=1;
                __delay_ms(50);
                RedLED=0;
                __delay_ms(50);
             //activate pump   
            }else{
                MotorRW=ON;
                //go to start of wheel slot (should be already there, though)
                do {
                    wheel_prev=wheel_mid;
                    __delay_ms(2);
                }
                while(!wheel_mid&&!wheel_prev);
                
                 __delay_ms(1);
                 
                // go until end of slot 
                do {
                    wheel_prev=wheel_mid;
                    __delay_ms(2);
                }
                while(wheel_mid&&wheel_prev);
                  __delay_ms(2);
                  
                // go to start of wheel slot
                do {
                    wheel_prev=wheel_mid;
                    __delay_ms(2);
                }
                while(!wheel_mid&&!wheel_prev);
                   __delay_ms(8);
                MotorRW=OFF;
            }
        }
        
        //if voltage low, flash RED, of OK, flash GREEN
        int voltage;
        voltage=ADRESH <<8 | ADRESL;
        if (voltage<AlarmVoltage){
            GreenLED=LED_OFF;
            RedLED=!FlashLEDbit;
        }else{
            RedLED=LED_OFF;
            GreenLED=!FlashLEDbit;
        }
        
        //motor error requires repair
        if (MotorError)
            while (1){
                RedLED=LED_ON;
                __delay_ms(50);
                RedLED=LED_OFF;
                __delay_ms(50);
                 RedLED=LED_ON;
                __delay_ms(50);
                RedLED=LED_OFF;
                __delay_ms(500);
                CLRWDT();
            }
       
    }
    return (EXIT_SUCCESS);
}

void Init_PIC(void){
    //WPUA=0;
    //RA0
    ANS0=Analog_IO; //RA2
    //RA2 
    ANS2=Digital_IO; //RA2
    TRISA2=IO_input;
    //RA 3
    TRISA2=IO_input;
    //RA 4
    TRISA4=IO_input;
    ANS3=Digital_IO; //RA4
    //RA 5
    TRISA5=IO_output;
    //RB 4
    TRISB4=IO_output;
    ANS10=Digital_IO; //RB4
    //RB 5
    TRISB5=IO_output;
    ANS11=Digital_IO; //RB5
    //RC7
    ANS9=Digital_IO; //RC6
    TRISC7=IO_output;
    
    MotorFW=OFF;
    MotorRW=OFF;
    
    GreenLED=1;
    RedLED=1;
    
   //Configure Timer 1
    // Timer 1 is running from fosc/4 :4Mhz/4= 1Mhz => 1us. with 1:8 prescaler => 1*8= 8 us
   //so the 16bit Timer1 is decreased at every 8us, 
   //to get 10ms interrupt we have to set TMR1 to: 10000us/8us=1250 

    //Timer 1 interrupt generated in 65535x4us == 262ms
    T1CKPS1=1; //set 1:8 prescaler
    T1CKPS0=1;
    TMR1CS=0; //internal clock from fosc/4
    T1CON=0; //must be 0 to be able to write TMR1 in normal way
    TMR1ON=1;// sw on Timer1
    //Timer and periferial interrupts are enabled
    TMR1IE=1; //enable timer interrupt
    PEIE=1; //Periferial  interrupts enabled 


    //Configure Watchdog Timer
    PSA=0; //disable tmr0 prescaler for WDT
    CLRWDT(); //clear watchdog timer
   // WDTCON=0b10111; //wdt prescaler 65k, WDT ON
 
    
    //ADC settings
    //by default RA0 is selected as ADC input
    //by default reference is Vdd
    ADCON1bits.ADCS=0b101;     //Set conversion time to fosc/14: appr. 50us conversion cycle 
    ADCON0bits.ADFM=1;//select AD output right justified format
    
    
    FlashLEDbit=OFF;
    FlashLEDTicks=FlashLEDOffTicks;
    MotorError=0;
    MotorONTicks=0;

    GIE=1; //General interrupts enabled
}

//functions
void interrupt InterruptHandler(void){

    if (TMR1IE && TMR1IF) {
        TMR1ON=0;
        TMR1=0xFFFF-Timer1Count;
        //write to TMR1 clears prescaler!
        T1CKPS1=1; //set 1:8 prescaler 
        T1CKPS0=1;
        TMR1ON=1;
        TMR1IF = 0;
  
        if (FlashLEDTicks--==0){ 
            if (FlashLEDbit==ON){
               FlashLEDbit=OFF;
               FlashLEDTicks=FlashLEDOffTicks;
               //start ADC
               ADON=1;
               ADCON0bits.GO=1;
            }else{
               FlashLEDbit=ON;
               FlashLEDTicks=FlashLEDOnTicks;

            }
        }
        //if motor runs too long, set error
        if (MotorRW==ON){
            if (MotorONTicks++>MotorONMaxTicks){
                MotorRW=OFF;
                MotorError=1;
            }
        }else{
            MotorONTicks=0;
        }
                
        
    CLRWDT(); //clear watchdog timer
    }

}