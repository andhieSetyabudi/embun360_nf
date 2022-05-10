/*
 * MCP23017_LCD12864.c
 *
 *  Created on: Feb 23, 2022
 *      Author: andhie
 */


#include "MCP23017_LCD12864.h"
#include "BSP.h"
#include "string.h"
#include "math.h"


extern I2C_HandleTypeDef hi2c1;

lcd_var lcd_ = {
		.BL = 0,
		.addr_ = 0x20,
		.i2c_ = &hi2c1,
		.pinIO = 0,
		.flag_ = 1, // failed
};

lcd_ctrl ctrl_lcd = {
		.count_char = 0,

		.font_length_sysinfo = 0,
		.font_characters_space = 0,
		.font_space = 0,
		.font_x = 0,
		.font_y = 0,

		.font_size_x = 0,
		.font_size_y = 0,

		.inverting_text = 0,
		.inverting_space = 0,
		.char_utf_8_1 = 0,
		.char_utf_8_2 = 0,

		.char_count_utf_8 = 0,

		.font_data = NULL,
};

static void lcd_initDriverIO()
{
	if( lcd_.i2c_ == NULL )
		return ;
	lcd_writeRegister(IOCON, 0b00100000);

	//enable all pull up resistors (will be effective for input pins only)
	lcd_writeRegisterPort(GPPU_A, 0xFF, 0xFF);

	// set port mode,
	lcd_writeRegister(IODIR_A , 0x00);     // set all pin as output
	lcd_writeRegister(GPPU_A , 0xFF);
	lcd_writeRegister(IPOL_A , 0x00);

	lcd_writeRegister(IODIR_B , 0x00);     // set all pin as output
	lcd_writeRegister(GPPU_B , 0xFF);
	lcd_writeRegister(IPOL_B , 0x00);
	DWT_Delay_us(10);
	lcd_writeRegister(GPIO_A , 0x10);     // set PSB to high
	lcd_writeRegister(GPIO_B , 0x00);
	DWT_Delay_us(50);
	lcd_writeRegister(GPIO_A , 0x14);

	// init lcd
	lcd_onParallelMode();
	lcd_transfer(0, 0x30);
	DWT_Delay_us(75);
	lcd_transfer(0, 0x34);
	DWT_Delay_us(75);
	lcd_transfer(0, 0x30);
	DWT_Delay_us(75);
	lcd_transfer(0, 0x0C);
	DWT_Delay_us(75);
	lcd_transfer(0, 0x01);
	DWT_Delay_us(75);
	lcd_transfer(0, 0x06);
	DWT_Delay_us(500);
//	HAL_Delay(1);//(1600);

	lcd_transfer(0, 0x34);
	DWT_Delay_us(75);
	lcd_transfer(0, 0x36);
	DWT_Delay_us(75);
	lcd_transfer(0, 0x80);
	DWT_Delay_us(75);
	lcd_transfer(0, 0x80);


}
void lcd_create(I2C_HandleTypeDef *i2c__, uint8_t addr)
{
	lcd_.i2c_=i2c__;
	lcd_.addr_ = addr;

	DWT_Delay_Init();

	lcd_initDriverIO();
	lcd_clear(0, 0);
}


static void lcd_writeRegister(uint8_t reg, uint8_t value)
{
	uint8_t buff[2] = {reg,value};
	//if( HAL_I2C_Master_Transmit_IT(lcd_.i2c_, lcd_.addr_<<1, buff, 2) != HAL_OK )
	if( HAL_I2C_Master_Transmit(lcd_.i2c_, lcd_.addr_<<1, buff, 2, 100)!= HAL_OK)
	{
		HAL_I2C_DeInit(lcd_.i2c_);
		HAL_I2C_Init(lcd_.i2c_);
		lcd_.flag_ = 1; // error
	}
	else
		lcd_.flag_ = HAL_OK;

}


static void lcd_writeRegisterPort(uint8_t reg, uint8_t portA, uint8_t portB)
{
	uint8_t data[3] = { reg,portA, portB };
	//if( HAL_I2C_Master_Transmit_IT(lcd_.i2c_, lcd_.addr_<<1, data,3) != HAL_OK )
	if( HAL_I2C_Master_Transmit(lcd_.i2c_, lcd_.addr_<<1, data, 3, 100) !=HAL_OK)
	{
		HAL_I2C_DeInit(lcd_.i2c_);
		HAL_I2C_Init(lcd_.i2c_);lcd_.flag_ = 1; // error
	}
	else
		lcd_.flag_ = HAL_OK;
}

