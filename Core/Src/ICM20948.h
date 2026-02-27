/*
 * ICM20948.h
 *
 *  Created on: Feb 26, 2026
 *      Author: Bacnk
 */

#ifndef SRC_ICM20948_H_
#define SRC_ICM20948_H_
#include "main.h"
#include <stdbool.h>
typedef struct 
{ 
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
} ICM20948_HandleTypeDef_SPI;

typedef struct
{
	float x;
	float y;
	float z;
} axises;
typedef enum
{
    power_down_mode = 0,
    single_measurement_mode = 1,
    continuous_measurement_10hz = 2,
    continuous_measurement_20hz = 3,
    continuous_measurement_50hz = 4,
    continuous_measurement_100hz = 8
}operation_mode_t;
typedef enum
{
    ub_0 = 0<<4,
    ub_1 = 1<<4,
    ub_2 = 2<<4,
    ub_3 = 3<<4,
}userbank_t;
typedef enum
{
    _250dps,
    _500dps,
    _1000dps,
    _2000dps,
}gyro_fsr_t;
typedef enum
{
    _4g,
    _8g,
    _16g,
    _32g,
}accel_fsr_t;
#define SPI_READ 0x80
#define SPI_WRITE 0x00
//FUNCTION PROTOTYPE
void ICM20948_Init_Config(ICM20948_HandleTypeDef_SPI *hICM, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);
void ICM20948_Init(ICM20948_HandleTypeDef_SPI *hICM, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);
void MagAk09916_Init(ICM20948_HandleTypeDef_SPI *hICM);
// data raw
void ICM20948_Read_Gyro(ICM20948_HandleTypeDef_SPI *hICM, axises *gyro_data);
void ICM20948_Read_Accel(ICM20948_HandleTypeDef_SPI *hICM, axises *accel_data);
bool MagAk09916_Read_Mag(ICM20948_HandleTypeDef_SPI *hICM, axises *mag_data);
//  convert 16 bit data
void ICM20948_Convert_Gyro(ICM20948_HandleTypeDef_SPI *hICM, axises *gyro_data);
void ICM20948_Convert_Accel(ICM20948_HandleTypeDef_SPI *hICM, axises *accel_data);
void MagAk09916_Convert_Mag(ICM20948_HandleTypeDef_SPI *hICM, axises *mag_data);
//  sub function 
bool ICM20948_Who_Am_I(ICM20948_HandleTypeDef_SPI *hICM);
bool MagAk09916_Who_Am_I(ICM20948_HandleTypeDef_SPI *hICM);
void ICM20948_Device_Reset(ICM20948_HandleTypeDef_SPI *hICM);
void MagAk09916_Device_Reset(ICM20948_HandleTypeDef_SPI *hICM);
void ICM20948_Wake_Up(ICM20948_HandleTypeDef_SPI *hICM);
void ICM20948_Sleep(ICM20948_HandleTypeDef_SPI *hICM);
void ICM20948_Spi_Slave_Enable(ICM20948_HandleTypeDef_SPI *hICM);
void ICM20948_I2c_Master_Enable(ICM20948_HandleTypeDef_SPI *hICM);
void ICM20948_I2c_Master_Reset(ICM20948_HandleTypeDef_SPI *hICM);
void ICM20948_I2C_Master_Clk_Frq(ICM20948_HandleTypeDef_SPI *hICM, uint8_t freq);
void ICM20948_Clock_Source(ICM20948_HandleTypeDef_SPI *hICM, uint8_t clk_src);
void ICM20948_Odr_Align_Enable(ICM20948_HandleTypeDef_SPI *hICM);
void ICM20948_Gyro_Low_Pass_Filter(ICM20948_HandleTypeDef_SPI *hICM, uint8_t lpf);
void ICM20948_Accel_Low_Pass_Filter(ICM20948_HandleTypeDef_SPI *hICM, uint8_t lpf);
// output data rate 
void ICM20948_Gyro_Sample_Rate_Divider(ICM20948_HandleTypeDef_SPI *hICM, uint8_t div);
void ICM20948_Accel_Sample_Rate_Divider(ICM20948_HandleTypeDef_SPI *hICM, uint16_t div);
void MagAk09916_Operation_Mode(ICM20948_HandleTypeDef_SPI *hICM, operation_mode_t mode);
// Calibration
void ICM20948_Calibrate_Gyro(ICM20948_HandleTypeDef_SPI *hICM );
void ICM20948_Calibrate_Accel(ICM20948_HandleTypeDef_SPI *hICMs);
void ICM20948_Gyro_Full_Scale_Select(ICM20948_HandleTypeDef_SPI *hICM, gyro_fsr_t fsr);
void ICM20948_Accel_Full_Scale_Select(ICM20948_HandleTypeDef_SPI *hICM, accel_fsr_t fsr);   
// 


