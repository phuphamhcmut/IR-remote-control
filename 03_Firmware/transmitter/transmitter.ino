#define F_CPU 16000000UL // Sử dụng thạch anh ngoài 16MHz
#include <avr/io.h>
#include <util/delay.h>

// --- HÀM KHỞI TẠO UART ---
void USART_Init(unsigned int baud) {
    unsigned int ubrr = F_CPU / 16 / baud - 1;
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0); // Chỉ bật tính năng phát (TX)
    UCSR0C = (1 << USBS0) | (3 << UCSZ00); // Khung truyền: 8 bit data, 2 stop bit
}

void USART_TxChar(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0))); // Chờ bộ đệm trống
    UDR0 = data; // Gửi dữ liệu
}

// --- HÀM KHỞI TẠO XUNG PWM 38kHz (Cập nhật cho 16MHz) ---
void PWM_Init(uint16_t period) {
    DDRB |= (1 << PB1); // Cấu hình PB1 (OC1A) là ngõ ra
    TCCR1A = (1 << WGM11) | (1 << COM1A1); 
    TCCR1B = (1 << WGM12) | (1 << WGM13) | (1 << CS10); // Chế độ Fast PWM, Prescaler = 1
    ICR1 = period;
}

void PWM_Set_Duty(uint16_t duty) {
    OCR1A = duty;
}

// --- HÀM CẤU HÌNH VÀ QUÉT BÀN PHÍM ---
void Keypad_Init() {
    // 4 Hàng (Row) là Output: PD2, PD3, PD4, PD5
    DDRD |= (1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5);
    // 4 Cột (Col) là Input: PD6, PD7, PB2, PB3
    DDRD &= ~((1<<PD6) | (1<<PD7));
    DDRB &= ~((1<<PB2) | (1<<PB3));
    // Bật điện trở kéo lên (Pull-up) cho 4 chân Cột
    PORTD |= (1<<PD6) | (1<<PD7);
    PORTB |= (1<<PB2) | (1<<PB3);
}

char keyfind() {
    char keys[4][4] = {
        {'1','2','3','A'},
        {'4','5','6','B'},
        {'7','8','9','C'},
        {'*','0','#','D'}
    };
    
    for (int r = 0; r < 4; r++) {
        // Cho tất cả 4 hàng lên HIGH
        PORTD |= (1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5);
        // Kéo duy nhất hàng đang xét xuống LOW
        PORTD &= ~(1 << (r + 2)); 
        _delay_ms(1); // Chờ điện áp ổn định
        
        // Kiểm tra xem có cột nào bị kéo xuống LOW không (Nghĩa là có nút được nhấn)
        if (!(PIND & (1<<PD6))) { _delay_ms(20); while(!(PIND & (1<<PD6))); return keys[r][0]; }
        if (!(PIND & (1<<PD7))) { _delay_ms(20); while(!(PIND & (1<<PD7))); return keys[r][1]; }
        if (!(PINB & (1<<PB2))) { _delay_ms(20); while(!(PINB & (1<<PB2))); return keys[r][2]; }
        if (!(PINB & (1<<PB3))) { _delay_ms(20); while(!(PINB & (1<<PB3))); return keys[r][3]; }
    }
    return 0; // Không có nút nào được nhấn
}

// --- CHƯƠNG TRÌNH CHÍNH ---
int main() {
    char j, d = 0;
    
    Keypad_Init();
    USART_Init(1200);  // Tốc độ Baud 1200
    
    // Config cho Thạch anh 16MHz để ra tần số PWM 38kHz
    PWM_Init(420);     
    PWM_Set_Duty(210); // Duty cycle 50%

    while (1) {
        j = keyfind();
        
        if (j != 0) { // Nếu có phím được bấm
            // Phát dữ liệu qua UART
            USART_TxChar('$'); // Ký tự mào đầu để mạch thu nhận diện
            USART_TxChar(j);   // Dữ liệu nút nhấn (Data)
            d = ~j;            // Tạo dữ liệu đảo (Chống nhiễu)
            USART_TxChar(d);   // Gửi dữ liệu đảo
            
            _delay_ms(150);    // Trễ một chút để tránh gửi liên tục quá nhanh
        }
    }
}