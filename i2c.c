#include <stdint.h>
#include <stdio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "i2c.h"


static uint8_t i2c_start(uint32_t i2c, uint8_t address, uint8_t mode);

void i2c_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_I2C3);

	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ,
		GPIO8);
	gpio_set_af(GPIOA, GPIO_AF4, GPIO8);

	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
	gpio_set_output_options(GPIOC, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ,
		GPIO9);
	gpio_set_af(GPIOC, GPIO_AF4, GPIO9);

	i2c_peripheral_disable(I2C3); /* disable i2c during setup */
	i2c_reset(I2C3);

	i2c_set_fast_mode(I2C3);
	i2c_set_clock_frequency(I2C3, I2C_CR2_FREQ_42MHZ);
	i2c_set_ccr(I2C3, 35);
	i2c_set_trise(I2C3, 43);
	//i2c_set_speed(I2C3, 0);

	i2c_peripheral_enable(I2C3); /* finally enable i2c */

	i2c_set_own_7bit_slave_address(I2C3, 0x00);
}

void i2c_deinit(void)
{
	i2c_send_stop(I2C3);

	i2c_reset(I2C3);
	i2c_peripheral_disable(I2C3); /* disable i2c during setup */

}

static uint8_t i2c_start(uint32_t i2c, uint8_t address, uint8_t mode)
{
	i2c_send_start(i2c);

	/* Wait for master mode selected */
	while (!((I2C_SR1(i2c) & I2C_SR1_SB)
		& (I2C_SR2(i2c) & (I2C_SR2_MSL | I2C_SR2_BUSY))));

	i2c_send_7bit_address(i2c, address, mode);

	int timeout = 20000;
	/* Waiting for address is transferred. */
	while (!(I2C_SR1(i2c) & I2C_SR1_ADDR)) {
		if (timeout > 0) {
			timeout--;
		} else {
			return 1;
		}
	}


	/* Cleaning ADDR condition sequence. */
	uint32_t reg32 = I2C_SR2(i2c);
	(void) reg32; /* unused */

	return 0;
}

uint8_t i2c_write(uint32_t i2c, uint8_t address, uint8_t reg,
	uint8_t data)
{
	i2c_start(i2c, address, I2C_WRITE);

	i2c_send_data(i2c, reg);

	while (!(I2C_SR1(i2c) & (I2C_SR1_BTF)));
	i2c_send_data(i2c, data);

	while (!(I2C_SR1(i2c) & (I2C_SR1_BTF)));

	i2c_send_stop(i2c);

	return 0;
}

int i2c_read(uint32_t i2c, uint8_t address, uint8_t reg)
{
	uint32_t timeout = 20000;
	while ((I2C_SR2(i2c) & I2C_SR2_BUSY)); // {
//		if (timeout > 0) {
//			timeout--;
//		} else {
//			return -1;
//		}
//	}

	if (i2c_start(i2c, address, I2C_WRITE)) {
		return 0;
	}
	i2c_send_data(i2c, reg);

	timeout = 20000;
	while (!(I2C_SR1(i2c) & (I2C_SR1_BTF))) {
		if (timeout > 0) {
			timeout--;
		} else {
			return -1;
		}
	}

	i2c_start(i2c, address, I2C_READ);

	i2c_send_stop(i2c);

	while (!(I2C_SR1(i2c) & I2C_SR1_RxNE));

	int result = (int)i2c_get_data(i2c);

	I2C_SR1(i2c) &= ~I2C_SR1_AF;
	msleep(50);
	i2c_send_stop(i2c);

	return result;
}
