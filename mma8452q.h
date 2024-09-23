#ifndef MMA8452Q_H
#define MMA8452Q_H

#include <stdint.h>
#include <math.h>
#include "uart.h"

// Địa chỉ I2C của MMA8452Q
#define MMA8452Q_ADDR 0x1C

// Các thanh ghi của MMA8452Q
#define WHO_AM_I 0x0D
#define CTRL_REG1 0x2A
#define XYZ_DATA_CFG 0x0E
#define OUT_X_MSB 0x01
#define OUT_X_LSB 0x02
#define OUT_Y_MSB 0x03
#define OUT_Y_LSB 0x04
#define OUT_Z_MSB 0x05
#define OUT_Z_LSB 0x06

// Các thanh ghi ngắt của MMA8452Q
#define CTRL_REG2 0x2B
#define CTRL_REG3 0x2C
#define CTRL_REG4 0x2D
#define CTRL_REG5 0x2E

// Các hằng số cấu hình
#define ACTIVE_MODE 0x01
#define DATA_RATE_100HZ 0x00  // 100 Hz, normal mode
#define FULL_SCALE_2G 0x00    // ±2g range

// Các cấu hình ngắt
#define INT1_ACTIVE_HIGH 0x02  // Ngắt mức cao
#define INT1_PULSED 0x08      // Ngắt kiểu pulse
#define INT1_DRDY 0x01        // Ngắt khi có dữ liệu mới

// Ngưỡng ngắt, điều chỉnh giá trị này theo yêu cầu của bạn
#define PULSE_THSX 0x23
#define PULSE_THSY 0x24
#define PULSE_THSZ 0x25

#define G_SCALE 2.0      // Phạm vi ±2g của cảm biến
#define RAW_RESOLUTION 4096.0  // Độ phân giải 14-bit của cảm biến

#define SAMPLE_COUNT 10 // 5s / 0.5s


typedef uint8_t byte;

typedef struct {
    int angle_x;
    int angle_y;
    int angle_z;
} Angles;



void mma8452q_init(void);
uint16_t read_sensor_data(uint8_t msb_reg, uint8_t lsb_reg);
float raw_to_g(int16_t raw_value);
Angles calculate_angles(void);
void process_and_display_angles();

void process_and_display_angles_medium();

void init_interrupts(void);
void init_sleep_mode(void);
uint8_t is_button_pressed();
#endif
