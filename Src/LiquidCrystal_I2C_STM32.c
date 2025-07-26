#include "LiquidCrystal_I2C_STM32.h"
#include "main.h"  // for delayMicroseconds


static void LCD_ExpanderWrite(LCD_I2C *lcd, uint8_t data);
static void LCD_Write4Bits(LCD_I2C *lcd, uint8_t value);
static void LCD_PulseEnable(LCD_I2C *lcd, uint8_t data);
static void LCD_Send(LCD_I2C *lcd, uint8_t value, uint8_t mode);
extern void delayMicroseconds(uint32_t us);  // must be implemented by user

void LCD_Init(LCD_I2C *lcd, I2C_HandleTypeDef *hi2c, uint8_t addr, uint8_t cols, uint8_t rows, uint8_t charsize) {
    lcd->hi2c = hi2c;
    lcd->addr = addr << 1; // STM32 HAL expects 8-bit address
    lcd->cols = cols;
    lcd->rows = rows;
    lcd->charsize = charsize;
    lcd->backlightval = LCD_BACKLIGHT;

    lcd->displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    if (rows > 1) lcd->displayfunction |= LCD_2LINE;
    if ((charsize != 0) && (rows == 1)) lcd->displayfunction |= LCD_5x10DOTS;

    HAL_Delay(50); // wait for LCD to power up

    LCD_ExpanderWrite(lcd, lcd->backlightval);
    HAL_Delay(1000);

    // Initialization sequence
    LCD_Write4Bits(lcd, 0x30);
    delayMicroseconds(4500);
    LCD_Write4Bits(lcd, 0x30);
    delayMicroseconds(4500);
    LCD_Write4Bits(lcd, 0x30);
    delayMicroseconds(150);
    LCD_Write4Bits(lcd, 0x20); // set to 4-bit mode

    LCD_Command(lcd, LCD_FUNCTIONSET | lcd->displayfunction);

    lcd->displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    LCD_DisplayOn(lcd);
    LCD_Clear(lcd);

    lcd->displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    LCD_Command(lcd, LCD_ENTRYMODESET | lcd->displaymode);
    LCD_Home(lcd);
}

void LCD_Clear(LCD_I2C *lcd) {
    LCD_Command(lcd, LCD_CLEARDISPLAY);
    delayMicroseconds(2000);
}

void LCD_Home(LCD_I2C *lcd) {
    LCD_Command(lcd, LCD_RETURNHOME);
    delayMicroseconds(2000);
}

void LCD_SetCursor(LCD_I2C *lcd, uint8_t col, uint8_t row) {
    static uint8_t row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
    if (row >= lcd->rows) row = lcd->rows - 1;
    LCD_Command(lcd, LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void LCD_DisplayOn(LCD_I2C *lcd) {
    lcd->displaycontrol |= LCD_DISPLAYON;
    LCD_Command(lcd, LCD_DISPLAYCONTROL | lcd->displaycontrol);
}

void LCD_DisplayOff(LCD_I2C *lcd) {
    lcd->displaycontrol &= ~LCD_DISPLAYON;
    LCD_Command(lcd, LCD_DISPLAYCONTROL | lcd->displaycontrol);
}

void LCD_CursorOn(LCD_I2C *lcd) {
    lcd->displaycontrol |= LCD_CURSORON;
    LCD_Command(lcd, LCD_DISPLAYCONTROL | lcd->displaycontrol);
}

void LCD_CursorOff(LCD_I2C *lcd) {
    lcd->displaycontrol &= ~LCD_CURSORON;
    LCD_Command(lcd, LCD_DISPLAYCONTROL | lcd->displaycontrol);
}

void LCD_BlinkOn(LCD_I2C *lcd) {
    lcd->displaycontrol |= LCD_BLINKON;
    LCD_Command(lcd, LCD_DISPLAYCONTROL | lcd->displaycontrol);
}

void LCD_BlinkOff(LCD_I2C *lcd) {
    lcd->displaycontrol &= ~LCD_BLINKON;
    LCD_Command(lcd, LCD_DISPLAYCONTROL | lcd->displaycontrol);
}

void LCD_ScrollLeft(LCD_I2C *lcd) {
    LCD_Command(lcd, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LCD_ScrollRight(LCD_I2C *lcd) {
    LCD_Command(lcd, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void LCD_LeftToRight(LCD_I2C *lcd) {
    lcd->displaymode |= LCD_ENTRYLEFT;
    LCD_Command(lcd, LCD_ENTRYMODESET | lcd->displaymode);
}

void LCD_RightToLeft(LCD_I2C *lcd) {
    lcd->displaymode &= ~LCD_ENTRYLEFT;
    LCD_Command(lcd, LCD_ENTRYMODESET | lcd->displaymode);
}

void LCD_AutoscrollOn(LCD_I2C *lcd) {
    lcd->displaymode |= LCD_ENTRYSHIFTINCREMENT;
    LCD_Command(lcd, LCD_ENTRYMODESET | lcd->displaymode);
}

void LCD_AutoscrollOff(LCD_I2C *lcd) {
    lcd->displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    LCD_Command(lcd, LCD_ENTRYMODESET | lcd->displaymode);
}

void LCD_CreateChar(LCD_I2C *lcd, uint8_t location, uint8_t charmap[]) {
    location &= 0x7;
    LCD_Command(lcd, LCD_SETCGRAMADDR | (location << 3));
    for (int i = 0; i < 8; i++) {
        LCD_Write(lcd, charmap[i]);
    }
}

void LCD_BacklightOn(LCD_I2C *lcd) {
    lcd->backlightval = LCD_BACKLIGHT;
    LCD_ExpanderWrite(lcd, 0);
}

void LCD_BacklightOff(LCD_I2C *lcd) {
    lcd->backlightval = LCD_NOBACKLIGHT;
    LCD_ExpanderWrite(lcd, 0);
}

void LCD_Print(LCD_I2C *lcd, const char *str) {
    while (*str) {
        LCD_Write(lcd, *str++);
    }
}

void LCD_Write(LCD_I2C *lcd, uint8_t value) {
    LCD_Send(lcd, value, Rs);
}

void LCD_Command(LCD_I2C *lcd, uint8_t value) {
    LCD_Send(lcd, value, 0);
}

static void LCD_Send(LCD_I2C *lcd, uint8_t value, uint8_t mode) {
    uint8_t highnib = value & 0xF0;
    uint8_t lownib  = (value << 4) & 0xF0;
    LCD_Write4Bits(lcd, highnib | mode);
    LCD_Write4Bits(lcd, lownib  | mode);
}

static void LCD_Write4Bits(LCD_I2C *lcd, uint8_t value) {
    LCD_ExpanderWrite(lcd, value);
    LCD_PulseEnable(lcd, value);
}

static void LCD_ExpanderWrite(LCD_I2C *lcd, uint8_t data) {
    uint8_t buf = data | lcd->backlightval;
    HAL_I2C_Master_Transmit(lcd->hi2c, lcd->addr, &buf, 1, HAL_MAX_DELAY);
}

static void LCD_PulseEnable(LCD_I2C *lcd, uint8_t data) {
    LCD_ExpanderWrite(lcd, data | En);
    delayMicroseconds(1);
    LCD_ExpanderWrite(lcd, data & ~En);
    delayMicroseconds(50);
}
