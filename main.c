/*
 * DA_TTNT.c
 *
 * Created: 7/11/2024 3:10:40 PM
 * Author : hop
 */ 
#include "main.h"

ISR(INT0_vect) {
    // Xử lý ngắt từ MMA8452Q (INT1)
    // process_and_display_angles();
    // _delay_ms(500);
    PORTB |= (1 << PB1); // Bật PB1 lên khi ngắt xảy ra
    _delay_ms(500); // Giữ PB1 bật trong 3 giây
    PORTB &= ~(1 << PB1); // Tắt PB1
}

int main(void)
{
	i2c_init();
	ssd1306_init();
  mma8452q_init();
  ssd1306_display_begin();
	setup();
  //init_interrupts();
  //init_sleep_mode();
  //sei();

	DDRB |= (1<<PB0);
	DDRB |= (1<<PB1);

    while (1) 
    {

    process_and_display_angles();
    }
	return 0;
}

