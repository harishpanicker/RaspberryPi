/* Variables for temperature calculation */
s32 dig_T1, dig_T2, dig_T3;

/* Variables for pressure calculation */
s64 dig_P1, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;


#define BMP280_REG_TEMP_CALIB1 		0x88
#define BMP280_REG_TEMP_CALIB2 		0x8A
#define BMP280_REG_TEMP_CALIB3 		0x8C

#define BMP280_REG_PRESS_CALIB1 	0x8E
#define BMP280_REG_PRESS_CALIB2 	0x90
#define BMP280_REG_PRESS_CALIB3 	0x92
#define BMP280_REG_PRESS_CALIB4 	0x94
#define BMP280_REG_PRESS_CALIB5 	0x96
#define BMP280_REG_PRESS_CALIB6 	0x98
#define BMP280_REG_PRESS_CALIB7 	0x9A
#define BMP280_REG_PRESS_CALIB8 	0x9C
#define BMP280_REG_PRESS_CALIB9 	0x9E

#define BMP280_REG_TEMP_XLSB		0xFC
#define BMP280_REG_TEMP_LSB		0xFB
#define BMP280_REG_TEMP_MSB		0xFA

#define BMP280_REG_PRESS_XLSB		0xF9
#define BMP280_REG_PRESS_LSB		0xF8
#define BMP280_REG_PRESS_MSB		0xF7

#define BMP280_REG_CONFIG		0xF5
#define BMP280_REG_CTRL_MEAS		0xF4
#define BMP280_REG_STATUS		0xF3
#define BMP280_REG_RESET		0xE0
#define BMP280_REG_ID			0xD0


