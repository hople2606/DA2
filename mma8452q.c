#include "mma8452q.h"
#include "i2c.h"

// Khai báo các biến để lưu giá trị offset
int offset_x = 0;
int offset_y = 0;
int offset_z = 0;
uint8_t reference_set = 0; // Cờ để lưu trạng thái hệ quy chiếu

// Khởi tạo MMA8452Q
void mma8452q_init(void) {
    // Cấu hình thanh ghi XYZ_DATA_CFG để thiết lập phạm vi đo ±2g
    i2c_writeToAddress(MMA8452Q_ADDR, XYZ_DATA_CFG, FULL_SCALE_2G);
    _delay_ms(10); // Chờ một chút để cấu hình được thiết lập

    // Cấu hình thanh ghi CTRL_REG1 để thiết lập chế độ hoạt động và tốc độ lấy mẫu
    uint8_t ctrl_reg1_value = ACTIVE_MODE | DATA_RATE_100HZ;
    i2c_writeToAddress(MMA8452Q_ADDR, CTRL_REG1, ctrl_reg1_value);
    _delay_ms(10); // Chờ một chút để cảm biến ổn định

    // Cấu hình thanh ghi CTRL_REG3 để thiết lập ngắt trên chân INT1
    uint8_t ctrl_reg3_value = INT1_ACTIVE_HIGH | INT1_PULSED;
    i2c_writeToAddress(MMA8452Q_ADDR, CTRL_REG3, ctrl_reg3_value);
    _delay_ms(10);

    // Cấu hình thanh ghi CTRL_REG4 để kích hoạt các ngắt
    uint8_t ctrl_reg4_value = INT1_PULSED;
    i2c_writeToAddress(MMA8452Q_ADDR, CTRL_REG4, ctrl_reg4_value);
    _delay_ms(10);

    // Cấu hình thanh ghi CTRL_REG5 để gửi ngắt đến chân INT1
    uint8_t ctrl_reg5_value = INT1_DRDY;
    i2c_writeToAddress(MMA8452Q_ADDR, CTRL_REG5, ctrl_reg5_value);
    _delay_ms(10);

    // Thiết lập ngưỡng kích hoạt
    // Ví dụ: Thiết lập ngưỡng ngắt với giá trị giả định cho PULSE_THSX, PULSE_THSY, PULSE_THSZ
    // Ngưỡng cần được điều chỉnh dựa trên giá trị gia tốc tính toán từ góc dịch chuyển mong muốn
// Thiết lập ngưỡng kích hoạt
    float angle_threshold = 10.0f; // Độ nghiêng tối thiểu để kích hoạt ngắt
    float sin_angle = sin(angle_threshold * 3.14159265f / 180.0f);
    float g = 2.0; // ±2g, phạm vi gia tốc
    float resolution = 1.0f / 256.0f; // Độ phân giải 8-bit của cảm biến

    uint8_t pulse_thresh_value = (uint8_t)(sin_angle * g / resolution);
    if (pulse_thresh_value > 255) pulse_thresh_value = 255; // Giới hạn trong phạm vi 8 bit
    
    i2c_writeToAddress(MMA8452Q_ADDR, PULSE_THSX, pulse_thresh_value);
    i2c_writeToAddress(MMA8452Q_ADDR, PULSE_THSY, pulse_thresh_value);
    _delay_ms(10);

    DDRD &= ~(1 << PD7);   // Thiết lập PB2 làm đầu vào
    PORTD |= (1 << PD7);   // Kích hoạt pull-up nội
    // Cấu hình các thanh ghi khác nếu cần thiết
}

uint16_t read_sensor_data(uint8_t msb_reg, uint8_t lsb_reg) {
    uint8_t msb = i2c_readFromAddress(MMA8452Q_ADDR, msb_reg);
    uint8_t lsb = i2c_readFromAddress(MMA8452Q_ADDR, lsb_reg);
    return ((uint16_t)msb << 8) | lsb;
}