static void lcd_transfer(uint8_t type_, uint8_t data)
{
    uint8_t ctrl=0x34;      // PSB & RST always pull-up, paralel mode, and enable hi
    if( lcd_.BL == 0 )
        ctrl |= 0x02;
    lcd_writeRegister(GPIO_A , ctrl);
    if( type_  )
    	ctrl |= 128;
    else
    	ctrl &= 0x7f;
    ctrl &= 0xbf;
    lcd_writeRegister(GPIO_A , ctrl);
    lcd_writeRegister(GPIO_B , data);
    ctrl &= 0xdf;
    lcd_writeRegister(GPIO_A , ctrl);
}

static void lcd_onParallelMode(void)
{
    uint8_t val = 0x24;
    lcd_writeRegister(GPIO_A , val);
    DWT_Delay_us(75);
    val = 0x34;
    lcd_writeRegister(GPIO_A , val);
    DWT_Delay_us(75);
    val = 0x14;
    lcd_writeRegister(GPIO_A , val);
    DWT_Delay_us(75);
}



void lcd_fillBuffer(bool fill_type)
{
    memset(ctrl_lcd.frame_buffer, fill_type ? 0xff : 0x0, 1024);
}

void lcd_clearBuffer(void)
{
	lcd_fillBuffer(0);
}

void lcd_clear(bool type_clear, bool fill_type)
{
	lcd_initDriverIO();
    if (type_clear == 0)
    {
        for (uint8_t i = 0; i < 32; i++) {
        	lcd_transfer(0, 0x80 | i);
        	lcd_transfer(0, 0x80);
            for (uint8_t i = 0; i < 32; i++) {
            	lcd_transfer(1, fill_type ? 0xff : 0x00);
            }
        }
    }
    else if (type_clear == 1)
    {
        for(uint8_t count=0;count <= 8;count+=8){
            for (uint8_t i = 0; i < 32; i++) {
            	lcd_transfer(0, 0x80 | i);
            	lcd_transfer(0, 0x80 | count);
                for (uint8_t i = 0; i < 16; i++) {
                	lcd_transfer(1, fill_type ? 0xff : 0x00);
                }
            }
        }
    }
}


void lcd_sendBuffer(bool type_flush)
{
	if( lcd_.flag_ != HAL_OK )
	{
		lcd_initDriverIO();
		lcd_clear(0, 0);
	};

	uint8_t temp_buffer = 0x0;
	if (type_flush == 0)
	{
		for (uint8_t row = 0; row < 32; row++) {
			lcd_transfer(0, 0x80 | row);
			lcd_transfer(0, 0x80);
			for (uint8_t ByteBuffer = 0 ; ByteBuffer < 16; ByteBuffer++) {
				temp_buffer = ctrl_lcd.frame_buffer[row][ByteBuffer];
				lcd_transfer(1, temp_buffer);
				temp_buffer = 0x0;
			}
			for (uint8_t ByteBuffer = 0 ; ByteBuffer < 16; ByteBuffer++) {
				temp_buffer = ctrl_lcd.frame_buffer[row + 32][ByteBuffer];
				lcd_transfer(1, temp_buffer);
				temp_buffer = 0x0;
			}
			if( lcd_.flag_ != HAL_OK )
			{
				lcd_initDriverIO();
				lcd_clear(0, 0);
			};
		}
	}
	else if (type_flush == 1)
	{
		for (uint8_t row = 0; row < 64; row++) {
			if ( row < 32) {
				lcd_transfer(0, 0x80 | row );
				lcd_transfer(0, 0x80);
			} else {
				lcd_transfer(0, 0x80 | row - 32);
				lcd_transfer(0, 0x80 | 8);
			}
			for (uint8_t ByteBuffer = 0 ; ByteBuffer < 16; ByteBuffer++) {
				temp_buffer = ctrl_lcd.frame_buffer[row][ByteBuffer];
				lcd_transfer(1, temp_buffer );
				temp_buffer = 0x0;
			}
			if( lcd_.flag_ != HAL_OK )
			{
				lcd_initDriverIO();
				lcd_clear(0, 0);
			};
		}
	}
}

