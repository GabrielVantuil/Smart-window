#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "app_timer.h"

#include "nrf_drv_gpiote.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_drv_clock.h"
#include "nrf_delay.h"

#include "app_ble.h"
#include "handles.h"
#include "Battery_level.h"

static void log_init(void){
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void lfclk_config(void){
	// initialize the low power low frequency clock
  APP_ERROR_CHECK(nrf_drv_clock_init());

	// request the lf clock to not to generate any events on ticks
	// One tick =  1 value increment in the counter register
  nrf_drv_clock_lfclk_request(NULL);
}


uint16_t getAdc(void){
    nrf_saadc_value_t value;
    APP_ERROR_CHECK(nrf_drv_saadc_sample_convert(0, &value));
    return value;
}

bool ledValue;
/**@brief Function for application main entry.
 */
int main(void){
    // Initialize.
    lfclk_config();
    log_init();
    timers_init();
    APP_ERROR_CHECK(nrf_pwr_mgmt_init());
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
//	calcBatteryLevel(NULL);
	// sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, m_adv_handle, 4);
	
	advertising_start();
	
//	setLedPwm(100000, 50);
	
	nrf_gpio_cfg_output(MI_A1_PIN);
	nrf_gpio_cfg_output(MI_A2_PIN);
	nrf_gpio_cfg_output(MI_B1_PIN);
	nrf_gpio_cfg_output(MI_B2_PIN);
	
	nrf_gpio_cfg_output(SENSORS_PWR_PIN);
	nrf_gpio_cfg_output(LED_PIN);
	
	nrf_gpio_pin_write(LED_PIN, LED_OFF);
	nrf_gpio_pin_write(SENSORS_PWR_PIN, 1);
	motorACtrl(0);
	motorBCtrl(0);
	
	nrf_gpio_cfg_input(SW1_PIN, NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(SW2_PIN, NRF_GPIO_PIN_PULLUP);
	
	
    APP_ERROR_CHECK(nrf_drv_saadc_init(NULL, NULL));
    nrf_saadc_channel_config_t channel_config = 
		NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN2);

    APP_ERROR_CHECK(nrf_drv_saadc_channel_init(0, &channel_config));
	
//    for (;;){
//		motorCtrl(1, 0);
//		nrf_delay_ms(1000);
//		motorCtrl(0, 0);
//		nrf_delay_ms(1000);
//		motorCtrl(-1, 0);
//		nrf_delay_ms(1000);
//		motorCtrl(0, 0);
//		nrf_delay_ms(1000);
//	}
	int8_t dir=0;
	int idleCount = 0;
    for (;;){
		uint16_t shunt = getAdc();
		if(shunt>60000)	idleCount++; 
		if((idleCount>50) || (shunt < 60000)){
			NRF_LOG_INFO("%d", shunt);
			NRF_LOG_FLUSH();
			idleCount = 0;
		}
		
		if((shunt>100) && (shunt<60000)){
			motorACtrl(0);
			nrf_delay_ms(1000);
			dir=0;
			motorACtrl(dir);
		}
		
		if(!nrf_gpio_pin_read(SW1_PIN) && !nrf_gpio_pin_read(SW2_PIN)){
			dir=0;
			motorACtrl(dir);
			nrf_delay_ms(500);
		}
		else if(!nrf_gpio_pin_read(SW1_PIN)){
			dir=1;
			motorACtrl(dir);
		}
		else if(!nrf_gpio_pin_read(SW2_PIN)){
			dir=-1;
			motorACtrl(dir);
		}
//		else{
//			motorCtrl(0, 0);
//			nrf_gpio_pin_write(LED_PIN, LED_OFF);
//		}
		nrf_delay_ms(10);
		nrf_gpio_pin_write(LED_PIN, LED_ON);
		nrf_delay_us(10);
		if(!ledValue){
			nrf_gpio_pin_write(LED_PIN, LED_OFF);
		}
		nrf_delay_ms(20);
        //idle_state_handle();
    }
}


/**
 * @}
 */