// Chuyển đổi giá trị raw thành giá trị gia tốc (g)
float raw_to_g(int16_t raw_value) {
    float g_value = (raw_value / 4096.0f) * 2;
    return g_value;
}

Angles calculate_angles(void) {
    int x_data_raw = read_sensor_data(OUT_X_MSB, OUT_X_LSB);
    int y_data_raw = read_sensor_data(OUT_Y_MSB, OUT_Y_LSB);
    int z_data_raw = read_sensor_data(OUT_Z_MSB, OUT_Z_LSB);

    float ax = raw_to_g(x_data_raw);
    float ay = raw_to_g(y_data_raw);
    float az = raw_to_g(z_data_raw);

    Angles angles;
    angles.angle_x = atan2(ax, sqrt(ay * ay + az * az)) * 180.0f / 3.14159265f;
    angles.angle_y = atan2(ay, sqrt(ax * ax + az * az)) * 180.0f / 3.14159265f;
    angles.angle_z = atan2(az, sqrt(ax * ax + ay * ay)) * 180.0f / 3.14159265f;

    return angles;
}

void process_and_display_angles() {
    char buffer[16];
    Angles angles = calculate_angles();

    //--------------------------------------//
// Kiểm tra nếu nút được nhấn và hệ quy chiếu chưa được thiết lập
    if (is_button_pressed() && !reference_set) {
        // Thiết lập hệ quy chiếu
        offset_x = angles.angle_x;
        offset_y = angles.angle_y;
        offset_z = angles.angle_z;
        reference_set = 1; // Đánh dấu rằng hệ quy chiếu đã được thiết lập
    }

    // Điều chỉnh giá trị góc với offset
    if (reference_set) {
        angles.angle_x -= offset_x;
        angles.angle_y -= offset_y;
        angles.angle_z -= offset_z;
    }
//     //--------------------------------------//
    ssd1306_clear_line(4);
    ssd1306_clear_line(5);
    ssd1306_clear_line(6);

    // Nếu có dữ liệu từ UART

    if (abs(angles.angle_x) < 20 && abs(angles.angle_y) < 20) {
        PORTB |= (1 << PB0);
        PORTB &= ~(1 << PB1);
        /*if (abs(angles.angle_x) > abs(angles.angle_y) ){
        //uart_transmit_string("Trasmitted: ");
        uart_transmit_integer(abs(angles.angle_x));
        } else {
        //uart_transmit_string("Trasmitted: ");
        uart_transmit_integer(abs(angles.angle_y));
        }*/
        _delay_ms(100);
    } else if (abs(angles.angle_x) > 20 || abs(angles.angle_y) > 20) {
        PORTB &= ~(1 << PB0);
        PORTB |= (1 << PB1);
        /*if (abs(angles.angle_x) > 20 ){
        //uart_transmit_string("Trasmitted: ");
        uart_transmit_integer(abs(angles.angle_x));
        } else {
        //uart_transmit_string("Trasmitted: ");
        uart_transmit_integer(abs(angles.angle_y));
        }*/
        _delay_ms(100);
    }

    snprintf(buffer, sizeof(buffer), "X: %d", angles.angle_x);
    ssd1306_display_string(0, 4, buffer);

    snprintf(buffer, sizeof(buffer), "Y: %d", angles.angle_y);
    ssd1306_display_string(0, 5, buffer);

    snprintf(buffer, sizeof(buffer), "Z: %d", angles.angle_z);
    ssd1306_display_string(0, 6, buffer);
    
    _delay_ms(5000); // Đợi 5 giây
}

void init_interrupts(void) {
    // Cấu hình INT0 để kích hoạt khi có sự thay đổi từ mức cao xuống thấp (falling edge)
    EICRA |= (1 << ISC01); // Kích hoạt ngắt INT0 khi có cạnh xuống
    EICRA &= ~(1 << ISC00);
    EIMSK |= (1 << INT0);  // Kích hoạt INT0
}

