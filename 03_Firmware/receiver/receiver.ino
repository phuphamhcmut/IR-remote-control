#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Khởi tạo LCD thực tế (thường là 0x27, nếu không lên chữ thì đổi thành 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Các biến toàn cục
char rx_count = 0;
char rx_data = 0;
char rx_invdata = 0;
bool new_command = false;
char current_cmd = 0;

void setup() {
    // Khởi tạo 3 Relay (tương ứng A1, A2, A3)
    DDRC |= (1<<PC1) | (1<<PC2) | (1<<PC3); 
    PORTC &= ~((1<<PC1) | (1<<PC2) | (1<<PC3)); // Tắt toàn bộ ban đầu
    
    // Khởi động màn hình LCD
    lcd.init();
    lcd.backlight();
    
    // Khởi động UART (Giao tiếp với mắt IR)
    Serial.begin(1200); 

    // Màn hình chờ
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("HELLO USER");
    lcd.setCursor(0, 1);
    lcd.print("WAITING CMD...");
}

void loop() {
    // Đọc liên tục nếu mắt hồng ngoại bắt được tín hiệu
    while (Serial.available() > 0) {
        char received = Serial.read(); 
        
        if (rx_count == 0) {
            if (received == '$') rx_count++; // Bắt đầu khung truyền
        } else if (rx_count == 1) {
            rx_data = received; // Ký tự lệnh (Data)
            rx_count++;
        } else if (rx_count == 2) {
            rx_invdata = received; // Ký tự đảo (InvData)
            
            // Đối chiếu chống nhiễu
            if (rx_data == (char)(~rx_invdata)) {
                current_cmd = rx_data;
                new_command = true;
            }
            rx_count = 0; // Reset để đón lệnh tiếp theo
        }
    }

    // Xử lý khi nhận được lệnh hợp lệ
    if (new_command) {
        new_command = false;
        
        // Hiện lên LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RECEIVED:");
        lcd.setCursor(0, 1);
        lcd.print(current_cmd);
        
        // Bật/Tắt Relay
        if (current_cmd == '1') PORTC ^= (1 << PC1); 
        if (current_cmd == '2') PORTC ^= (1 << PC2); 
        if (current_cmd == '3') PORTC ^= (1 << PC3); 
    }
}