//USER BANK0
#define ICM20948_WHO_AM_I 0x00
#define ICM20948_USER_CTRL 0x03
#define ICM20948_LP_CONFIG 0x05
#define ICM20948_PWR_MGMT_1 0x06
#define ICM20948_PWR_MGMT_2 0x07
#define ICM20948_INT_PIN_CFG 0x0F
#define ICM20948_INT_ENABLE 0x10
#define ICM20948_INT_PIN_CFG 0x0F
#define ICM20948_INT_ENABLE1 0x11
#define ICM20948_INT_ENABLE2 0x12
#define ICM20948_INT_ENABLE3 0x13
#define ICM20948_I2C_MST_STATUS 0x17
#define ICM20948_INT_STATUS 0x19
#define ICM20948_INT_STATUS1 0x1A
#define ICM20948_INT_STATUS2 0x1B
#define ICM20948_INT_STATUS3 0x1C
#define ICM20948_DELAY_TIMEH 0x28
#define ICM20948_DELAY_TIMEL 0x29
#define ICM20948_ACCEL_XOUT_H 0x2D
#define ICM20948_ACCEL_XOUT_L 0x2E
#define ICM20948_ACCEL_YOUT_H 0x2F
#define ICM20948_ACCEL_YOUT_L 0x30
#define ICM20948_ACCEL_ZOUT_H 0x31
#define ICM20948_ACCEL_ZOUT_L 0x32
#define ICM20948_GYRO_XOUT_H 0x33
#define ICM20948_GYRO_XOUT_L 0x34
#define ICM20948_GYRO_YOUT_H 0x35
#define ICM20948_GYRO_YOUT_L 0x36
#define ICM20948_GYRO_ZOUT_H 0x37
#define ICM20948_GYRO_ZOUT_L 0x38
#define ICM20948_TEMP_OUT_H 0x39
#define ICM20948_TEMP_OUT_L 0x3A
#define EXT_SLV_SENS_DATA_00 0x3B
#define EXT_SLV_SENS_DATA_01 0x3C
#define EXT_SLV_SENS_DATA_02 0x3D
#define EXT_SLV_SENS_DATA_03 0x3E
#define EXT_SLV_SENS_DATA_04 0x3F
#define EXT_SLV_SENS_DATA_05 0x40
#define EXT_SLV_SENS_DATA_06 0x41
#define EXT_SLV_SENS_DATA_07 0x42
#define EXT_SLV_SENS_DATA_08 0x43
#define EXT_SLV_SENS_DATA_09 0x44
#define EXT_SLV_SENS_DATA_10 0x45
#define EXT_SLV_SENS_DATA_11 0x46
#define EXT_SLV_SENS_DATA_12 0x47
#define EXT_SLV_SENS_DATA_13 0x48
#define EXT_SLV_SENS_DATA_14 0x49
#define EXT_SLV_SENS_DATA_15 0x4A
#define EXT_SLV_SENS_DATA_16 0x4B
#define EXT_SLV_SENS_DATA_17 0x4C
#define EXT_SLV_SENS_DATA_18 0x4D
#define EXT_SLV_SENS_DATA_19 0x4E
#define EXT_SLV_SENS_DATA_20 0x4F
#define EXT_SLV_SENS_DATA_21 0x50
#define EXT_SLV_SENS_DATA_22 0x51
#define EXT_SLV_SENS_DATA_23 0x52
#define ICM20948_FIFO_EN1 0x66
#define ICM20948_FIFO_EN2 0x67
#define ICM20948_FIFO_RST 0x68
#define ICM20948_FIFO_MODE 0x69
#define ICM20948_FIFO_COUNT_H 0x70
#define ICM20948_FIFO_COUNT_L 0x71
#define ICM20948_FIFO_RW 0x72
#define ICM20948_DATA_RDY_STATUS 0x74
#define ICM20948_FIFO_CFG 0x76
#define ICM20948_REG_BANK_SEL 0x7F
//USER BANK1
#define ICM20948_SELF_TEST_X_GYRO 0x02
#define ICM20948_SELF_TEST_Y_GYRO 0x03
#define ICM20948_SELF_TEST_Z_GYRO 0x04
#define ICM20948_SELF_TEST_X_ACCEL 0x0E
#define ICM20948_SELF_TEST_Y_ACCEL 0x0F
#define ICM20948_SELF_TEST_Z_ACCEL 0x10
#define ICM20948_XA_OFFS_H   0x14
#define ICM20948_XA_OFFS_L   0x15
#define ICM20948_YA_OFFS_H   0x17
#define ICM20948_YA_OFFS_L   0x18
#define ICM20948_ZA_OFFS_H   0x1A
#define ICM20948_ZA_OFFS_L   0x1B
#define ICM20948_TIMEBASE_CORRECTION_PLL 0x28
#define ICM20948_REG_BANK_SEL 0x7F
//USER BANK2
#define ICM20948_GYRO_SMPLRT_DIV 0x00
#define ICM20948_GYRO_CONFIG_1 0x01
#define ICM20948_GYRO_CONFIG_2 0x02
#define ICM20948_XG_OFFS_USRH 0x03
#define ICM20948_XG_OFFS_USRL 0x04
#define ICM20948_YG_OFFS_USRH 0x05
#define ICM20948_YG_OFFS_USRL 0x06
#define ICM20948_ZG_OFFS_USRH 0x07
#define ICM20948_ZG_OFFS_USRL 0x08
#define ICM20948_ODR_ALIGN_EN 0x09
#define ICM20948_ACCEL_SMPLRT_DIV_1 0x10
#define ICM20948_ACCEL_SMPLRT_DIV_2 0x11
#define ICM20948_ACCEL_INTEL_CTRL 0x12
#define ICM20948_ACCEL_WOM_THR 0x13
#define ICM20948_ACCEL_CONFIG 0x14
#define ICM20948_ACCEL_CONFIG_2 0x15
#define ICM20948_FSYNC_CONFIG 0x52
#define ICM20948_TEMP_CONFIG 0x53
#define ICM20948_MOD_CTRL_USR 0x54
#define ICM20948_REG_BANK_SEL 0x7F
//USER BANK3
#define ICM20948_I2C_MST_ODR_CONFIG 0x00
#define ICM20948_I2C_MST_CTRL 0x01
#define ICM20948_I2C_MST_DELAY_CTRL 0x02
#define ICM20948_I2C_SLV0_ADDR 0x03
#define ICM20948_I2C_SLV0_REG 0x04
#define ICM20948_I2C_SLV0_CTRL 0x05
#define ICM20948_I2C_SLV0_DO 0x06
#define ICM20948_I2C_SLV1_ADDR 0x07
#define ICM20948_I2C_SLV1_REG 0x08
#define ICM20948_I2C_SLV1_CTRL 0x09
#define ICM20948_I2C_SLV2_DO 0x0A
#define ICM20948_I2C_SLV2_ADDR 0x0B
#define ICM20948_I2C_SLV2_REG 0x0C
#define ICM20948_I2C_SLV2_CTRL 0x0D
#define ICM20948_I2C_SLV3_DO 0x0E
#define ICM20948_I2C_SLV3_ADDR 0x0F
#define ICM20948_I2C_SLV3_REG 0x10
#define ICM20948_I2C_SLV3_CTRL 0x11
#define ICM20948_I2C_SLV4_DO 0x12
#define ICM20948_I2C_SLV4_ADDR 0x13
#define ICM20948_I2C_SLV4_REG 0x14
#define ICM20948_I2C_SLV4_CTRL 0x15
#define ICM20948_I2C_SLV4_DI 0x16
#define ICM20948_REG_BANK_SEL 0x7F
//I2C Address
#define ICM20948_ID 0xEA
#define ICM20948_REG_BANK_SEL 0x7F
//MAG BANK0
#define MAG20948_SLAVE_ADDR 0x0C
#define MAG20948_ID 0x09
#define MAG20948_WIA2 0x01
#define MAG20948_ST1 0x10
#define MAG20948_HXL 0x11
#define MAG20948_HXH 0x12
#define MAG20948_HYL 0x13
#define MAG20948_HYH 0x14
#define MAG20948_HZL 0x15
#define MAG20948_HZH 0x16
#define MAG20948_ST2 0x18
#define MAG20948_CNTL2 0x31
#define MAG20948_CNTL3 0x32
#define MAG20948_TS1 0x33
#define MAG20948_TS2 0x34

#endif /* SRC_ICM20948_H_ */