void init_sleep_mode(void) {
    SMCR = (1 << SM1) | (1 << SE);  // Chế độ ngủ sâu
}

uint8_t is_button_pressed() {
    // Giả sử PB2 sẽ ở mức thấp khi nhấn nút
    if (!(PINC & (1 << PC6))) {
        _delay_ms(50); // Debounce
        if (!(PINC & (1 << PC6))) {
            return 1;
        }
    }
    return 0;
}


void process_and_display_angles_medium() {
    char buffer[16];
    //Angles angles = calculate_angles();
    Angles angles;
    int sum_x = 0, sum_y = 0, sum_z = 0;
    int count = 0;

    //--------------------------------------//
// Kiểm tra nếu nút được nhấn và hệ quy chiếu chưa được thiết lập
    if (is_button_pressed() && !reference_set) {
        // Thiết lập hệ quy chiếu
        angles = calculate_angles();
        offset_x = angles.angle_x;
        offset_y = angles.angle_y;
        offset_z = angles.angle_z;
        reference_set = 1; // Đánh dấu rằng hệ quy chiếu đã được thiết lập
    }

    // Điều chỉnh giá trị góc với offset
    if (reference_set) {
        for (int i = 0; i < 5; i++) {
            angles = calculate_angles();

            // Điều chỉnh giá trị góc với offset
            angles.angle_x -= offset_x;
            angles.angle_y -= offset_y;
            angles.angle_z -= offset_z;

            // Tính tổng giá trị góc
            sum_x += abs(angles.angle_x);
            sum_y += abs(angles.angle_y);
            sum_z += abs(angles.angle_z);
            count++;

            if (abs(angles.angle_x)<10 || abs(angles.angle_y)< 10 || abs(angles.angle_z) < 10)
            {
              ssd1306_clear_line(4);
              ssd1306_clear_line(5);
              ssd1306_clear_line(6);
            }
            snprintf(buffer, sizeof(buffer), "X: %d", abs(angles.angle_x));
            ssd1306_display_string(0, 4, buffer);

            snprintf(buffer, sizeof(buffer), "Y: %d", abs(angles.angle_y));
            ssd1306_display_string(0, 5, buffer);

            snprintf(buffer, sizeof(buffer), "Z: %d", abs(angles.angle_z));
            ssd1306_display_string(0, 6, buffer);

            _delay_ms(1000); // Đợi 0.5 giây
        }
    }
            // Tính giá trị trung bình
        int avg_x = sum_x / count;
        int avg_y = sum_y / count;
        int avg_z = sum_z / count;
//     //--------------------------------------//
    ssd1306_clear_line(4);
    ssd1306_clear_line(5);
    ssd1306_clear_line(6);

    // Nếu có dữ liệu từ UART

    if (avg_x < 20 && avg_y < 20) {
        PORTB |= (1 << PB0);
        PORTB &= ~(1 << PB1);
        snprintf(buffer, sizeof(buffer), "X: %d", avg_x);
        ssd1306_display_string(0, 4, buffer);

        snprintf(buffer, sizeof(buffer), "Y: %d", avg_y);
        ssd1306_display_string(0, 5, buffer);

        snprintf(buffer, sizeof(buffer), "Z: %d", avg_z);
        ssd1306_display_string(0, 6, buffer);
        _delay_ms(3000);
    } else if (avg_x > 20 || avg_y > 20) {
        PORTB &= ~(1 << PB0);
        PORTB |= (1 << PB1);
        snprintf(buffer, sizeof(buffer), "X: %d", avg_x);
        ssd1306_display_string(0, 4, buffer);

        snprintf(buffer, sizeof(buffer), "Y: %d", avg_y);
        ssd1306_display_string(0, 5, buffer);

        snprintf(buffer, sizeof(buffer), "Z: %d", avg_z);
        ssd1306_display_string(0, 6, buffer);
        _delay_ms(3000);
    }


}