/**
 * File Name:   htu21d.h
 * Description: Header file for the htu21d temperature and humidity sensor driver.
 */

// Header Guard
#ifndef _HTU21D_H_
#define _HTU21D_H_

/***************************************************************************************************/
/* Include Files */
/***************************************************************************************************/
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "log.h"
/***************************************************************************************************/
/* Public Datatypes */
/***************************************************************************************************/

typedef void *						function_cb_t;
#define FUNCTION_CB(v)  		((function_cb_t)(v))

typedef enum {
	htu21_i2c_hold, 
	htu21_i2c_no_hold
} htu21_i2c_master_mode_t;


typedef enum {
	htu21_status_ok,
	htu21_status_no_i2c_acknowledge,
	htu21_status_i2c_transfer_error,
	htu21_status_crc_error
} htu21_status_t;

typedef enum {
	htu21_resolution_t_14b_rh_12b = 0,
	htu21_resolution_t_12b_rh_8b,
	htu21_resolution_t_13b_rh_10b,
	htu21_resolution_t_11b_rh_11b
} htu21_resolution_t;

typedef enum {
	htu21_battery_ok,
	htu21_battery_low
} htu21_battery_status_t;

typedef enum {
	htu21_heater_off,
	htu21_heater_on
} htu21_heater_status_t;
typedef struct htu21 {
	TaskHandle_t thread;
	QueueHandle_t msg_queue;
} htu21_t;
typedef struct htu21_data {
	float temperature;
	float humidity;
} htu21_data_t;
/***************************************************************************************************/
/* Public Constants */
/***************************************************************************************************/
// HTU21 device address
#define HTU21_ADDR											0x40 //0b1000000

// HTU21 device commands
#define HTU21_RESET_COMMAND									0xFE
#define HTU21_READ_TEMPERATURE_W_HOLD_COMMAND				0xE3
#define HTU21_READ_TEMPERATURE_WO_HOLD_COMMAND				0xF3
#define HTU21_READ_HUMIDITY_W_HOLD_COMMAND					0xE5
#define HTU21_READ_HUMIDITY_WO_HOLD_COMMAND					0xF5
#define HTU21_READ_SERIAL_FIRST_8BYTES_COMMAND				0xFA0F
#define HTU21_READ_SERIAL_LAST_6BYTES_COMMAND				0xFCC9
#define HTU21_WRITE_USER_REG_COMMAND						0xE6
#define HTU21_READ_USER_REG_COMMAND							0xE7

#define RESET_TIME											15			// ms value

// Processing constants
#define HTU21_TEMPERATURE_COEFFICIENT						(float)(-0.15)
#define HTU21_CONSTANT_A									(float)(8.1332)
#define HTU21_CONSTANT_B									(float)(1762.39)
#define HTU21_CONSTANT_C									(float)(235.66)

// Coefficients for temperature computation
#define TEMPERATURE_COEFF_MUL								(175.72)
#define TEMPERATURE_COEFF_ADD								(-46.85)

// Coefficients for relative humidity computation
#define HUMIDITY_COEFF_MUL									(125)
#define HUMIDITY_COEFF_ADD									(-6)

// Conversion timings
#define HTU21_TEMPERATURE_CONVERSION_TIME_T_14b_RH_12b		50000
#define HTU21_TEMPERATURE_CONVERSION_TIME_T_13b_RH_10b		25000
#define HTU21_TEMPERATURE_CONVERSION_TIME_T_12b_RH_8b		13000
#define HTU21_TEMPERATURE_CONVERSION_TIME_T_11b_RH_11b		7000
#define HTU21_HUMIDITY_CONVERSION_TIME_T_14b_RH_12b			16000
#define HTU21_HUMIDITY_CONVERSION_TIME_T_13b_RH_10b			5000
#define HTU21_HUMIDITY_CONVERSION_TIME_T_12b_RH_8b			3000
#define HTU21_HUMIDITY_CONVERSION_TIME_T_11b_RH_11b			8000

// HTU21 User Register masks and bit position
#define HTU21_USER_REG_RESOLUTION_MASK						0x81
#define HTU21_USER_REG_END_OF_BATTERY_MASK					0x40
#define HTU21_USER_REG_ENABLE_ONCHIP_HEATER_MASK			0x4
#define HTU21_USER_REG_DISABLE_OTP_RELOAD_MASK				0x2
#define HTU21_USER_REG_RESERVED_MASK						(~(	  HTU21_USER_REG_RESOLUTION_MASK			\
																| HTU21_USER_REG_END_OF_BATTERY_MASK		\
																| HTU21_USER_REG_ENABLE_ONCHIP_HEATER_MASK	\
																| HTU21_USER_REG_DISABLE_OTP_RELOAD_MASK ))

// HTU User Register values
// Resolution
#define HTU21_USER_REG_RESOLUTION_T_14b_RH_12b				0x00
#define HTU21_USER_REG_RESOLUTION_T_13b_RH_10b				0x80
#define HTU21_USER_REG_RESOLUTION_T_12b_RH_8b				0x01
#define HTU21_USER_REG_RESOLUTION_T_11b_RH_11b				0x81

// End of battery status
#define HTU21_USER_REG_END_OF_BATTERY_VDD_ABOVE_2_25V		0x00
#define HTU21_USER_REG_END_OF_BATTERY_VDD_BELOW_2_25V		0x40
// Enable on chip heater
#define HTU21_USER_REG_ONCHIP_HEATER_ENABLE					0x04
#define HTU21_USER_REG_OTP_RELOAD_DISABLE					0x02
/***************************************************************************************************/
/* Public Variables(extern) */
/***************************************************************************************************/

