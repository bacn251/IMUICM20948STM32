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
static void ICM20948_Read_Multiple_Registers(ICM20948_HandleTypeDef_SPI *hICM, userbank_t bank, uint8_t reg, uint8_t *buffer, uint16_t length);
static void ICM20948_Write_Multiple_Registers(ICM20948_HandleTypeDef_SPI *hICM, userbank_t bank, uint8_t reg, uint8_t *buffer, uint16_t length);
static uint8_t MagAk09916_Read_Register(ICM20948_HandleTypeDef_SPI *hICM, uint8_t reg);
static void MagAk09916_Write_Register(ICM20948_HandleTypeDef_SPI *hICM, uint8_t reg, uint8_t data);
static void MagAk09916_Read_Multiple_Registers(ICM20948_HandleTypeDef_SPI *hICM, uint8_t reg, uint8_t *buffer, uint16_t length);
// INIT FUNCTION
void ICM20948_Init(ICM20948_HandleTypeDef_SPI *hICM, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    ICM20948_Init_Config(hICM, hspi, cs_port, cs_pin);
    ICM20948_Device_Reset(hICM);
    ICM20948_Wake_Up(hICM);
    if (!ICM20948_Who_Am_I(hICM))
    {
        // Handle error: device not found
        return;
    }
    ICM20948_Clock_Source(hICM, 0x01); // Set clock source to Auto selects the best available clock source – PLL if ready, else use the Internal oscillator
    ICM20948_Odr_Align_Enable(hICM);
    ICM20948_Spi_Slave_Enable(hICM);
    ICM20948_Gyro_Sample_Rate_Divider(hICM, 0x00);  // Set gyro sample rate to 1kHz
    ICM20948_Accel_Sample_Rate_Divider(hICM, 0x00); // Set accel sample rate to 1kHz
    ICM20948_Gyro_Full_Scale_Select(hICM, _2000dps);
    ICM20948_Accel_Full_Scale_Select(hICM, _16g);
    ICM20948_Gyro_Low_Pass_Filter(hICM, 0x00);  // Set gyro low pass filter to 250Hz
    ICM20948_Accel_Low_Pass_Filter(hICM, 0x00); // Set accel low pass filter to 460Hz
}
void MagAk09916_Init(ICM20948_HandleTypeDef_SPI *hICM)
{
    ICM20948_I2c_Master_Reset(hICM);

    ICM20948_I2c_Master_Enable(hICM);
    ICM20948_I2C_Master_Clk_Frq(hICM, 0x07);
    if (!MagAk09916_Who_Am_I(hICM))
    {
        // Handle error: device not found
        return;
    }
    MagAk09916_Device_Reset(hICM);
    MagAk09916_Operation_Mode(hICM, continuous_measurement_100hz); // Set magnetometer to continuous measurement mode 1 (10Hz)
}
void ICM20948_Read_Gyro(ICM20948_HandleTypeDef_SPI *hICM, axises *gyro_data)
{
    uint8_t raw_data[6];
    ICM20948_Read_Multiple_Registers(hICM, ub_0, ICM20948_GYRO_XOUT_H, raw_data, 6);
    gyro_data->x = (int16_t)(raw_data[0] << 8 | raw_data[1]);
    gyro_data->y = (int16_t)(raw_data[2] << 8 | raw_data[3]);
    gyro_data->z = (int16_t)(raw_data[4] << 8 | raw_data[5]);
}
void ICM20948_Read_Accel(ICM20948_HandleTypeDef_SPI *hICM, axises *accel_data)
{
    uint8_t raw_data[6];
    ICM20948_Read_Multiple_Registers(hICM, ub_0, ICM20948_ACCEL_XOUT_H, raw_data, 6);
    accel_data->x = (int16_t)(raw_data[0] << 8 | raw_data[1]);
    accel_data->y = (int16_t)(raw_data[2] << 8 | raw_data[3]);
    accel_data->z = (int16_t)(raw_data[4] << 8 | raw_data[5]);
}
bool MagAk09916_Read_Mag(ICM20948_HandleTypeDef_SPI *hICM, axises *mag_data)
{
    uint8_t *temp;
    uint8_t drdy, hofl; // data ready, overflow
    
    uint8_t raw_data[6];
    drdy = MagAk09916_Read_Register(hICM, MAG20948_ST1) & 0x01;
    if (!drdy)
    {
        // No new data available
        return false;
    }
    MagAk09916_Read_Multiple_Registers(hICM, MAG20948_HXL, raw_data, 6);
    hofl = MagAk09916_Read_Register(hICM, MAG20948_ST1) & 0x08;
    if (hofl)
    {
        return false;
    }
    mag_data->x = (int16_t)(raw_data[1] << 8 | raw_data[0]);
    mag_data->y = (int16_t)(raw_data[3] << 8 | raw_data[2]);
    mag_data->z = (int16_t)(raw_data[5] << 8 | raw_data[4]);
    return true;
}
void ICM20948_Convert_Gyro(ICM20948_HandleTypeDef_SPI *hICM, axises *gyro_data)
{
    ICM20948_Read_Gyro(hICM, gyro_data);
    gyro_data->x /= gyro_scale_factor;
    gyro_data->y /= gyro_scale_factor;
    gyro_data->z /= gyro_scale_factor;
}
void ICM20948_Convert_Accel(ICM20948_HandleTypeDef_SPI *hICM, axises *accel_data)
{
    ICM20948_Read_Accel(hICM, accel_data);
    accel_data->x /= accel_scale_factor;
    accel_data->y /= accel_scale_factor;
    accel_data->z /= accel_scale_factor;
}
bool MagAk09916_Convert_Mag(ICM20948_HandleTypeDef_SPI *hICM, axises *mag_data)
{
    axises temp;
    bool new_data = MagAk09916_Read_Mag(hICM, &temp);
    if(!new_data)
    {
        return false;
    }
    mag_data->x = temp.x * 0.15f; // Convert to microteslas
    mag_data->y = temp.y * 0.15f;
    mag_data->z = temp.z * 0.15f;
    return true;
}

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
// SUB FUNCTION PROTOTYPE
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
    ICM20948_Write_Register(hICM, ub_0, ICM20948_PWR_MGMT_1, 0x80 | 0x41);
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
    uint8_t new_val = ICM20948_Read_Register(hICM, ub_2, ICM20948_GYRO_CONFIG_1) | (lpf << 3);
    ICM20948_Write_Register(hICM, ub_2, ICM20948_GYRO_CONFIG_1, new_val);
}
void ICM20948_Accel_Low_Pass_Filter(ICM20948_HandleTypeDef_SPI *hICM, uint8_t lpf)
{
    uint8_t new_val = ICM20948_Read_Register(hICM, ub_2, ICM20948_ACCEL_CONFIG) | (lpf << 3);
    ICM20948_Write_Register(hICM, ub_2, ICM20948_ACCEL_CONFIG, new_val);
}
// output data rate
void ICM20948_Gyro_Sample_Rate_Divider(ICM20948_HandleTypeDef_SPI *hICM, uint8_t div)
{
    ICM20948_Write_Register(hICM, ub_2, ICM20948_GYRO_SMPLRT_DIV, div);
}
void ICM20948_Accel_Sample_Rate_Divider(ICM20948_HandleTypeDef_SPI *hICM, uint16_t div)
{
    ICM20948_Write_Register(hICM, ub_2, ICM20948_ACCEL_SMPLRT_DIV_1, (div >> 8) & 0xFF);
    ICM20948_Write_Register(hICM, ub_2, ICM20948_ACCEL_SMPLRT_DIV_2, div & 0xFF);
}
void MagAk09916_Operation_Mode(ICM20948_HandleTypeDef_SPI *hICM, operation_mode_t mode)
{
    MagAk09916_Write_Register(hICM, MAG20948_CNTL2, mode);
}
// Calibration
void ICM20948_Calibrate_Gyro(ICM20948_HandleTypeDef_SPI *hICM)
{
    axises temp;
    int32_t gyro_bias[3] = {0, 0, 0};
    uint8_t gyro_offset_data[6];
    for (int i = 0; i < 100; i++)
    {
        ICM20948_Read_Gyro(hICM, &temp);
        gyro_bias[0] += temp.x;
        gyro_bias[1] += temp.y;
        gyro_bias[2] += temp.z;
        HAL_Delay(10);
    }
    gyro_bias[0] /= 100;
    gyro_bias[1] /= 100;
    gyro_bias[2] /= 100;
    gyro_offset_data[0] = (-gyro_bias[0] / 4) >> 8 & 0XFF;
    gyro_offset_data[1] = (-gyro_bias[0] / 4) & 0XFF;
    gyro_offset_data[2] = (-gyro_bias[1] / 4) >> 8 & 0XFF;
    gyro_offset_data[3] = (-gyro_bias[1] / 4) & 0XFF;
    gyro_offset_data[4] = (-gyro_bias[2] / 4) >> 8 & 0XFF;
    gyro_offset_data[5] = (-gyro_bias[2] / 4) & 0XFF;
    ICM20948_Write_Multiple_Registers(hICM, ub_2, ICM20948_XG_OFFS_USRH, gyro_offset_data, 6);
}
void ICM20948_Calibrate_Accel(ICM20948_HandleTypeDef_SPI *hICMs)
{
    axises temp;
    uint8_t temp2[2];
    uint8_t temp3[2];
    uint8_t temp4[2];
    int32_t accel_bias[3] = {0, 0, 0};
    int32_t accel_bias_reg[3] = {0, 0, 0};
    uint8_t accel_offset_data[6];
    for (int i = 0; i < 100; i++)
    {
        ICM20948_Read_Accel(hICMs, &temp);
        accel_bias[0] += temp.x;
        accel_bias[1] += temp.y;
        accel_bias[2] += temp.z;
        HAL_Delay(10);
    }
    accel_bias[0] /= 100;
    accel_bias[1] /= 100;
    accel_bias[2] /= 100;
    uint8_t mask_bit[3] = {0, 0, 0};
    ICM20948_Read_Multiple_Registers(hICMs, ub_1, ICM20948_XA_OFFS_H, temp2, 2);
    accel_bias_reg[0] = (int32_t)(temp2[0] << 8 | temp2[1]);
    mask_bit[0] = temp2[1] & 0x01;
    ICM20948_Read_Multiple_Registers(hICMs, ub_1, ICM20948_YA_OFFS_H, temp3, 2);
    accel_bias_reg[1] = (int32_t)(temp3[0] << 8 | temp3[1]);
    mask_bit[1] = temp2[1] & 0x01;
    ICM20948_Read_Multiple_Registers(hICMs, ub_1, ICM20948_ZA_OFFS_H, temp4, 2);
    accel_bias_reg[2] = (int32_t)(temp4[0] << 8 | temp4[1]);
    mask_bit[2] = temp2[1] & 0x01;
    accel_bias_reg[0] -= (accel_bias[0] / 8);
    accel_bias_reg[1] -= (accel_bias[1] / 8);
    accel_bias_reg[2] -= (accel_bias[2] / 8);
    accel_offset_data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
    accel_offset_data[1] = (accel_bias_reg[0]) & 0xFF;
    accel_offset_data[1] = accel_offset_data[1] & mask_bit[0];
    accel_offset_data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
    accel_offset_data[3] = (accel_bias_reg[1]) & 0xFF;
    accel_offset_data[3] = accel_offset_data[3] & mask_bit[1];
    accel_offset_data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
    accel_offset_data[5] = (accel_bias_reg[2]) & 0xFF;
    accel_offset_data[5] = accel_offset_data[5] & mask_bit[2];
    ICM20948_Write_Multiple_Registers(hICMs, ub_1, ICM20948_XA_OFFS_H, &accel_offset_data[0], 2);
    ICM20948_Write_Multiple_Registers(hICMs, ub_1, ICM20948_YA_OFFS_H, &accel_offset_data[2], 2);
    ICM20948_Write_Multiple_Registers(hICMs, ub_1, ICM20948_ZA_OFFS_H, &accel_offset_data[4], 2);
}
void ICM20948_Gyro_Full_Scale_Select(ICM20948_HandleTypeDef_SPI *hICM, gyro_fsr_t fsr)
{
    uint8_t new_val = ICM20948_Read_Register(hICM, ub_2, ICM20948_GYRO_CONFIG_1);
    switch (fsr)
    {
    case _250dps:
        new_val |= 0x00;
        gyro_scale_factor = 131.0f;
        break;
    case _500dps:
        gyro_scale_factor = 65.5f;
        new_val |= 0x02; // Set FSR to 500dps
        break;
    case _1000dps:
        gyro_scale_factor = 32.8f;
        new_val |= 0x04; // Set FSR to 1000dps
        break;
    case _2000dps:
        gyro_scale_factor = 16.4f;
        new_val |= 0x06; // Set FSR to 2000dps
        break;
    }
    ICM20948_Write_Register(hICM, ub_2, ICM20948_GYRO_CONFIG_1, new_val);
}
void ICM20948_Accel_Full_Scale_Select(ICM20948_HandleTypeDef_SPI *hICM, accel_fsr_t fsr)
{
    uint8_t new_val = ICM20948_Read_Register(hICM, ub_2, ICM20948_ACCEL_CONFIG);
    switch (fsr)
    {
    case _2g:
        new_val |= 0x00;
        accel_scale_factor = 16384.0f;
        break;
    case _4g:
        new_val |= 0x02; // Set FSR to 4g
        accel_scale_factor = 8192.0f;
        break;
    case _8g:
        new_val |= 0x04; // Set FSR to 8g
        accel_scale_factor = 4096.0f;

        break;
    case _16g:
        new_val |= 0x06; // Set FSR to 16g
        accel_scale_factor = 2048.0f;
        break;
    }
    ICM20948_Write_Register(hICM, ub_2, ICM20948_ACCEL_CONFIG, new_val);
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
static void ICM20948_Read_Multiple_Registers(ICM20948_HandleTypeDef_SPI *hICM, userbank_t bank, uint8_t reg, uint8_t *buffer, uint16_t length)
{
    uint8_t write_data = SPI_READ | reg;
    Select_User_Bank(hICM, bank);
    Cs_Low(hICM);
    HAL_SPI_Transmit(hICM->hspi, &write_data, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(hICM->hspi, buffer, length, HAL_MAX_DELAY);
    Cs_High(hICM);
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
static void MagAk09916_Read_Multiple_Registers(ICM20948_HandleTypeDef_SPI *hICM, uint8_t reg, uint8_t *buffer, uint16_t length)
{
    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_SLV0_ADDR, MAG20948_SLAVE_ADDR | SPI_READ);
    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_SLV0_REG, reg);
    ICM20948_Write_Register(hICM, ub_3, ICM20948_I2C_SLV0_CTRL, 0x80 | length);
    HAL_Delay(10);
    ICM20948_Read_Multiple_Registers(hICM, ub_0, EXT_SLV_SENS_DATA_00, buffer, length);
}
