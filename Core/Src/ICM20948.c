/*
 * ICM20948.c
 *
 *  Created on: Feb 26, 2026
 *      Author: Bacnk
 */
#include "ICM20948.h"

static float gyro_scale_factor;
static float accel_scale_factor;
static void Cs_High(ICM20948_HandleTypeDef_SPI *hICM);
static void Cs_Low(ICM20948_HandleTypeDef_SPI *hICM);
static void Select_User_Bank(ICM20948_HandleTypeDef_SPI *hICM, userbank_t bank);
static uint8_t ICM20948_Read_Register(ICM20948_HandleTypeDef_SPI *hICM, userbank_t bank, uint8_t reg);
static void ICM20948_Write_Register(ICM20948_HandleTypeDef_SPI *hICM, userbank_t bank, uint8_t reg, uint8_t data);
static uint8_t *ICM20948_Read_Multiple_Registers(ICM20948_HandleTypeDef_SPI *hICM, userbank_t bank, uint8_t reg, uint8_t *buffer, uint16_t length);
static void ICM20948_Write_Multiple_Registers(ICM20948_HandleTypeDef_SPI *hICM, userbank_t bank, uint8_t reg, uint8_t *buffer, uint16_t length);
static uint8_t MagAk09916_Read_Register(ICM20948_HandleTypeDef_SPI *hICM, uint8_t reg);
static void MagAk09916_Write_Register(ICM20948_HandleTypeDef_SPI *hICM, uint8_t reg, uint8_t data);
static uint8_t* MagAk09916_Read_Multiple_Registers(ICM20948_HandleTypeDef_SPI *hICM, uint8_t reg, uint8_t *buffer, uint16_t length);

