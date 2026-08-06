#include "../kcode/io.h"
#include "stdio.h"

void serial_init()
{
    outb(0x00, 0x3F9); // Disable interrupts
    outb(0x80, 0x3FB); // Enable DLAB
    outb(0x03, 0x3F8); // Divisor low (38400 baud)
    outb(0x00, 0x3F9); // Divisor high
    outb(0x03, 0x3FB); // 8 bits, no parity, one stop bit
    outb(0xC7, 0x3FA); // Enable FIFO
    outb(0x0B, 0x3FC); // IRQs enabled, RTS/DSR
}

void serial_putchar(char c) {
    while (!(inb(0x3FD) & 0x20))
        ;

    outb(c, 0x3F8);
}
void serial_print(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        serial_putchar(str[i]);
        i++;
    }
}
void serial_print_hex(uint64_t value)
{
    const char hex_chars[] = {
        '0','1','2','3','4','5','6','7',
        '8','9','A','B','C','D','E','F',
        0
    };

    char buffer[19];

    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[18] = 0;

    for (int i = 17; i >= 2; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }

    serial_print(buffer);
    serial_putchar('\n');
}