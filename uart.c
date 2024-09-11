  /*
  * uart.c
  *
  * Created: 7/20/2024 9:33:51 PM
  *  Author: hop
  */ 

  #include "uart.h"

  // Kh?i t?o UART
  void uart_init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr>>8); // setting UBRRO Upper Byte
    UBRR0L = (unsigned char)ubrr;// setting UBRRO Lower Byte
    
    UCSR0B = (1<<RXEN0)|(1<<TXEN0); // Enable transmitter and Receiver
    UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); // setting 8 bit data
  }

  // G?i m?t byte qua UART
  void uart_transmit(unsigned char data) {
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;
  }

  void uart_transmits(unsigned int data)
  {
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1<<UDRE0)));
    /* Copy 9th bit to TXB8 */
    UCSR0B &= ~(1<<TXB80);
    if (data & 0x0100)
    UCSR0B |= (1<<TXB80);
    /* Put data into buffer, sends the data */
    UDR0 = data;
  }

  // G?i m?t chu?i qua UART
  void uart_transmit_string(char* str) {
    while(*str) {
      uart_transmit(*str);
      str++;
    }
  }

  unsigned char uart_receive(void) {

    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
  }

  unsigned int uart_receives( void )
  {
    unsigned char status, resh, resl;
    /* Wait for data to be received */
    while (!(UCSR0A & (1<<RXC0)))
    ;
    /* Get status and 9th bit, then data */
    /* from buffer */
    status = UCSR0A;
    resh = UCSR0B;
    resl = UDR0;

    /* If error, return -1 */
    if (status & ((1<<FE0)|(1<<DOR0)|(1<<UPE0)))
        return -1;
    /* Filter the 9th bit, then return */
    resh = (resh >> 1) & 0x01;
    return ((resh << 8) | resl);
  }

void uart_transmit_integer(int value) {
    char buffer[16]; // Buffer để chứa chuỗi số
    itoa(value, buffer, 10); // Chuyển đổi số thành chuỗi
    uart_transmit_string(buffer); // Gửi chuỗi
    uart_transmit('\n'); // Thêm ký tự xuống dòng
}