static void Cs_High(ICM20948_HandleTypeDef_SPI *hICM)
{
    HAL_GPIO_WritePin(hICM->cs_port, hICM->cs_pin, GPIO_PIN_SET);
}
static void Cs_Low(ICM20948_HandleTypeDef_SPI *hICM)
{
    HAL_GPIO_WritePin(hICM->cs_port, hICM->cs_pin, GPIO_PIN_RESET);
}
void ICM20948_Init_Config(ICM20948_HandleTypeDef_SPI *hICM, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    hICM->hspi = hspi;
    hICM->cs_port = cs_port;
    hICM->cs_pin = cs_pin;
}
static void Select_User_Bank(ICM20948_HandleTypeDef_SPI *hICM, userbank_t bank)
{
    uint8_t write_data[2];
    write_data[0] = SPI_WRITE | ICM20948_REG_BANK_SEL;
    write_data[1] = bank;
    Cs_Low(hICM);
    HAL_SPI_Transmit(hICM->hspi, write_data, 2, HAL_MAX_DELAY);
    Cs_High(hICM);
}
//SUB FUNCTION PROTOTYPE
bool ICM20948_Who_Am_I(ICM20948_HandleTypeDef_SPI *hICM)
{
    uint8_t who_am_i = ICM20948_Read_Register(hICM, ub_0, ICM20948_WHO_AM_I);
    return who_am_i == ICM20948_ID;
}
bool MagAk09916_Who_Am_I(ICM20948_HandleTypeDef_SPI *hICM)
{
    uint8_t who_am_i = MagAk09916_Read_Register(hICM, MAG20948_WIA2);
    return who_am_i == MAG20948_ID;
}
void ICM20948_Device_Reset(ICM20948_HandleTypeDef_SPI *hICM)
{
    ICM20948_Write_Register(hICM, ub_0, ICM20948_PWR_MGMT_1, 0x80|0x41);
    HAL_Delay(100);
}
void MagAk09916_Device_Reset(ICM20948_HandleTypeDef_SPI *hICM)
{
    MagAk09916_Write_Register(hICM, MAG20948_CNTL3, 0x01);
    HAL_Delay(100);
}
void ICM20948_Wake_Up(ICM20948_HandleTypeDef_SPI *hICM)
{
    uint8_t new_val = ICM20948_Read_Register(hICM, ub_0, ICM20948_PWR_MGMT_1) & 0xBF;
    ICM20948_Write_Register(hICM, ub_0, ICM20948_PWR_MGMT_1, new_val);
    HAL_Delay(100);
}
void ICM20948_Sleep(ICM20948_HandleTypeDef_SPI *hICM)
{
    uint8_t new_val = ICM20948_Read_Register(hICM, ub_0, ICM20948_PWR_MGMT_1) | 0x40;
    ICM20948_Write_Register(hICM, ub_0, ICM20948_PWR_MGMT_1, new_val);
    HAL_Delay(100);
}
void ICM20948_Spi_Slave_Enable(ICM20948_HandleTypeDef_SPI *hICM)
{
    uint8_t new_val = ICM20948_Read_Register(hICM, ub_0, ICM20948_USER_CTRL) | 0x10;
    ICM20948_Write_Register(hICM, ub_0, ICM20948_USER_CTRL, new_val);
}
void ICM20948_I2c_Master_Reset(ICM20948_HandleTypeDef_SPI *hICM)
{
    uint8_t new_val = ICM20948_Read_Register(hICM, ub_0, ICM20948_USER_CTRL) | 0x2;
    ICM20948_Write_Register(hICM, ub_0, ICM20948_USER_CTRL, new_val);
}
void ICM20948_I2c_Master_Enable(ICM20948_HandleTypeDef_SPI *hICM)
{
    uint8_t new_val = ICM20948_Read_Register(hICM, ub_0, ICM20948_USER_CTRL) | 0x20;
    ICM20948_Write_Register(hICM, ub_0, ICM20948_USER_CTRL, new_val);
}
void ICM20948_I2C_Master_Clk_Frq(ICM20948_HandleTypeDef_SPI *hICM, uint8_t freq)
{
    uint8_t new_val = ICM20948_Read_Register(hICM, ub_3, ICM20948_I2C_MST_CTRL) | freq;
    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_MST_CTRL, new_val);
}
void ICM20948_Clock_Source(ICM20948_HandleTypeDef_SPI *hICM, uint8_t clk_src)
{
    uint8_t new_val = ICM20948_Read_Register(hICM, ub_0, ICM20948_PWR_MGMT_1) | clk_src;
    ICM20948_Write_Register(hICM, ub_0, ICM20948_PWR_MGMT_1, new_val);
}
void ICM20948_Odr_Align_Enable(ICM20948_HandleTypeDef_SPI *hICM)
{
    ICM20948_Write_Register(hICM, ub_2, ICM20948_ODR_ALIGN_EN, 0x1);
}
void ICM20948_Gyro_Low_Pass_Filter(ICM20948_HandleTypeDef_SPI *hICM, uint8_t lpf)
{
    uint8_t new_val = ICM20948_Read_Register(hICM, ub_2, ICM20948_GYRO_CONFIG_1) | (lpf<<3);
    ICM20948_Write_Register(hICM, ub_2, ICM20948_GYRO_CONFIG_1, new_val);
}
void ICM20948_Accel_Low_Pass_Filter(ICM20948_HandleTypeDef_SPI *hICM, uint8_t lpf)
{
    uint8_t new_val = ICM20948_Read_Register(hICM, ub_2, ICM20948_ACCEL_CONFIG) | (lpf<<3);
    ICM20948_Write_Register(hICM, ub_2, ICM20948_ACCEL_CONFIG, new_val);
}
// output data rate 
void ICM20948_Gyro_Sample_Rate_Divider(ICM20948_HandleTypeDef_SPI *hICM, uint8_t div)
{
    ICM20948_Write_Register(hICM, ub_2, ICM20948_GYRO_SMPLRT_DIV, div);
}
void ICM20948_Accel_Sample_Rate_Divider(ICM20948_HandleTypeDef_SPI *hICM, uint16_t div)
{
    ICM20948_Write_Register(hICM, ub_2, ICM20948_ACCEL_SMPLRT_DIV_1, (div>>8) & 0xFF);
    ICM20948_Write_Register(hICM, ub_2, ICM20948_ACCEL_SMPLRT_DIV_2, div & 0xFF);
}
void MagAk09916_Operation_Mode(ICM20948_HandleTypeDef_SPI *hICM, operation_mode_t mode)
{
    MagAk09916_Write_Register(hICM, MAG20948_CNTL2, mode);
}
// Calibration
void ICM20948_Calibrate_Gyro(ICM20948_HandleTypeDef_SPI *hICM )
{
    //TODO
}
// READ AND WRITE REGISTER
static uint8_t ICM20948_Read_Register(ICM20948_HandleTypeDef_SPI *hICM, userbank_t bank, uint8_t reg)
{
    uint8_t write_data[2];
    uint8_t read_data;
    write_data[0] = SPI_READ | reg;
    Select_User_Bank(hICM, bank);
    Cs_Low(hICM);
    HAL_SPI_TransmitReceive(hICM->hspi, write_data, &read_data, 1, HAL_MAX_DELAY);
    Cs_High(hICM);
    return read_data;
}
static void ICM20948_Write_Register(ICM20948_HandleTypeDef_SPI *hICM, userbank_t bank, uint8_t reg, uint8_t data)
{
    uint8_t write_data[2];
    write_data[0] = SPI_WRITE | reg;
    write_data[1] = data;
    Select_User_Bank(hICM, bank);
    Cs_Low(hICM);
    HAL_SPI_Transmit(hICM->hspi, write_data, 2, HAL_MAX_DELAY);
    Cs_High(hICM);
}
static uint8_t *ICM20948_Read_Multiple_Registers(ICM20948_HandleTypeDef_SPI *hICM, userbank_t bank, uint8_t reg, uint8_t *buffer, uint16_t length)
{
    uint8_t write_data = SPI_READ | reg;
    Select_User_Bank(hICM, bank);
    Cs_Low(hICM);
    HAL_SPI_Transmit(hICM->hspi,  &write_data, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(hICM->hspi, buffer, length, HAL_MAX_DELAY);
    Cs_High(hICM);
    return buffer;
}
static void ICM20948_Write_Multiple_Registers(ICM20948_HandleTypeDef_SPI *hICM, userbank_t bank, uint8_t reg, uint8_t *buffer, uint16_t length)
{
    uint8_t write_data = SPI_WRITE | reg;
    Select_User_Bank(hICM, bank);
    Cs_Low(hICM);
    HAL_SPI_Transmit(hICM->hspi, &write_data, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(hICM->hspi, buffer, length, HAL_MAX_DELAY);
    Cs_High(hICM);
}
static uint8_t MagAk09916_Read_Register(ICM20948_HandleTypeDef_SPI *hICM, uint8_t reg)
{
    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_SLV0_ADDR, MAG20948_SLAVE_ADDR | SPI_READ);
    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_SLV0_REG, reg);
    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_SLV0_CTRL, 0x81);
    HAL_Delay(10);
    return ICM20948_Read_Register(hICM, ub_0, EXT_SLV_SENS_DATA_00);
}
static void MagAk09916_Write_Register(ICM20948_HandleTypeDef_SPI *hICM, uint8_t reg, uint8_t data)
{
    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_SLV0_ADDR, MAG20948_SLAVE_ADDR | SPI_WRITE);
    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_SLV0_REG, reg);

    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_SLV0_DO, data);
    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_SLV0_CTRL, 0x81);
}
static uint8_t* MagAk09916_Read_Multiple_Registers(ICM20948_HandleTypeDef_SPI *hICM, uint8_t reg, uint8_t *buffer, uint16_t length)
{
    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_SLV0_ADDR, MAG20948_SLAVE_ADDR | SPI_READ);
    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_SLV0_REG, reg);
    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_SLV0_CTRL, 0x80 | length);
    HAL_Delay(10);
    return ICM20948_Read_Multiple_Registers(hICM, ub_0, EXT_SLV_SENS_DATA_00, buffer, length);
}

