#include <msp430.h>

/*****************************************************************
 * 变量定义
 **************************************************************/
long temp;
long IntDegC;


/*************************************************************
 * 初始化函数
 * ***********************************************************/

/**************************************************************
 * 函数名：uart_init
 * 参数：无
 * 功能：初始化UART
 * 版本：V1.0
 */
void uart_init(void)
{
	P1SEL = BIT1 + BIT2;			//IO口模式选择
	P1SEL2 = BIT1 + BIT2;

	UCA0CTL1 = UCSWRST;				//复位UART

	UCA0CTL1 = UCSSEL1;				//时钟源选择

	UCA0BR0 = 104;					//波特率设置9600
	UCA0BR1 = 0;

	UCA0MCTL = UCBRS_1;				//BITCLK 的调制模式1

	UCA0CTL1 &= ~UCSWRST;			//使能UART
	IE2 = UCA0RXIE;					//使能中断
}

/****************************************************************
 * 函数名：ADC10_init
 * 参数：无
 * 功能：初始化ADC10
 * 版本：V1.0
 */
void ADC10_init(void){

	int i = 5000;

	ADC10CTL1 = INCH_10 + ADC10DIV_3;
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10IE + ADC10ON;

	while(i--);

}

/****************************************************************
 * 函数名：main函数
 * 功能：
 * 参数：无
 * 版本：V1.0
 * main.c
 */
int main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    /*基础时钟配置，设置DCO为1M，
     * 从相关内存地址中获取校正后的配置参数，
     * CALBC1_1MHZ定义在MSP4302553.cmd。
     */
    if(CALBC1_1MHZ == 0xFF){
    	while(1);
    }
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P1DIR = 0X01;

    uart_init();
    ADC10_init();


    while(1){
    	 ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
    	 __bis_SR_register(LPM0_bits+GIE);		//进入LPM0模式,开中断。

    	 // oC = ((A10/1024)*1500mV)-986mV)*1/3.55mV = A10*423/1024 - 278
    	 temp = ADC10MEM;

    	// while (!(IFG2&UCA0TXIFG));
    	  //UCA0TXBUF = temp;

    	 __no_operation();
    	 __no_operation();

    	 IntDegC = ((temp - 673) * 423) / 1024;

    	 while (!(IFG2&UCA0TXIFG));
    	 UCA0TXBUF =  IntDegC;
    	 __no_operation();                       // SET BREAKPOINT HERE
    }


	//return 0;
}

/**************************************************************
 * 函数名：UART中断函数
 * 功能：处理UART中断事务
 * 参数：无
 * 适用编译器 CCS, IAR, GNUC
 * 版本:v1.0
 */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
  while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
  UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character
  P1OUT ^= 0x01;                          // Toggle P1.0 using exclusive-OR
}

/*******************************************************************
 * 函数名：ADC10_ISR
 * 参数：无
 * 功能：ADC10中断处理函数
 * 版本：V1.0
 */
// ADC10 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC10_VECTOR))) ADC10_ISR (void)
#else
#error Compiler not supported!
#endif
{
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

