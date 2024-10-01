#include "program.h"


uint8_t RxData = 0; // Variable for UART module receive register, RXREG


// Delay x1.5us
void delay_x1o5us(uint8_t delay) {
    for(uint8_t i=0; i<delay; i++) NOP();
}

// Delay x24.25us
void delay_x24o25us(uint16_t delay) {
    for(uint16_t i=0; i<delay; i++) delay_x1o5us(15);
}

// Delay x1ms
void delay_ms(uint32_t delay) {
    for(uint32_t i=0; i<delay; i++) delay_x24o25us(41);
}


void programInitialize(void) {
    TRISBbits.TRISB6 = 0;
    TRISBbits.TRISB7 = 0;
    
    led1 = 0;
    led2 = 0;
    
    TRISBbits.TRISB5 = 0;
    TRISBbits.TRISB4 = 0;
    TRISBbits.TRISB3 = 0;
    TRISBbits.TRISB2 = 0;
    TRISBbits.TRISB1 = 0;
    TRISCbits.TRISC5 = 0;
    
    ANSELBbits.ANSB5 = 0;
    ANSELBbits.ANSB4 = 0;
    ANSELBbits.ANSB3 = 0;
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB1 = 0;
    
    RS_Pin = 0;
    E_Pin = 0;
    D4_Pin = 0;
    D5_Pin = 0;
    D6_Pin = 0;
    D7_Pin = 0;
    
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA2 = 1;
    TRISAbits.TRISA3 = 1;
    TRISAbits.TRISA4 = 1;
    
    ANSELAbits.ANSA1 = 0;
    ANSELAbits.ANSA2 = 0;
    ANSELAbits.ANSA3 = 0;
    ANSELAbits.ANSA4 = 0;
    
    // Tx pin
    TRISCbits.TRISC6 = 0;
    APFCONbits.TXSEL = 0;
    
    // Rx pin
    TRISCbits.TRISC7 = 1;
    APFCONbits.RXSEL = 0;
    
    lcd_Initialize();
    
    lcd_Goto(0, 0);
    lcd_PrintString("UART TxRx");
    
    uart_Initialize(_User_FOSC, 115200);
    
    uart_PrintString("UART Transceiver\n");
}

void programLoop(void) {
    if(!pb_Up) {
        uart_PrintString("Up button pressed\n");
        pb_DelayDebounce();
    }
    
    else if(!pb_Down) {
        uart_PrintString("Down button pressed\n");
        pb_DelayDebounce();
    }
    
    else if(!pb_Left) {
        uart_PrintString("Left button pressed\n");
        pb_DelayDebounce();
    }
    
    else if(!pb_Right) {
        uart_PrintString("Right button pressed\n");
        pb_DelayDebounce();
    }
    
    uart_ScanRxRegister();
}


// Delay to debounce mechanical noise
void pb_DelayDebounce(void) {
    delay_ms(200);
}

void lcd_DelaySetupTime(void) {
    // China TGK LCD delay
    delay_x1o5us(200);
}

void lcd_DelayPulse(void) {
    // China TGK LCD delay
    delay_x1o5us(200);
}

void lcd_EPulse(void) {
    E_Pin = 1;
    lcd_DelayPulse();
    
    E_Pin = 0;
    lcd_DelayPulse();
}

void lcd_WriteData(uint8_t data) {
    // Send upper nibble data
    D7_Pin = (data >> 7) & 0x01;
    D6_Pin = (data >> 6) & 0x01;
    D5_Pin = (data >> 5) & 0x01;
    D4_Pin = (data >> 4) & 0x01;
    
    lcd_EPulse();
    
    // Send lower nibble data
    D7_Pin = (data >> 3) & 0x01;
    D6_Pin = (data >> 2) & 0x01;
    D5_Pin = (data >> 1) & 0x01;
    D4_Pin = data & 0x01;
    
    lcd_EPulse();
    
    delay_x1o5us(30);   // Execution time for instruction >37us - Page 24
                        // delay = 37us / 1.5us = 25
}

void lcd_PrintCmd(uint8_t command) {
    RS_Pin = 0;
    lcd_DelaySetupTime();
    
    lcd_WriteData(command);
}

void lcd_Initialize(void) {
    delay_ms(20);           // Wait for LCD power supply rise time >10ms - Datasheet page 50
    
    // China TGK LCD reset process
    lcd_PrintCmd(0x33);
    lcd_PrintCmd(0x32);
    
    // LCD command - Datasheet page 24
    lcd_PrintCmd(0x28);     // Set LCD to 4-bit mode
    lcd_PrintCmd(0x02);     // Set DDRAM address counter to 0
    lcd_PrintCmd(0x0C);     // Display is set ON, cursor is set OFF, cursor blink is set OFF
    lcd_PrintCmd(0x06);     // Cursor is set to shift right
    lcd_PrintCmd(0x80);     // Set cursor back to home
    lcd_PrintCmd(0x01);     // Clear entire display
    
    delay_x24o25us(65);     // Execution time to clear display instruction, lcd_PrintCmd(0x01) >1.52ms,
                            // delay = 1.52ms / 24.25us = 63
}

void lcd_ClearAll(void) {
    lcd_PrintCmd(0x02);
    lcd_PrintCmd(0x01);
    delay_x24o25us(65);
}

void lcd_Goto(uint8_t y, uint8_t x) {
    switch(y) {
        case 0:
            lcd_PrintCmd(0x80 + x);
            break;
            
        case 1:
            lcd_PrintCmd(0xC0 + x);
            break;
            
        default:
            lcd_PrintCmd(0x80 + x);
            break;
    }
}

void lcd_PrintChar(char character) {
    RS_Pin = 1;
    lcd_DelaySetupTime();
    
    lcd_WriteData(character);
}

