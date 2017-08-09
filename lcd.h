/*
 * lcd.h
 *
 *  Created on: May 10, 2017
 *      Author: thivi
 */

#include <msp430.h>

/*
 * main.c
 */
#define DATAW data |= BIT0 //use OR
#define CMDW data &= ~BIT0 //use AND
#define WRITE data &= ~BIT1 //use AND
#define READ data |= BIT1  //use OR
#define ENABLE data |= BIT2 //use OR
#define DISABLE data &= ~BIT2 //use AND


int i, j;

void delay(unsigned int k)
{
    for(j=0;j<=k;j++)
    {
       for(i=0;i<100;i++);
    }
}

void data_write(){
    ENABLE;
    send_data();
    delay(2);
    DISABLE;
}

void lcd_cmd(unsigned char cmd){
    WRITE;
    CMDW;
    data = (data &= 0xF)|(cmd & 0xF0); //send higher nibble;
    send_data();
    data_write();
    data = (data &= 0xF)|((cmd <<4) & 0xF0); //send lower nibble
    send_data();
    data_write();

}

void lcd_data(unsigned char txdata){
    WRITE;
    DATAW;
    data = (data &= 0x0F)|(txdata & 0xF0); //send higher nibble;
    send_data();
    data_write();
    data = (data &= 0x0F)|((txdata <<4) & 0xF0); //send lower nibble
    send_data();
    data_write();

}

void lcd_string(char *s){
    while(*s){
        lcd_data(*s);
        s++;
}
}
void lcd_init(){
    i2c_init();
    lcd_cmd(0x33);
    lcd_cmd(0x32);
    lcd_cmd(0x28); // 4 bit mode
    lcd_cmd(0x0F); // display on cursor on
    lcd_cmd(0x01); // clear the screen
    lcd_cmd(0x06); // increment cursor
    lcd_cmd(0x80); // row 1 column 1
}
//

