/*
 * uart.h
 *
 * Created: 7/20/2024 9:33:37 PM
 *  Author: hop
 */ 


#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <string.h>
//#include <stdio.h>

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

void uart_init(unsigned int ubrr);
void uart_transmit(unsigned char data);
void uart_transmits(unsigned int data);
void uart_transmit_string(char* str);

unsigned char uart_receive(void);
unsigned int uart_receives(void);

void uart_transmit_integer(int value);


#endif /* UART_H_ */