void lcd_PrintString(char *string) {
    while(*string!=0) {
        lcd_PrintChar(*string);
        string++;
    }
}

void lcd_PrintInt32(int32_t number) {
    uint8_t i1 = 0,
            i2 = 0,
            totalDigit = 0;
    
    char numberRevChar[11];
    char numberChar[11];
    
    memset(numberRevChar, 0, 11);
    memset(numberChar, 0, 11);
    
    if(number<0) {
        lcd_PrintChar('-');
        number = labs(number);
    }
    
    do {
        int32_t tempN = number;
        number /= 10;
        char tempC = (char)(tempN -10 * number);
        numberRevChar[i1] = tempC + 48;
        i1++;
    } while(number);
    
    totalDigit = i1;
    
    for(i1=totalDigit, i2=0; i1>0; i1--, i2++) {
        numberChar[i2] = numberRevChar[i1-1];
    }
    
    lcd_PrintString(numberChar);
}

void lcd_PrintDigitInt32(int32_t number, uint8_t noDigit, bool enSign, bool enZero) {
    uint8_t i1 = 0,
            i2 = 0,
            totalDigit = 0;
    
    char numberRevChar[11];
    char numberChar[11];
    
    memset(numberRevChar, 0, 11);
    memset(numberChar, 0, 11);
    
    if(number<0) {
        if(enSign) lcd_PrintChar('-');
        number = labs(number);
    } else {
        if(enSign) lcd_PrintChar(' ');
    }
    
    do {
        int32_t tempN = number;
        number /= 10;
        char tempC = (char)(tempN -10 * number);
        numberRevChar[i1] = tempC + 48;
        i1++;
    } while(number);
    
    totalDigit = i1;
    
    for(i1=0; i1<(noDigit-totalDigit); i1++) {
        if(enZero) lcd_PrintChar('0');
        else lcd_PrintChar(' ');
    }
    
    for(i1=totalDigit, i2=0; i1>0; i1--, i2++) {
        numberChar[i2] = numberRevChar[i1-1];
    }
    
    lcd_PrintString(numberChar);
}

void uart_Initialize(uint32_t fosc, uint32_t baudrate) {
    // Datasheet page 320
    TXSTAbits.CSRC = 0;     // UART module clock source generated internally from BRG
    TXSTAbits.TX9 = 0;      // Disable 9bit transmission
    TXSTAbits.TXEN = 1;     // Set enable UART module
    TXSTAbits.SYNC = 0;     // Set UART module to use asynchronous mode
    TXSTAbits.BRGH = 0;     // Set UART module to use low speed baud rate
    
    // Datasheet page 321
    RCSTAbits.SPEN = 1;     // Set enable serial port
    RCSTAbits.RX9 = 0;      // Set UART module to use 8bit reception mode
    RCSTAbits.CREN = 1;     // Set enable receiver
    
    BAUDCONbits.BRG16 = 1;  // Set UART module baud rate period to use 16bit register
    BAUDCONbits.ABDEN = 0;  // Set UART module not to use baud rate auto detect mode
    
    // Calculate baud rate generator, BRG period using formula, SPBRG = FOSC/[16(n+1)]
    // Refer to table 27-3: Baud Rate Formula - Page 324
    uint16_t brgPeriod = (uint16_t)(((fosc/baudrate)/16) - 1);
    
    SPBRGH = (uint8_t)(brgPeriod << 8);
    SPBRGL = (uint8_t)brgPeriod;
    
    delay_x1o5us(23); // Wait for UART to reconfigure BRG - Page 373 from I/O pin timing
}

void uart_PrintChar(char character) {
    TXREG = character;      // Set register value from argument character - Page 312
    while(!TXSTAbits.TRMT); // Polling to hold program to wait all byte in TXREG register is tranmitted
                            // Condition is true if TXREG if full - Page 320
}

void uart_PrintString(char *string) {
    while(*string!=0) {
        uart_PrintChar(*string);
        string++;
    }
}

void uart_PrintInt32(int32_t number) {
    uint8_t i1 = 0,
            i2 = 0,
            totalDigit = 0;
    
    char numberRevChar[11];
    char numberChar[11];
    
    memset(numberRevChar, 0, 11);
    memset(numberChar, 0, 11);
    
    if(number<0) {
        uart_PrintChar('-');
        number = labs(number);
    }
    
    do {
        int32_t tempN = number;
        number /= 10;
        char tempC = (char)(tempN -10 * number);
        numberRevChar[i1] = tempC + 48;
        i1++;
    } while(number);
    
    totalDigit = i1;
    
    for(i1=totalDigit, i2=0; i1>0; i1--, i2++) {
        numberChar[i2] = numberRevChar[i1-1];
    }
    
    uart_PrintString(numberChar);
}

void uart_ScanRxRegister(void) {
    // Single byte data receiver
    if(!BAUDCONbits.RCIDL) { // Start bit has been received - Page 322
        while(!PIR1bits.RCIF); // Polling to hold program to wait data filled into RCREG register
                               // Refer datasheet topic 27.1.2.2 Receiving Data in page 315 and register table page 318
        RxData = RCREG; // Write RCREG register into RxData - Page 318
        
        lcd_Goto(1, 0);
        lcd_PrintChar(RxData); // Display RxData on LCD
        
        // Condition to control LEDs using RxData received
        // Refer to ASCII table to use software Serial Monitor
        
        if(RxData=='1') {
            led1 = 1;
        } else if(RxData=='2'){
            led1 = 0;
        }
        
        else if(RxData=='3') {
            led2 = 1;
        } else if(RxData=='4'){
            led2 = 0;
        }
    }
}