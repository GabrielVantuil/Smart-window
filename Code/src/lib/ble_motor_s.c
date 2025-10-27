#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_MOTOR_S)
#include "ble_motor_s.h"
#include "ble_srv_common.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "handles.h"

/**@brief Function for handling the Write event.
 *
 * @param[in] p_lbs      LED Button Service structure.
 * @param[in] p_ble_evt  Event received from the BLE stack.
 */
void on_write(ble_motor_s_t * p_motor_s, ble_evt_t const * p_ble_evt){
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if ((p_evt_write->handle == p_motor_s->set_motor_handles.value_handle)     && (p_evt_write->len >= 10)  && (p_motor_s->set_motor_handler != NULL)){
		NRF_LOG_INFO("set_motor_handler");
        p_motor_s->set_motor_handler(p_ble_evt->evt.gap_evt.conn_handle, p_motor_s, p_evt_write->data);
    }
    else if ((p_evt_write->handle == p_motor_s->set_config_handles.value_handle)       && (p_evt_write->len == 2)  && (p_motor_s->set_config_handler != NULL)){
		NRF_LOG_INFO("set_config_handler");
        p_motor_s->set_config_handler(p_ble_evt->evt.gap_evt.conn_handle, p_motor_s, p_evt_write->data);
    }
	else {
		NRF_LOG_INFO("write handle not found");
	}
    NRF_LOG_FLUSH();
}

void ble_motor_s_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context){
    ble_motor_s_t * p_motor_s = (ble_motor_s_t *)p_context;
    switch (p_ble_evt->header.evt_id){
        case BLE_GATTS_EVT_WRITE:
            on_write(p_motor_s, p_ble_evt);
            break;
        default:	//No implementation needed.
//            NRF_LOG_INFO("Event unhadled: %d", p_ble_evt->header.evt_id);
            break;
    }
}
uint32_t configAndAddChar(ble_motor_s_t * p_motor_s, uint16_t uuid, uint16_t len, bool isWrite, ble_gatts_char_handles_t * p_char_handle){
    ble_add_char_params_t add_char_params;
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = uuid;
    add_char_params.uuid_type         = p_motor_s->uuid_type;
    add_char_params.init_len          = len;
    add_char_params.max_len           = len;
    add_char_params.char_props.read   = !isWrite;
    add_char_params.char_props.write  = isWrite;
	
    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access 	  = SEC_OPEN;
//	if(!isWrite)    add_char_params->p_init_value	  = packet;
    return characteristic_add(p_motor_s->service_handle, &add_char_params, p_char_handle);
}

uint32_t ble_motor_s_init(ble_motor_s_t * p_motor_s, const ble_motor_s_init_t * p_motor_s_init){
    uint32_t              err_code;
    ble_uuid_t            ble_uuid;

    // Initialize service structure.
	p_motor_s->set_motor_handler        = p_motor_s_init->set_motor_handler;
	p_motor_s->set_config_handler       = p_motor_s_init->set_config_handler;

    // Add service.
    ble_uuid128_t base_uuid = {MOTOR_S_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_motor_s->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_motor_s->uuid_type;
    ble_uuid.uuid = MOTOR_S_UUID_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_motor_s->service_handle);
    VERIFY_SUCCESS(err_code); 
		
    err_code = configAndAddChar(p_motor_s, MOTOR_S_UUID_SET_MOTOR_CHAR  , 11, 1, &p_motor_s->set_motor_handles);
    if (err_code != NRF_SUCCESS)	return err_code;

    err_code = configAndAddChar(p_motor_s, MOTOR_S_UUID_SET_CONFIG_CHAR , 2, 1, &p_motor_s->set_config_handles);
    if (err_code != NRF_SUCCESS)	return err_code;

    err_code = configAndAddChar(p_motor_s, MOTOR_S_UUID_GET_INFO_CHAR   , 1, 0, &p_motor_s->get_info_handles);		//find out th do th, &p_motor_s->read_ldr_handlesis
    if (err_code != NRF_SUCCESS)	return err_code;
	
    return err_code;
}
#endif // NRF_MODULE_ENABLED(BLE_MOTOR_S)
