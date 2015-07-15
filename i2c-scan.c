/*
 * This file is part of the libopencm3 project.
 *
 *               2015 brabo <brabo.sil@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include "clock.h"
#include "usart.h"
#include "i2c.h"

int _write(int file, char *ptr, int len);

int _write(int file, char *ptr, int len)
{
	int i;

	if (file == 1) {
		for (i = 0; i < len; i++)
			usart_send_blocking(USART1, ptr[i]);
		return i;
	}

	errno = EIO;
	return -1;
}

void print_bits(size_t const size, void const *const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    printf("ret=0x");
    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = b[i] & (1<<j);
            byte >>= j;
            printf("%u", byte);
        }
    }
    printf("\r\n");
}

int main(void)
{
	clock_setup();
	usart_clock_setup();
	gpio_setup();
	usart_setup();

	printf("\r\nstarting i2c scan.\r\n");

	int i, j;
	for (i = 1; i < 0x80; i++) {
		i2c_init();
		msleep(50);
		for (j = 0; j < 0x100; j++) {
			int data; // = 0;
			data = i2c_read(I2C3, i, j);
			if (data > -1) {
				if (data) {
					printf("device on address 0x%02X : reg = 0x%02X with data == 0x%02X\r\n", i, j, data);
				}
			} else {
				printf("errerr!!\r\n");
				break;
			}
			//i2c_send_stop(I2C3);
		}
		i2c_deinit();
		msleep(50);
	}
	printf("scan ended!\r\n");
	return 0;
}
