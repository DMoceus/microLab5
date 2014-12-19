/*
 * File:   newmain.c
 * Author: wkowalc1
 *
 * Created on October 7, 2014, 6:55 PM
 */

  #pragma config OSC = HSPLL
  #pragma config BOREN = OFF
  #pragma config WDT = OFF
  #pragma config LVP = OFF
  #pragma config MCLRE = ON
  #pragma config DEBUG = ON

#define BASE_EEPROM_ADDRESS 0x7F00

#include <stdio.h>
#include <stdlib.h>
#include "xc.h"
#include "header.h"
#include <string.h>

/*
 *
 */

unsigned char eepromRead(unsigned int address){
    LATAbits.LATA3 = 0;//clear chip select
    PIR1bits.SSP1IF = 0; //clear ssp1if
    SSP1BUF = 0b00000011;//assign instruction to sspbuf
    while(PIR1bits.SSP1IF == 0){}//poll ssp1if
    PIR1bits.SSP1IF = 0;//clear ssp1if
    SSP1BUF = address >> 8;//assign address MSB to sspbuf
    while(PIR1bits.SSP1IF == 0){}//poll sspif
    PIR1bits.SSP1IF = 0;//clear sspif
    SSP1BUF = address;//assign address LSB to sspbuf
    while(PIR1bits.SSP1IF == 0){}//poll ssp1if
    PIR1bits.SSP1IF = 0;//clear ssp1if
    SSP1BUF = 0;
    while(PIR1bits.SSP1IF == 0){}//poll sspif
    while(SSP1STATbits.BF == 0){} //poll sspbuf buffer full
    PIR1bits.SSP1IF = 0; //clear interupt flag
    unsigned char bufferNum = SSP1BUF; //copy value from the buffer
    LATAbits.LATA3 = 1; //set cs
    return bufferNum;
}

void eepromWrite(unsigned int address, unsigned char value){
//break into two command sequences
    //enable writing
        LATAbits.LATA3 = 0;//set cs to low (i wanna talk to the eeprom)
        SSP1BUF = 0b00000110;//enable writing via the write enable latch (wren)
        while(PIR1bits.SSP1IF == 0){}//poll sspif
        PIR1bits.SSP1IF = 0;//clear sspif
        LATAbits.LATA3 = 1;//set cs to low
    //write
        LATAbits.LATA3 = 0;//set cs to low
        SSP1BUF = 0b00000010;//send write command
        while(PIR1bits.SSP1IF == 0){}//poll sspif
        PIR1bits.SSP1IF = 0;//clear sspif
        SSP1BUF = address >> 8;//send msb of address
        while(PIR1bits.SSP1IF == 0){}//poll sspif
        PIR1bits.SSP1IF = 0;//clear sspif
        SSP1BUF = address;//send lsb of address
        while(PIR1bits.SSP1IF == 0){}//poll sspif
        PIR1bits.SSP1IF = 0;//clear sspif
        SSP1BUF = value;//send the value to write
        while(PIR1bits.SSP1IF == 0){}//poll sspif
        PIR1bits.SSP1IF = 0;//clear sspif
        LATAbits.LATA3 = 1;//set cs to high
        _delay(5);//brief pause (5ms)
}

//void errorEEPROM(unsigned char){}

void writeAddressToLCDInHex(unsigned int val){
    char x[256];
    sprintf(x, "%x", val);
    pic18_writeStringToLCD(x);
}

void writeAddressToLCDInDec(unsigned int val){
    char x[256];
    sprintf(x, "%d", val);
    pic18_writeStringToLCD(x);
}

void writeAddressToLCDInBin(unsigned int val){
    char x[256];
    char y[256];
    sprintf(y, "%s", itoa(x, val, 2));
    pic18_writeStringToLCD(y);
}

