
#ifdef CONFIG_GPIO_LED_STUBS

/* 'generic' override of colored LED stubs, to use GPIO functions instead */

#ifdef CONFIG_LED_STATUS_RED
void red_led_on(void)
{
	__led_set(CONFIG_LED_STATUS_RED, CONFIG_LED_STATUS_ON);
}

void red_led_off(void)
{
	__led_set(CONFIG_LED_STATUS_RED, CONFIG_LED_STATUS_OFF);
}
#endif

#ifdef CONFIG_LED_STATUS_GREEN
void green_led_on(void)
{
	__led_set(CONFIG_LED_STATUS_GREEN, CONFIG_LED_STATUS_ON);
}

void green_led_off(void)
{
	__led_set(CONFIG_LED_STATUS_GREEN, CONFIG_LED_STATUS_OFF);
}
#endif

#ifdef CONFIG_LED_STATUS_YELLOW
void yellow_led_on(void)
{
	__led_set(CONFIG_LED_STATUS_YELLOW, CONFIG_LED_STATUS_ON);
}

void yellow_led_off(void)
{
	__led_set(CONFIG_LED_STATUS_YELLOW, CONFIG_LED_STATUS_OFF);
}
#endif

#ifdef CONFIG_LED_STATUS_BLUE
void blue_led_on(void)
{
	__led_set(CONFIG_LED_STATUS_BLUE, CONFIG_LED_STATUS_ON);
}

void blue_led_off(void)
{
	__led_set(CONFIG_LED_STATUS_BLUE, CONFIG_LED_STATUS_OFF);
}
#endif

#endif /* CONFIG_GPIO_LED_STUBS */
