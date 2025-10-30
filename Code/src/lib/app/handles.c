#include "handles.h"
#include "utils.h"

#include "app_pwm.h"

APP_PWM_INSTANCE(PWM0,0);
APP_PWM_INSTANCE(PWM1,1);
//APP_PWM_INSTANCE(PWM2,2);
bool ledPowerLocked;

extern bool ledValue;
extern int16_t MA_duty;
extern int16_t MB_duty;
int16_t scheduledDuty[2];

void timers_init(void){
    // Initialize timer module, making it use the scheduler
    APP_ERROR_CHECK(app_timer_init());
	APP_ERROR_CHECK(app_timer_create(&motor_timer_id, APP_TIMER_MODE_SINGLE_SHOT, motorTimer));
}

void motorTimer(void * p_context){
	if((scheduledDuty[0]>>8)!=0x7F) motorACtrl(scheduledDuty[0]);
	if((scheduledDuty[1]>>8)!=0x7F) motorBCtrl(scheduledDuty[1]);
}

void motorsCtrl(int16_t dutyA, int16_t dutyB){
	motorACtrl(dutyA);
	motorBCtrl(dutyB);
}
void motorACtrl(int16_t duty){
	MA_duty = duty;
	nrf_gpio_pin_write(MI_A1_PIN, duty>=1);
	nrf_gpio_pin_write(MI_A2_PIN, duty<=-1);	
	ledValue = (duty!=0)?LED_ON:LED_OFF;
}
void motorBCtrl(int16_t duty){
	MB_duty = duty;
	nrf_gpio_pin_write(MI_B1_PIN, duty>=1);
	nrf_gpio_pin_write(MI_B2_PIN, duty<=-1);	
	ledValue = (duty!=0)?LED_ON:LED_OFF;
}
void pwm_ready_callback(uint32_t pwm_id){}
void setLedPwm(uint32_t freqX1000, uint16_t duty){	
	app_pwm_uninit(&PWM1);
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH(1000000000/freqX1000, MI_A1_PIN);
    APP_ERROR_CHECK(app_pwm_init(&PWM1, &pwm1_cfg, pwm_ready_callback));
    app_pwm_enable(&PWM1);
	while (app_pwm_channel_duty_set(&PWM1, 0, 100-duty) == NRF_ERROR_BUSY);
}

void set_motor_handler	(uint16_t conn_handle, ble_motor_s_t * p_motor_s, const uint8_t *params){
	APP_ERROR_CHECK(app_timer_stop(motor_timer_id));
	uint32_t timeout;
	ArrayToInt32(params, 6, timeout);
	if(timeout){
		scheduledDuty[0] = ((params[0]<<8) +params[1]);
		scheduledDuty[1] = ((params[2]<<8) +params[3]);
		APP_ERROR_CHECK(app_timer_start(motor_timer_id, APP_TIMER_TICKS(timeout), NULL));
		return;
	}
	if(params[0]!=0x7F) motorACtrl((params[0]<<8) + params[1]);
	if(params[2]!=0x7F) motorBCtrl((params[2]<<8) + params[3]);
}

void set_config_handler	(uint16_t conn_handle, ble_motor_s_t * p_motor_s, const uint8_t *params){
	if(ledPowerLocked) return;
	uint32_t freq, duty, timeout;
	ArrayToInt32(params, 0, freq);
	duty = (params[4]<<8) + params[5];
	ArrayToInt32(params, 6, timeout);
	setLedPwm(freq, duty);
}
void connectionTimeout(void * p_context){
	sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
}

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
void leds_init(void){
	setLedPwm(10000000, 70);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
void idle_state_handle(void){
    if (NRF_LOG_PROCESS() == false){
        nrf_pwr_mgmt_run();
    }
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
void conn_params_error_handler(uint32_t nrf_error){
    APP_ERROR_HANDLER(nrf_error);
}
/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name){
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}