void lcd_setBL_off(void)
{
    // backlight pin on PA1, active low
	lcd_.BL = 1;
    uint8_t val = 0x14;
    lcd_writeRegister(GPIO_A,val);
}


void lcd_setBL_on(void)
{
    // backlight pin on PA1, active low
	lcd_.BL = 0;
    uint8_t val = 0x16;
    lcd_writeRegister(GPIO_A,val);
}


//-=================== LCD base command and control

uint16_t lcdBase_utf8(uint8_t char_utf_8_data)
{
    uint16_t char_utf_8 = 0;

    if (ctrl_lcd.char_count_utf_8 == 0){

    	ctrl_lcd.char_utf_8_1 |= char_utf_8_data;
    	ctrl_lcd.char_count_utf_8  = 1;
    }
    else if (ctrl_lcd.char_count_utf_8 == 1){

    	ctrl_lcd.char_utf_8_2 |= char_utf_8_data;
        ctrl_lcd.char_count_utf_8  = 2;
    }
    if (ctrl_lcd.char_count_utf_8 == 2){

    	char_utf_8 |= ctrl_lcd.char_utf_8_1;
        char_utf_8 = char_utf_8<<8;
        char_utf_8 |= ctrl_lcd.char_utf_8_2;

        ctrl_lcd.char_utf_8_1 = 0;
        ctrl_lcd.char_utf_8_2 = 0;
        ctrl_lcd.char_count_utf_8 = 0;

        return char_utf_8;
    }
    else return 0;
}


void lcdBase_write(uint8_t character)
{
	 if(character - 32 == 0){
	        lcdBase_drawVLine(ctrl_lcd.font_x, ctrl_lcd.font_y, ctrl_lcd.font_size_y, ctrl_lcd.font_space, ctrl_lcd.inverting_space);
	        ctrl_lcd.font_x += ctrl_lcd.font_space - 1;
	}
	else if(character <= 200){
		lcdBase_write_char(character);
	} else {
		uint16_t character_utf8 = lcdBase_utf8(character);

		if ( character_utf8 >= 0xD090 && character_utf8 <= 0xD0BF){
			character_utf8 -= 53200;
		}
		else if(character_utf8 >= 0xD180 && character_utf8 <= 0xD18F){
			character_utf8 -= 53392;
		}

		if(character_utf8 != 0){
			lcdBase_write_char(character_utf8);
		}
	}
}

void lcdBase_write_char(uint16_t character)
{
	character -= 32;
	uint8_t temp_data;
	uint8_t count_space = 0;
	uint32_t count_var;

	for (uint8_t count_x = 0; count_x < ctrl_lcd.font_size_x; count_x++){
		count_var = count_x + ctrl_lcd.font_length_sysinfo + (character * ctrl_lcd.font_size_x);

		if (ctrl_lcd.font_data[count_var] != 0){
			for (uint8_t count_y = 0; count_y < ctrl_lcd.font_size_y; count_y++){
				temp_data = ((ctrl_lcd.font_data[count_var] << 7-count_y) >> 7) & 1;
				if(ctrl_lcd.inverting_text) temp_data ^= 1;
				lcdBase_drawPixel(ctrl_lcd.font_x + count_x - count_space, ctrl_lcd.font_y + count_y, temp_data);
			}
			ctrl_lcd.count_char++;
		}
		else if (ctrl_lcd.font_data[count_var + 1] == 0){
			count_space++;//=ctrl_lcd.font_characters_space;
		}
	}
	ctrl_lcd.font_x += ctrl_lcd.count_char + ctrl_lcd.font_characters_space;
	lcdBase_drawVLine(ctrl_lcd.font_x, ctrl_lcd.font_y, ctrl_lcd.font_size_y, ctrl_lcd.font_characters_space, ctrl_lcd.inverting_space);
	ctrl_lcd.count_char = 0;
}

size_t lcdBase_getWitdthStr(const uint8_t* buffer, size_t size_)
{
	size_t ret = 0;
	uint16_t character;
	uint32_t count_var;
	uint8_t count_char= 0;
	uint8_t count_space = 0;
	for( uint8_t ui = 0; ui < size_ ; ui++)
	{
		if( buffer[ui] - 32 == 0 )
			ret += ctrl_lcd.font_space - 1;
		else
		{
			count_space = 0;
			count_char= 0;
			character = buffer[ui]-32;
			for (uint8_t count_x = 0; count_x < ctrl_lcd.font_size_x; count_x++){
				count_var = count_x + ctrl_lcd.font_length_sysinfo + (character * ctrl_lcd.font_size_x);
				if (ctrl_lcd.font_data[count_var] != 0){
					count_char++;
				}
			}
			ret += count_char + ctrl_lcd.font_characters_space;
		}
	}
	return (ret-ctrl_lcd.font_characters_space);
}


