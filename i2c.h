
#define DATAW data |= BIT0 //use OR
#define CMDW data &= ~BIT0 //use AND
#define WRITE data &= ~BIT1 //use AND
#define READ data |= BIT1  //use OR
#define ENABLE data |= BIT2 //use OR
#define DISABLE data &= ~BIT2 //use AND

unsigned char data=0x08;
unsigned char motor=0x00;
unsigned char senddata=0x00;
int txc;

//i2c
void i2c_init(){
    P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = 12;                             // fSCL = SMCLK/12 = ~100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = 0x3F;                         // Slave Address is 048h
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    IE2 |= UCB0TXIE;                          // Enable TX interrupt
    __enable_interrupt();
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
    senddata = motor;
    while (UCB0CTL1 & UCTXSTP);
    i2c_start();
}
//


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
