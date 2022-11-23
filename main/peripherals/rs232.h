#ifndef RS232_H_INCLUDED
#define RS232_H_INCLUDED


#include <stdint.h>
#include <stdlib.h>


void rs232_init(void);
void rs232_write(const uint8_t *data, size_t len);
int  rs232_read(uint8_t *buffer, size_t len, unsigned long ms);
void rs232_flush(void);

#endif