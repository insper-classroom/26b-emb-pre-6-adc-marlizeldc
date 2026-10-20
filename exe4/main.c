/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


 #include <stdio.h>
 #include "pico/stdlib.h"
 #include "hardware/gpio.h"
 #include "hardware/timer.h"
 #include "hardware/adc.h"

 const int PIN_LED_B = 4;
 const int PIN_POT = 28;

 const float conversion_factor = 3.3f / (1 << 12);

 volatile int g_timer_led = 0;

 static bool led_timer_callback(repeating_timer_t *rt) {
     g_timer_led = 1;
     return true; // keep repeating
 }

 static int get_zone(float voltage) {
     if (voltage < 1.0f)
         return 0;
     if (voltage < 2.0f)
         return 1;
     return 2;
 }

 /**
  * 0..1.0V: Desligado
  * 1..2.0V: 300 ms
  * 2..3.3V: 500 ms
  */
 int main() {
     stdio_init_all();

     gpio_init(PIN_LED_B);
     gpio_set_dir(PIN_LED_B, GPIO_OUT);
     gpio_put(PIN_LED_B, 0);

     adc_init();
     adc_gpio_init(PIN_POT);
     adc_select_input(2); // GP28 -> ADC2

     const int zone_period_ms[] = {0, 300, 500};

     repeating_timer_t timer;
     int timer_running = 0;
     int zone_atual = 0;

     while (1) {
         float voltage = adc_read() * conversion_factor;
         int zone = get_zone(voltage);

         if (zone != zone_atual) {
             zone_atual = zone;

             if (timer_running) {
                 cancel_repeating_timer(&timer);
                 timer_running = 0;
             }
             g_timer_led = 0;
             gpio_put(PIN_LED_B, 0);

             if (zone != 0) {
                 gpio_put(PIN_LED_B, 1);
                 add_repeating_timer_ms(zone_period_ms[zone], led_timer_callback, NULL, &timer);
                 timer_running = 1;
             }
         }

         if (g_timer_led) {
             g_timer_led = 0;
             gpio_put(PIN_LED_B, !gpio_get(PIN_LED_B));
         }
     }
 }
