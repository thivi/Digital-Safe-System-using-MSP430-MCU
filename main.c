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
int j=0;
unsigned char data=0x08;
unsigned char motor=0x00;
unsigned char matrix=0x00;
unsigned char senddata=0x00;
int txc;
int i,j,b;
int dc=0;
int dowc=0;
int confirmsix=0;
int confirmpin=0;
int changepwd=0;
int pass=0;
int lockdoor=0;
int unlockdoor=0;
int reset=0;
int pwdset=0;
int apass=0;
unsigned char *Flash_ptr;
unsigned char *Flash_ptrC;
int f=0;
int locked=0;
int enter=0;
int row=0;
unsigned char key=' ';
unsigned char pin[6]={' ',' ',' ',' ',' ',' '};
unsigned char epin[6]={' ',' ',' ',' ',' ',' '};
unsigned char apin[6]={'9','5','1','3','5','7'};

int pin_index=0;
int pin_s_i=0;
//i2c
void i2c_init(){
    P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = 12;                             // fSCL = SMCLK/12 = ~100kHz
    UCB0BR1 = 0;

    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    IE2 |= UCB0TXIE;                          // Enable TX interrupt

}
void i2c_start(){
        UCB0CTL1 |= UCTR + UCTXSTT; //start condition
}

void send_data(){
    txc=1;
    senddata = data;
    while (UCB0CTL1 & UCTXSTP);
    i2c_start();
}
void send_datam(){
    txc=1;
    motor &= 0xF0;
    senddata &=0x0F;
    senddata |= motor;
    while (UCB0CTL1 & UCTXSTP);
    i2c_start();
}
void send_datama(unsigned char m){
    txc=1;
    m &= 0x0F;
    matrix &= 0xF0;
    matrix |= m;
    senddata = matrix;
    while (UCB0CTL1 & UCTXSTP);
    i2c_start();
}
void i2c_stop_chk(){
    while (UCB0CTL1 & UCTXSTP);
}
//
//lcd
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
void lcd_serial(){
    UCB0I2CSA = 0x3F;                         // Slave Address is 048h
}
void lcd_string(char *s){
    lcd_serial();
    while(*s){
        lcd_data(*s);
        s++;
}
    send_data();
}