size_t lcdBase_write_str(const uint8_t *buffer, size_t size)
{
  size_t n = 0;
  while (n<size) {
    lcdBase_write(buffer[n]);//)
    n++;
  }
  return n;
}


void lcdBase_setFont(const uint8_t* data)
{
	ctrl_lcd.font_length_sysinfo = data[0];
	ctrl_lcd.font_characters_space = data[1];
	ctrl_lcd.font_space = data[2];
	ctrl_lcd.font_size_x = data[3];
	ctrl_lcd.font_size_y = data[4];
	ctrl_lcd.font_data = (uint8_t*) data;
}

void lcdBase_invertingText(bool _inverting_text, bool _inverting_space)
{
	ctrl_lcd.inverting_text = _inverting_text;
	ctrl_lcd.inverting_space = _inverting_space;
}

void lcdBase_drawText(uint8_t x, uint8_t y, char* data)
{
	ctrl_lcd.count_char = 0;
	ctrl_lcd.font_x = x;
	ctrl_lcd.font_y = y;
	lcdBase_write_str((const uint8_t*)data, strlen(data));
    ctrl_lcd.count_char = 0;
}


void lcdBase_resumeDrawText(char* data)
{
	lcdBase_write_str((const uint8_t*)data, strlen(data));
}


void lcdBase_drawPixel(uint8_t x, uint8_t y, uint8_t pixel)
{
	 if (x <= 127 && y <= 63) {
		uint8_t x_byte = x/8 ;
		uint8_t x_bit = x%8;

		if(pixel != 0 ){ // if pixel = 1

			ctrl_lcd.frame_buffer[y][x_byte] |= (1 << (7 - x_bit));
		}
		else { // if pixel not = 1
			ctrl_lcd.frame_buffer[y][x_byte] &= ~(1 << (7 - x_bit));
		}
	}
}

void lcdBase_drawHLine(uint8_t x, uint8_t y, uint8_t end_line, uint8_t line_width, uint8_t pixel)
{
    for (int i = 0; i < end_line; i++) {
        for (int i2 = 0; i2 < line_width; i2++) {
        	lcdBase_drawPixel(x + i, y + i2, pixel);
        }
    }
}

void lcdBase_drawVLine(uint8_t x, uint8_t y, uint8_t end_line, uint8_t line_width, uint8_t pixel)
{
    for (int i = 0; i < end_line; i++) {
        for (int i2 = 0; i2 < line_width; i2++) {
        	lcdBase_drawPixel(x + i2, y + i, pixel);
        }
    }
}

void lcdBase_drawBox(uint8_t x, uint8_t y, uint8_t end_x, uint8_t end_y, uint8_t border_width, uint8_t pixel)
{
	lcdBase_drawHLine(x, y, end_x, border_width , pixel);
	lcdBase_drawHLine(x, y + end_y - border_width, end_x, border_width , pixel);

	lcdBase_drawVLine(x, y, end_y, border_width , pixel);
	lcdBase_drawVLine(x + end_x - border_width, y, end_y, border_width , pixel);
}

void lcdBase_drawFillBox(uint8_t x, uint8_t y, uint8_t end_x, uint8_t end_y, uint8_t pixel)
{
	lcdBase_drawHLine(x, y, end_x, end_y , pixel);
}

void lcdBase_drawCircle(uint8_t x, uint8_t y, uint8_t radius, uint8_t pixel)
{
    /*из wikipedia*/
    int x0 = x;
    int y0 = y;

    x = radius;
    y = 0;
    int dx = 1 - (radius <<1);
    int dy =  0;
    int err = 0;

    while (x >= y)
    {
    	lcdBase_drawPixel(x0 + x, y0 + y, pixel);
    	lcdBase_drawPixel(x0 + y, y0 + x, pixel);
    	lcdBase_drawPixel(x0 - y, y0 + x, pixel);
    	lcdBase_drawPixel(x0 - x, y0 + y, pixel);
    	lcdBase_drawPixel(x0 - x, y0 - y, pixel);
    	lcdBase_drawPixel(x0 - y, y0 - x, pixel);
    	lcdBase_drawPixel(x0 + y, y0 - x, pixel);
    	lcdBase_drawPixel(x0 + x, y0 - y, pixel);

        y++;
        err += dy;
        dy += 2;

        if ((err << 1) + dx > 0)
        {
            x--;
            err += dx;
            dx += 2;
        }
    }
}