/***************************************************************************************************/
/* Public Function Prototypes */
/***************************************************************************************************/
/**
 * @brief Main routine for talking to the htu21 temp humidity sensor
 */
void htu21_main(void* arg);
/**
 * @brief Configures I2C master to be used with the HTU21 device.
 */
void htu21_init(void);

/**
 * @brief Check whether HTU21 device is connected
 *
 * @return bool : status of HTU21
 *       - true : Device is present
 *       - false : Device is not acknowledging I2C address
 */
bool htu21_is_connected(void);

/**
 * @brief Reset the HTU21 device
 *
 * @return htu21_status : status of HTU21
 *       - htu21_status_ok : I2C transfer completed successfully
 *       - htu21_status_i2c_transfer_error : Problem with i2c transfer
 *       - htu21_status_no_i2c_acknowledge : I2C did not acknowledge
 */
htu21_status_t htu21_reset(void);

/**
 * @brief Reads the htu21 serial number.
 *
 * @param[out] uint64_t* : Serial number
 *
 * @return htu21_status : status of HTU21
 *       - htu21_status_ok : I2C transfer completed successfully
 *       - htu21_status_i2c_transfer_error : Problem with i2c transfer
 *       - htu21_status_no_i2c_acknowledge : I2C did not acknowledge
 *       - htu21_status_crc_error : CRC check error
 */
htu21_status_t htu21_read_serial_number(uint64_t *);

/**
 * @brief Set temperature and humidity ADC resolution.
 *
 * @param[in] htu21_resolution : Resolution requested
 *
 * @return htu21_status : status of HTU21
 *       - htu21_status_ok : I2C transfer completed successfully
 *       - htu21_status_i2c_transfer_error : Problem with i2c transfer
 *       - htu21_status_no_i2c_acknowledge : I2C did not acknowledge
 *       - htu21_status_crc_error : CRC check error
 */
htu21_status_t htu21_set_resolution(htu21_resolution_t);

/**
 * @brief Set I2C master mode. 
 *        This determines whether the program will hold while ADC is accessed or will wait some time
 *
 * @param[in] htu21_i2c_master_mode : I2C mode
 *
 */
void htu21_set_i2c_master_mode(htu21_i2c_master_mode_t);

/**
 * @brief Reads the relative humidity value.
 *
 * @param[out] float* : Celsius Degree temperature value
 * @param[out] float* : %RH Relative Humidity value
 *
 * @return htu21_status : status of HTU21
 *       - htu21_status_ok : I2C transfer completed successfully
 *       - htu21_status_i2c_transfer_error : Problem with i2c transfer
 *       - htu21_status_no_i2c_acknowledge : I2C did not acknowledge
 *       - htu21_status_crc_error : CRC check error
 */
htu21_status_t htu21_read_temperature_and_relative_humidity( float *, float*);

/**
 * @brief Provide battery status
 *
 * @param[out] htu21_battery_status* : Battery status
 *                      - htu21_battery_ok,
 *                      - htu21_battery_low
 *
 * @return status of HTU21
 *       - htu21_status_ok : I2C transfer completed successfully
 *       - htu21_status_i2c_transfer_error : Problem with i2c transfer
 *       - htu21_status_no_i2c_acknowledge : I2C did not acknowledge
 */
htu21_status_t htu21_get_battery_status(htu21_battery_status_t*);

/**
 * @brief Enable heater
 *
 * @return htu21_status : status of HTU21
 *       - htu21_status_ok : I2C transfer completed successfully
 *       - htu21_status_i2c_transfer_error : Problem with i2c transfer
 *       - htu21_status_no_i2c_acknowledge : I2C did not acknowledge
 */
htu21_status_t htu21_enable_heater(void);

/**
 * @brief Disable heater
 *
 * @return htu21_status : status of HTU21
 *       - htu21_status_ok : I2C transfer completed successfully
 *       - htu21_status_i2c_transfer_error : Problem with i2c transfer
 *       - htu21_status_no_i2c_acknowledge : I2C did not acknowledge
 */
htu21_status_t htu21_disable_heater(void);

/**
 * @brief Get heater status
 *
 * @param[in] htu21_heater_status* : Return heater status (above or below 2.5V)
 *	                    - htu21_heater_off,
 *                      - htu21_heater_on
 *
 * @return htu21_status : status of HTU21
 *       - htu21_status_ok : I2C transfer completed successfully
 *       - htu21_status_i2c_transfer_error : Problem with i2c transfer
 *       - htu21_status_no_i2c_acknowledge : I2C did not acknowledge
 */
htu21_status_t htu21_get_heater_status(htu21_heater_status_t*);

/**
 * @brief Returns result of compensated humidity
 *
 * @param[in] float - Actual temperature measured (degC)
 * @param[in] float - Actual relative humidity measured (%RH)
 *
 * @return float - Compensated humidity (%RH).
 */
float htu21_compute_compensated_humidity(float, float);

/**
 * @brief Returns the computed dew point
 *
 * @param[in] float - Actual temperature measured (degC)
 * @param[in] float - Actual relative humidity measured (%RH)
 *
 * @return float - Dew point temperature (DegC).
 */
float htu21_compute_dew_point(float, float);

#endif //End Header Guard