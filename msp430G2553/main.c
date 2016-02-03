#include <msp430.h>

/*****************************************************************
 * ��������
 **************************************************************/
long temp;
long IntDegC;


/*************************************************************
 * ��ʼ������
 * ***********************************************************/

/**************************************************************
 * ��������uart_init
 * ��������
 * ���ܣ���ʼ��UART
 * �汾��V1.0
 */
void uart_init(void)
{
	P1SEL = BIT1 + BIT2;			//IO��ģʽѡ��
	P1SEL2 = BIT1 + BIT2;

	UCA0CTL1 = UCSWRST;				//��λUART

	UCA0CTL1 = UCSSEL1;				//ʱ��Դѡ��

	UCA0BR0 = 104;					//����������9600
	UCA0BR1 = 0;

	UCA0MCTL = UCBRS_1;				//BITCLK �ĵ���ģʽ1

	UCA0CTL1 &= ~UCSWRST;			//ʹ��UART
	IE2 = UCA0RXIE;					//ʹ���ж�
}

/****************************************************************
 * ��������ADC10_init
 * ��������
 * ���ܣ���ʼ��ADC10
 * �汾��V1.0
 */
void ADC10_init(void){

	int i = 5000;

	ADC10CTL1 = INCH_10 + ADC10DIV_3;
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10IE + ADC10ON;

	while(i--);

}

/****************************************************************
 * ��������main����
 * ���ܣ�
 * ��������
 * �汾��V1.0
 * main.c
 */
int main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    /*����ʱ�����ã�����DCOΪ1M��
     * ������ڴ��ַ�л�ȡУ��������ò�����
     * CALBC1_1MHZ������MSP4302553.cmd��
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
    	 __bis_SR_register(LPM0_bits+GIE);		//����LPM0ģʽ,���жϡ�

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
 * ��������UART�жϺ���
 * ���ܣ�����UART�ж�����
 * ��������
 * ���ñ����� CCS, IAR, GNUC
 * �汾:v1.0
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
 * ��������ADC10_ISR
 * ��������
 * ���ܣ�ADC10�жϴ�����
 * �汾��V1.0
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