int main(int argc, char** argv) {

    //initialize LCD
    pic18_initializeLCD();

    //setup buttons and potentiometer
    TRISBbits.RB0 = 1;
    TRISAbits.RA5 = 1;
    TRISAbits.RA0 = 1;

    TRISDbits.RD0 = 1;
    TRISDbits.RD1 = 1;

    //ADCON1bits.PCFG ;//

    //Setting up the potentiometer
    ADCON0bits.CHS0 = 0;
    ADCON0bits.ADON = 1;
    ADCON2bits.ADFM = 1;

    //initialize EEPROM
    TRISAbits.RA3 = 0; //set port direction
    TRISCbits.RC4 = 1;
    TRISCbits.RC3 = 0;
    TRISCbits.RC5 = 0;
    LATAbits.LATA3 = 1; // cs
    SSP1CON1bits.SSPEN = 1;
    SSP1CON1bits.CKP = 0;
    SSP1CON1bits.SSPM = 0b0010;
    SSP1STATbits.SMP = 1;
    SSP1STATbits.CKE = 1;

    //enabling/configuring interrupts
    INTCONbits.GIE = 1;
    INTCON2bits.INTEDG0 = 0;
    INTCON2bits.INTEDG1 = 0;
    INTCONbits.INT0IF = 0;
    INTCONbits.INT0IE = 1;

    int leftButtonDown = 0;
    int rightButtonDown = 0;
    unsigned char currVal;

    int mode = 0;
    char temp[256];
    pic18_gotoColAndRowOnLCD(0,0);
    pic18_writeStringToLCD("12345");

    while(1){
		/*Pressing the right button once will lock the first line of the
         lcd and use the address in the first line to write to.
         Pressing the right button again will lock the second line of the
         lcd and use that value, and write it to the previously selected
         address.
         Pressing the right button one final time will actually write the
         value to that address in the eeprom.
         Pressing the left button at any point will reset the mode to 0.*/
        ADCON0bits.GO_NOT_DONE = 1;
        while(ADCON0bits.GO_NOT_DONE){ }
        
        /*While */
        while(PORTAbits.RA5 == 1){
            rightButtonDown = 1;
            //pic18_clearLineOnLCD(1);
            //pic18_gotoColAndRowOnLCD(0,0);
            //pic18_writeStringToLCD("12345");
        }
        while(PORTBbits.RB0 == 0){
            leftButtonDown = 1;
            //pic18_clearLineOnLCD(0);
            //pic18_gotoColAndRowOnLCD(0,1);
            //pic18_writeStringToLCD("67890");
        }
        if(rightButtonDown){
            if(mode == 0){
                _delay(1000);
                currVal = BASE_EEPROM_ADDRESS + (ADRES >> 2);
                mode = 1;
                
            }else{
                mode = 0;
            }
            rightButtonDown = 0;
        }
        else if(leftButtonDown){
            if(mode == 1){
                eepromWrite(currVal,BASE_EEPROM_ADDRESS + (ADRES >> 2));
                mode = 0;
            }
            leftButtonDown = 0;
        }
        if(mode == 0){
            LATDbits.LATD0 = 1;
            LATDbits.LATD1 = 0;
        }
        else if(mode == 1){
            LATDbits.LATD1 = 1;
            LATDbits.LATD0 = 0;
        }

        if(mode == 0){
            pic18_clearLineOnLCD(0);
            pic18_gotoColAndRowOnLCD(0,0);
            pic18_writeStringToLCD("0x");
            pic18_gotoColAndRowOnLCD(2, 0);
            writeAddressToLCDInHex(BASE_EEPROM_ADDRESS + (ADRES >> 2)); //write in hex
            pic18_gotoColAndRowOnLCD(8, 0);
            
            sprintf(temp,"%d",BASE_EEPROM_ADDRESS + (ADRES >> 2));
            sprintf(temp,"%c%c,%c%c%c",temp[0],temp[1],temp[2],temp[3],temp[4]);
            pic18_writeStringToLCD(temp);

            pic18_clearLineOnLCD(1);
            pic18_gotoColAndRowOnLCD(0,1);
            writeAddressToLCDInHex(eepromRead(BASE_EEPROM_ADDRESS + (ADRES >> 2)));
            pic18_gotoColAndRowOnLCD(3, 1);
            writeAddressToLCDInBin(eepromRead(BASE_EEPROM_ADDRESS + (ADRES >> 2)));
            pic18_gotoColAndRowOnLCD(12, 1);
            writeAddressToLCDInDec(eepromRead(BASE_EEPROM_ADDRESS + (ADRES >> 2)));
        }
        if(mode==1){
            pic18_clearLineOnLCD(1);
            pic18_gotoColAndRowOnLCD(0,1);
            writeAddressToLCDInHex(ADRES >> 2);
            pic18_gotoColAndRowOnLCD(3,1);
            writeAddressToLCDInBin(ADRES >> 2);
            pic18_gotoColAndRowOnLCD(12,1);
            writeAddressToLCDInDec(ADRES >> 2);
        }

  /*
        if(rightButtonDown){
            _delay(1000);
            rightButtonDown = 0;
            currVal = BASE_EEPROM_ADDRESS + (ADRES >> 2);
           /* while(rightButtonDown == 0){
                ADCON0bits.GO_NOT_DONE = 1;
                while(ADCON0bits.GO_NOT_DONE){ // polling the potentiometer
                }
                if(PORTBbits.RB0 == 1) {
                    leftButtonDown = 1;
                }
                if(leftButtonDown == 1) {
                    eepromWrite(currVal,BASE_EEPROM_ADDRESS + (ADRES >> 2));
                    leftButtonDown = 0;
                    rightButtonDown = 0;
                }
                pic18_clearLineOnLCD(1);
                pic18_gotoColAndRowOnLCD(0,1);
                pic18_writeStringToLCD("0x");
                pic18_gotoColAndRowOnLCD(2,1);
                writeAddressToLCDInHex(currVal));
                pic18_gotoColAndRowOnLCD(8,1);
                writeAddressToLCDInDec(currVal);
            }
            rightButtonDown = 0;
            leftButtonDown = 0;
        }
            //read/write to eeprom
        //update lcd based

        //check the eeprom datasheet for instructions for read/write/write enable

        //Write to the LCD
        pic18_clearLineOnLCD(0);
        pic18_gotoColAndRowOnLCD(0, 0);
        pic18_writeStringToLCD("0x");
        pic18_gotoColAndRowOnLCD(2, 0);
        writeAddressToLCDInHex(BASE_EEPROM_ADDRESS + (ADRES >> 2)); //write in hex

        pic18_gotoColAndRowOnLCD(8, 0);
        writeAddressToLCDInDec(BASE_EEPROM_ADDRESS + (ADRES >> 2)); //write in dec
        pic18_gotoColAndRowOnLCD(0, 1);
        writeAddressToLCDInHex(eepromRead(BASE_EEPROM_ADDRESS + (ADRES >> 2)));
        pic18_gotoColAndRowOnLCD(3, 1);
        writeAddressToLCDInBin(eepromRead(BASE_EEPROM_ADDRESS + (ADRES >> 2)));
        pic18_gotoColAndRowOnLCD(12, 1);
        writeAddressToLCDInDec(eepromRead(BASE_EEPROM_ADDRESS + (ADRES >> 2)));*/
    }

    return (EXIT_SUCCESS);
}
