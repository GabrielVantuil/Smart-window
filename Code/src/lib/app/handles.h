#ifndef HANDLERS_H__
#define HANDLERS_H__
#include "nrf_drv_gpiote.h"
#include "ble_motor_s.h"
#include "parameters.h"
#include "app_timer.h"
#include "nrf_pwr_mgmt.h"
#include "app_ble.h"
#include "nrf_delay.h"

APP_TIMER_DEF(power_off_led_timer_id);

void timers_init(void);
void startTimers(void);
void setLed(bool val);
void blinkLed(uint8_t led);
void motorACtrl(int16_t duty);
void motorBCtrl(int16_t duty);

void setLedOff(void * p_context);
void setLedPwm(uint32_t freqX1000, uint16_t duty);
void set_motor_handler	(uint16_t conn_handle, ble_motor_s_t * p_motor_s, const uint8_t *params);
void set_config_handler (uint16_t conn_handle, ble_motor_s_t * p_motor_s, const uint8_t *params);

void blinkLedHandler(void * p_context);

void connectionTimeout(void * p_context);

void leds_init(void);

void idle_state_handle(void);
void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void conn_params_error_handler(uint32_t nrf_error);
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name);

#endif