void lcd_init(){
    lcd_serial();

    lcd_cmd(0x33);
    lcd_cmd(0x32);
    lcd_cmd(0x28); // 4 bit mode
    lcd_cmd(0x0F); // display on cursor on
    lcd_cmd(0x01); // clear the screen
    lcd_cmd(0x06); // increment cursor
    lcd_cmd(0x80); // row 1 column 1
}
//
//lock
void unlock(){
    UCB0I2CSA = 0x3E;                         // Slave Address is 048h

    int n;
    for(n=0;n<700;n++){
                motor &= 0x0F;
                motor |=BIT4 + BIT5;
                send_datam();
                delay(2);
                motor &= 0x0F;
                motor|=BIT5 + BIT6;
                send_datam();
                delay(2);
                motor &= 0x0F;
                motor|=BIT6 + BIT7;
                send_datam();
                delay(2);
                motor &= 0x0F;
                motor|=BIT7 + BIT4;
                send_datam();
                delay(2);
    }
}
void lock(){
    UCB0I2CSA = 0x3E;                         // Slave Address is 048h

    int n;
    for(n=0;n<700;n++){
                motor &= 0x0F;
                motor |=BIT7 + BIT6;
                send_datam();
                delay(2);
                motor &= 0x0F;
                motor|=BIT6 + BIT5;
                send_datam();
                delay(2);
                motor &= 0x0F;
                motor|=BIT5 + BIT4;
                send_datam();
                delay(2);
                motor &= 0x0F;
                motor|=BIT4 + BIT7;
                send_datam();
                delay(2);
    }
}
//
//keypad
void keypad(){

    P1DIR &= 0xF0;
    P1REN |= 0x0F;
    P1OUT &= 0xF0;
    P1IES &= 0xF0;
    P1IFG &= 0xF0;
    P1IE |=0x0F;

    send_datama(0xFF);

}
void key_init(){
    UCB0I2CSA = 0x3E;                         // Slave Address is 048h


}
void buzzer_init(){
    P1DIR |= BIT4;
    P1OUT |= BIT4;
}
void beep(){
    P1OUT &= ~BIT4;
    delay(4);
    P1OUT |= BIT4;
}
void right_tone(){
    for(b=0;b<2;b++){
        P1OUT &= ~BIT4;
        delay(100);
        P1OUT |= BIT4;
        delay(100);
    }
    P1OUT &= ~BIT4;
            delay(500);
            P1OUT |= BIT4;
}
void wrong_tone(){
    for(b=0;b<5;b++){
        P1OUT &= ~BIT4;
        delay(100);
        P1OUT |= BIT4;
        delay(100);
    }

}
void backspace(){
    lcd_serial();
    lcd_cmd(0x10);
    lcd_data(' ');
    lcd_cmd(0x10);
    lcd_cmd(0x00);
    pin_index--;
}
void print_key(unsigned char k){
    key=k;
    if(pin_index<6){
        pin[pin_index]=key;
        pin_index++;
        if(k=='1'){
            lcd_string("1");
        }
        else if (k=='2'){
            lcd_string("2");
        }
        else if (k=='3'){
            lcd_string("3");
        }
        else if (k=='4'){
            lcd_string("4");
        }
        else if (k=='5'){
            lcd_string("5");
        }
        else if (k=='6'){
            lcd_string("6");
        }
        else if (k=='7'){
            lcd_string("7");
        }
        else if (k=='8'){
            lcd_string("8");
        }
        else if (k=='9'){
            lcd_string("9");
        }
        else if (k=='0'){
            lcd_string("0");
        }

        send_data();
        beep();
    }
}
void key_cmd(unsigned char cmd){
    key=cmd;
    if(cmd=='B'){
        if(pin_index>0){
            backspace();
        }

    }
    else if (cmd=='A'){
        enter=1;
    }
    else if (cmd=='C'){
        lockdoor=1;
    }
    else if (cmd=='D'){
        unlockdoor=1;
    }
    else if (cmd=='#'){
        changepwd=1;
    }
    else if(cmd=='*'){
        reset=1;
    }
    beep();
}
void detect_key(){

    if((row==1)){
                key_init();

                send_datama(BIT0);
                i2c_stop_chk();
                delay(2);
                if ((P1IN & BIT0)==BIT0){
                    print_key('1');

                }
                key_init();
                send_datama(BIT1);
                i2c_stop_chk();
                delay(2);
                if((P1IN & BIT0)==BIT0){
                    print_key('2');

                }
                key_init();
                send_datama(BIT2);
                i2c_stop_chk();
                delay(2);
                if((P1IN & BIT0)==BIT0){
                    print_key('3');
                }
                key_init();
                send_datama(BIT3);
                i2c_stop_chk();
                delay(2);
                if((P1IN & BIT0)==BIT0){
                    key_cmd('A');
                }


                row=0;
                key_init();
                send_datama(0x0F);
                i2c_stop_chk();

            }
            else if(row==2){
                key_init();
                            send_datama(BIT0);
                            i2c_stop_chk();
                            delay(2);
                            if ((P1IN & BIT1)==BIT1){
                                print_key('4');
                            }
                            key_init();
                            send_datama(BIT1);
                            i2c_stop_chk();
                            delay(2);
                            if((P1IN & BIT1)==BIT1){
                                print_key('5');
                            }
                            key_init();
                            send_datama(BIT2);
                            i2c_stop_chk();
                            delay(2);
                            if((P1IN & BIT1)==BIT1){
                                print_key('6');
                            }
                            key_init();
                            send_datama(BIT3);
                            i2c_stop_chk();
                            delay(2);
                            if((P1IN & BIT1)==BIT1){
                                key_cmd('B');
                            }
                            row=0;
                                               key_init();
                                               send_datama(0x0F);
                                               i2c_stop_chk();

            }
            else if (row==3){
                key_init();
                            send_datama(BIT0);
                            i2c_stop_chk();
                            delay(2);
                            if ((P1IN & BIT2)==BIT2){
                                print_key('7');
                            }
                            key_init();
                            send_datama(BIT1);
                            i2c_stop_chk();
                            delay(2);
                            if((P1IN & BIT2)==BIT2){
                                print_key('8');
                            }
                            key_init();
                            send_datama(BIT2);
                            i2c_stop_chk();
                            delay(2);
                            if((P1IN & BIT2)==BIT2){
                                print_key('9');
                            }
                            key_init();
                            send_datama(BIT3);
                            i2c_stop_chk();
                            delay(2);
                            if((P1IN & BIT2)==BIT2){
                                key_cmd('C');
                            }
                            row=0;
                            key_init();
                            send_datama(0x0F);
                            i2c_stop_chk();

            }
            else if (row==4){
                key_init();
                            send_datama(BIT0);
                            i2c_stop_chk();
                            delay(2);
                            if ((P1IN & BIT3)==BIT3){
                                key_cmd('*');
                            }
                            key_init();
                            send_datama(BIT1);
                            i2c_stop_chk();
                            delay(2);
                            if((P1IN & BIT3)==BIT3){
                                print_key('0');
                            }
                            key_init();
                            send_datama(BIT2);
                            i2c_stop_chk();
                            delay(2);
                            if((P1IN & BIT3)==BIT3){
                                key_cmd('#');
                            }
                            key_init();
                            send_datama(BIT3);
                            i2c_stop_chk();
                            delay(2);
                            if((P1IN & BIT3)==BIT3){
                                key_cmd('D');
                            }

                            row=0;
                            key_init();
                            send_datama(0x0F);
                            i2c_stop_chk();


            }

}
void pin_enter(){
    lcd_serial();
    lcd_cmd(0x01);
    pin_index=0;
    lcd_string("Enter a PIN");

    lcd_cmd(0xC0);
    send_data();

    while(enter==0){

        detect_key();

    }
    if (enter==1){
        for (pin_s_i=0;pin_s_i<6;pin_s_i++){
            if (pin[pin_s_i]==' '){
                confirmsix=1;
            }
        }
        enter=0;
    }
}
void confirm_pin(){
    confirmpin=0;
    lcd_serial();
    lcd_cmd(0x01);
    pin_index=0;
    lcd_string("Confirm PIN");

    lcd_cmd(0xC0);
    send_data();

    while(enter==0){

       detect_key();

    }
    if (enter==1){
       for (pin_s_i=0;pin_s_i<6;pin_s_i++){
           if (epin[pin_s_i]!=pin[pin_s_i]){
               confirmpin=1;
               break;
           }
       }

    }
    enter=0;

}
void password(){

        pass=0;
        lcd_serial();
        lcd_cmd(0x01);
        pin_index=0;
        lcd_string("Enter the PIN!");

        lcd_cmd(0xC0);
        send_data();

        while(enter==0){

           detect_key();

        }
        if (enter==1){
           for (pin_s_i=0;pin_s_i<6;pin_s_i++){
               if (epin[pin_s_i]!=pin[pin_s_i]){
                   pass=1;
                   break;
               }
           }

        }
        enter=0;

}
void change_pwd(){
    pin_index=0;
    lcd_serial();
    lcd_cmd(0x01);
    lcd_string("Change PIN menu");
    send_data();
    delay(2000);
    lcd_cmd(0x01);
    lcd_string("Enter old PIN ");
    lcd_cmd(0xC0);
    delay(2000);
    send_data();

    password();
    while(pass==1){
                                    lcd_serial();
                                    lcd_cmd(0x01);
                                    lcd_string("PIN incorrect!");
                                    send_data();
                                    delay(2000);
                                    password();
                                }
                                if(pass==0){
                                    lcd_serial();
                                    lcd_cmd(0x01);
                                    lcd_string("PIN matched!");

                                    delay(2000);
                                    lcd_cmd(0x01);
                                    lcd_string("Enter new PIN");
                                    lcd_cmd(0xC0);

                                    pin_enter();
                                            while(confirmsix==1){
                                               pin_index=0;
                                               confirmsix=0;
                                               lcd_serial();
                                               lcd_cmd(0x01);
                                               send_data();
                                               lcd_string("Enter 6 digits!");
                                               delay(1000);
                                               pin_enter();

                                            }

                                            for(pin_s_i=0;pin_s_i<6;pin_s_i++){
                                                epin[pin_s_i]=pin[pin_s_i];
                                            }
                                            confirm_pin();
                                            while(confirmpin==1){
                                                lcd_serial();
                                                lcd_cmd(0x01);
                                                lcd_string("PINs mismatch!");
                                                delay(1000);
                                                pin_enter();
                                                while(confirmsix==1){
                                                    confirmsix=0;
                                                    lcd_serial();
                                                    lcd_cmd(0x01);
                                                    send_data();
                                                    lcd_string("Enter 6 digits!");
                                                    delay(1000);
                                                    pin_enter();

                                                 }
                                                 for(pin_s_i=0;pin_s_i<6;pin_s_i++){
                                                     epin[pin_s_i]=pin[pin_s_i];
                                                 }
                                                 confirm_pin();
                                             }

                                            if(confirmpin==0){
                                                write_flash();
                                                 lcd_serial();
                                                 lcd_cmd(0x01);
                                                 lcd_string("PIN saved!");
                                                 send_data();
                                                 pwdset=1;
                                                 delay(1000);
                                             }
                                }



}
void lock_door(){

    lockdoor=0;
    reset=0;
    while(locked==0){
        lcd_serial();
            lcd_cmd(0x01);
            lcd_string("Safe not locked");
            lcd_cmd(0xC0);
            lcd_string("Press C to lock!");
            send_data();
        pin_index=0;
        while (lockdoor==0 && changepwd==0 && reset==0){
            detect_key();

        }
        if(reset==1){
            erase_flash();
            break;

        }
        if(changepwd==1){
            change_pwd();
            changepwd=0;
        }
        while((P2IN & BIT1) != BIT1){

                    if(dc==0){
                        lcd_serial();
                        lcd_cmd(0x01);
                        lcd_string("Close door!");
                        wrong_tone();
                        send_data();
                    }
                    dc=1;
        }
        if(lockdoor==1){
            lcd_serial();
            lcd_cmd(0x01);
            lcd_string("Locking...........");
            send_data();
            lock();
            locked=1;
            lcd_serial();
            lcd_cmd(0x01);
            lcd_string("Locked!");

            send_data();
            right_tone();
            delay(1000);
        }
    }
}
void unlock_door(){
    unlockdoor=0;
    while(locked==1){
        lcd_serial();
                            lcd_cmd(0x01);
                            lcd_string("Safe locked!");
                            lcd_cmd(0xC0);
                            lcd_string("Push D to unlock");
                            send_data();
        pin_index=0;

        while(unlockdoor==0 && changepwd==0 && reset==0){
            detect_key();
        }
        if(changepwd==1){
            change_pwd();
            changepwd=0;
        }
        if(reset==1){
            erase_flash();
            reset=0;
            break;
        }
        if(unlockdoor==1){
        password();
                            while(pass==1){
                                lcd_serial();
                                lcd_cmd(0x01);
                                lcd_string("PIN incorrect!");

                                send_data();
                                wrong_tone();
                                delay(2000);
                                password();
                            }
                            if(pass==0){
                                lcd_serial();
                                lcd_cmd(0x01);
                                lcd_string("PIN matched!");
                                right_tone();
                                send_data();
                                delay(2000);
                                lcd_cmd(0x01);
                                lcd_string("Unlocking........");
                                send_data();
                                unlock();
                                lcd_serial();
                                lcd_cmd(0x01);
                                lcd_string("Unlocked!");
                                right_tone();
                                send_data();
                                locked=0;
                            }
        }
    }
}
void feedback_init(){
    P2DIR &= 0xFC;
    P2REN |= 0x03;
    P2OUT &= 0xFC;

}
void flash_init(){
    DCOCTL |= DCO2;
        BCSCTL1|= RSEL0+RSEL1+RSEL2;
        FCTL2 = FWKEY + FSSEL0 + FN1;
}
void erase_flash(){
    apass=0;
    while(apass==0){
    lcd_serial();
    lcd_cmd(0x01);
    lcd_string("Enter admin PIN");
    lcd_cmd(0xC0);
    send_data();
    while(enter==0){
        detect_key();
    }
    if(enter==1){
        for(i=0;i<6;i++){
            if(apin[i]!=pin[i]){
                apass=1;
                break;
            }
        }
        if(apass!=1){
            Flash_ptr = (unsigned char *) 0x1040;              // Initialize Flash pointer
            FCTL1 = FWKEY + ERASE;                    // Set Erase bit
            FCTL3 = FWKEY;                            // Clear Lock bit
            *Flash_ptr = 0;                           // Dummy write to erase Flash segment
            lcd_serial();
            lcd_cmd(0x01);

            lcd_string("Reset!");
            delay(2000);
            pwdset=0;
            apass=1;
        }
        else{
            apass=0;
            lcd_serial();
                  lcd_cmd(0x01);
                  lcd_string("PIN mismatch!");
                  delay(2000);
        }

    }
    enter=0;
    }

}
void write_flash(){

    Flash_ptr = (unsigned char *) 0x1040;              // Initialize Flash pointer
    FCTL1 = FWKEY + ERASE;                    // Set Erase bit
    FCTL3 = FWKEY;                            // Clear Lock bit
    *Flash_ptr = 0;                           // Dummy write to erase Flash segment
    FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
    for (f=0; f<7; f++)
    {
        if(f==0){
            *Flash_ptr++='P';
        }
        else{
            *Flash_ptr++ = epin[f-1];
        }// Write value to flash
    }

    FCTL1 = FWKEY;                            // Clear WRT bit
    FCTL3 = FWKEY + LOCK;                     // Set LOCK bit


}
void pwd_set(){
    Flash_ptrC = (unsigned char *) 0x1040;
    if (*Flash_ptrC=='P'){
        pwdset=1;
    }
    else{
        pwdset=0;
    }
}
void read_flash(){
    Flash_ptrC = (unsigned char *) 0x1040;
        *Flash_ptrC++;
          for (f=0; f<6; f++)
          {
                epin[f]=*Flash_ptrC++;
          }
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
      __enable_interrupt();
    i2c_init();
    lcd_init();
    key_init();
    flash_init();
    feedback_init();
    buzzer_init();
    keypad();
    pwd_set();
    if(pwdset==1){
        read_flash();
    }

    if((P2IN & BIT0) == BIT0){
        locked=1;
    }

    while(1){
    if(pwdset==0){
        pin_enter();
        while(confirmsix==1){
           pin_index=0;
           confirmsix=0;
           lcd_serial();
           lcd_cmd(0x01);
           send_data();
           lcd_string("Enter 6 digits!");
           wrong_tone();
           delay(1000);
           pin_enter();


        }

        for(pin_s_i=0;pin_s_i<6;pin_s_i++){
            epin[pin_s_i]=pin[pin_s_i];
        }
        confirm_pin();
        while(confirmpin==1){
            lcd_serial();
            lcd_cmd(0x01);
            lcd_string("PINs mismatch!");
            wrong_tone();
            delay(1000);
            pin_enter();
            while(confirmsix==1){
                confirmsix=0;
                lcd_serial();
                lcd_cmd(0x01);
                send_data();
                lcd_string("Enter 6 digits!");
                wrong_tone();
                delay(1000);
                pin_enter();

             }
             for(pin_s_i=0;pin_s_i<6;pin_s_i++){
                 epin[pin_s_i]=pin[pin_s_i];
             }
             confirm_pin();
         }

        if(confirmpin==0){
             write_flash();
             lcd_serial();
             lcd_cmd(0x01);
             lcd_string("PIN saved!");
             right_tone();
             send_data();
             pwdset=1;
             delay(1000);
         }
    }

    if(pwdset==1){
              if(locked==0){
                    while((P2IN & BIT1) != BIT1){

                        if(dc==0){
                            lcd_serial();
                            lcd_cmd(0x01);
                            lcd_string("Close door!");
                            wrong_tone();

                        }
                        dc=1;
                    }
                    dc=0;
                    lock_door();
                    if(reset==1){
                        reset=0;
                        continue;
                    }
                    if((P2IN & BIT0) != BIT0){
                        lcd_serial();
                        lcd_cmd(0x01);
                        lcd_string("Error! Door");
                        lcd_cmd(0xC0);
                        lcd_string("not locked!");
                        wrong_tone();
                        delay(2000);
                        lcd_cmd(0x01);
                        lcd_string("Unlock &");
                        lcd_cmd(0xC0);
                        lcd_string("Try again!");

                        delay(4000);

                    }
                }

                if (locked==1){

                    unlock_door();

                }
            }




    }


	return 0;
}



#pragma vector = PORT1_VECTOR
__interrupt void port1(void){
    if ((P1IFG & BIT0)==BIT0){
        row=1;
        delay(55);
    }
    else if ((P1IFG & BIT1)==BIT1){
        row=2;
        delay(55);
    }
    else if ((P1IFG & BIT2)==BIT2){
        row=3;
        delay(55);
    }
    else if ((P1IFG & BIT3)==BIT3){
        row=4;
        delay(55);
    }
    P1IFG &= 0xF0;
}
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void){
    if(txc==1){
        UCB0TXBUF=senddata;
        txc--;
    }
    else{
        UCB0CTL1 |= UCTXSTP;                       // I2C stop condition
        IFG2 &= ~UCB0TXIFG;
    }

}
