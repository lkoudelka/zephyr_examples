/*
 * Copyright (c) 2021 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include <zephyr/init.h>
#include <zephyr/pm/policy.h>


#define SLEEP_TIME_MS   2000


#if defined(CONFIG_GPIO)
/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS_OKAY(LED0_NODE)
#include <zephyr/drivers/gpio.h>
#include <stm32wbaxx_ll_gpio.h>
#include <stm32wbaxx_ll_bus.h>
#define HAS_LED     1

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

// Define portable LL-compatible port/pin from devicetree
#define LED_GPIO_PORT ((GPIO_TypeDef *)DT_REG_ADDR(DT_GPIO_CTLR(DT_ALIAS(led0), gpios)))
#define LED_GPIO_PIN  (1U << DT_GPIO_PIN(DT_ALIAS(led0), gpios))  // Bitmask version

#endif /* LED0_NODE */
#endif /* CONFIG_GPIO */


static int blink_setup(void)
{
#if defined(HAS_LED)
	int err;
	printk("Checking LED device...");
	if (!gpio_is_ready_dt(&led)) {
		printk("failed.\n");
		return -EIO;
	}
	printk("done.\n");

	printk("Configuring GPIO pin...");
	err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (err) {
		printk("failed.\n");
		return -EIO;
	}
	printk("done.\n");

#endif
	return 0;
}

static void led_on(void)
{
#if defined(HAS_LED)
	printk("LED on\n");
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
#endif
}

static void led_off(void)
{
#if defined(HAS_LED)
	printk("LED off\n");

	LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
    LL_GPIO_SetPinMode(LED_GPIO_PORT, LED_GPIO_PIN, LL_GPIO_MODE_ANALOG);
	LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOA); 
	
#endif
}

int main(void)
{
	int err;

	printk("Device ready\n");

#if defined(HAS_LED)
	err = blink_setup();
	if (err) {
		return 0;
	}

#endif /* HAS_LED */

	while (true) {
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, 1);
		led_on();
		k_msleep(SLEEP_TIME_MS);
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, 1);
		led_off();
		k_msleep(SLEEP_TIME_MS);
		
	}
	return 0;
}