void lcdBase_drawFillCircle(uint8_t x, uint8_t y, uint8_t radius, uint8_t pixel)
{
    int x0 = radius;
    int y0 = 0;
    int xChange = 1 - (radius << 1);
    int yChange = 0;
    int radiusError = 0;

    while (x0 >= y0)
    {
        for (int i = x - x0; i <= x + x0; i++)
        {
        	lcdBase_drawPixel(i, y + y0, pixel);
        	lcdBase_drawPixel(i, y - y0, pixel);
        }
        for (int i = x - y0; i <= x + y0; i++)
        {
        	lcdBase_drawPixel(i, y + x0, pixel);
        	lcdBase_drawPixel(i, y - x0, pixel);
        }

        y0++;
        radiusError += yChange;
        yChange += 2;
        if (((radiusError << 1) + xChange) > 0)
        {
            x0--;
            radiusError += xChange;
            xChange += 2;
        }
    }
}


void lcdBase_drawBitmap(uint8_t x, uint8_t y, uint8_t size_x, uint8_t size_y, const uint8_t *bitmap_buffer, uint8_t pixel)
{
    uint8_t x_byte = x/8 ;
    uint8_t x_bit = x%8;

    if (x == 0 && y == 0 && size_x == 128 && size_y == 64)
    {
        int i = 0;
        for (int i2 = 0; i2 < 64; i2++) {
            for (int i3 = 0; i3 < 16; i3++) {
                if(pixel == 0) ctrl_lcd.frame_buffer[i2][i3] &= ~bitmap_buffer[i];
                else
                if(pixel == 1) ctrl_lcd.frame_buffer[i2][i3] |= bitmap_buffer[i];

                i++;
            }
        }
    }
    else if ((x_bit == 0) && (x + size_x <= 128))
    {
        int i3 = 0 ;
        for (int i = 0; i < size_y - y; i++) {
            for (int i2 = 0; i2 < round((size_x - x) / 8); i2++) {

                if(pixel == 0) ctrl_lcd.frame_buffer[i + y][i2 + x / 8 ] &= ~bitmap_buffer[i3];
                else
                if(pixel == 1) ctrl_lcd.frame_buffer[i + y][i2 + x / 8 ] |= bitmap_buffer[i3];

                i3++;
            }
        }
    }
    else if ((x_bit != 0) || ((x_bit == 0) && (x + size_x >= 128)))
    {
        uint8_t temp_bitmap;
        int CountBuffer = 0;

        for (int CountY = 0; CountY < size_y; CountY++) {
            for (int i = 0; i < round((float)size_x / 8.f); i++) {
                for (int i2 = 0; i2 < 8; i2++) {

                    if(pixel == 0) temp_bitmap = ~bitmap_buffer[i + CountBuffer];
                    else
                    if(pixel == 1) temp_bitmap = bitmap_buffer[i + CountBuffer];

                    temp_bitmap <<= i2;
                    temp_bitmap >>= 7;
                    lcdBase_drawPixel(x + i2 + i * 8, y + CountY, temp_bitmap);
                }
            }
            CountBuffer += round((float)size_x / 8.f);
        }
    }
}


lcd_def lcd =
{
		lcd_create,
		lcd_clearBuffer,
		lcd_clear,
		lcd_setBL_on,
		lcd_setBL_off,
		lcd_sendBuffer,

		// lcd base function
		lcdBase_setFont,
		lcdBase_utf8,
		lcdBase_write,
		lcdBase_write_char,
		lcdBase_write_str,

		lcdBase_invertingText,
		lcdBase_drawText,

		lcdBase_drawPixel,

		lcdBase_drawHLine,
		lcdBase_drawVLine,
		lcdBase_drawBox,
		lcdBase_drawFillBox,
		lcdBase_drawCircle,
		lcdBase_drawFillCircle,
		lcdBase_drawBitmap,

		lcdBase_getWitdthStr,
};
