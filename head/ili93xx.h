#ifndef _ILI93XX_H_
#define _ILI93XX_H_

#include "delay.h"
#include "fsmc.h"
#include "stm32f4xx.h"

#define LCD_WIDTH    320
#define LCD_HEIGHT   240


#define SCREEN_ORIENTATION_LANDSCAPE    1
#define ZXBEE_PLUSE
#define SWAP_RGB (1<<3)

/* lcd 使用PD7脚 NE1功能基地址为0x60000000, PD13引脚A18 作为rs选择*/
#ifdef ZXBEE_PLUSE
#define ILI93xx_REG (*((volatile uint16_t *)(0x60000000)))
#define ILI93xx_DAT (*((volatile uint16_t *)(0x60000000 | (1<<(17+1)))))
#else
//NE4功能基地址为0x6C000000, rs A6
#define ILI93xx_REG (*((volatile uint16_t *)(0x6C000000)))
#define ILI93xx_DAT (*((volatile uint16_t *)(0x6C000000 | (1<<(6+1)))))
#endif


#define RGB888toRGB565(c24) ((unsigned short)(((c24&0x0000f8)>>3)+((c24&0x00fc00)>>5)+((c24&0xf80000)>>8)))

//lcd颜色参数
#define LCD_COLOR_BLACK             0x0000
#define LCD_COLOR_WHITE             0xFFFF

#define LCD_COLOR_GRAY              RGB888toRGB565(0x7F7F7F)    //灰色
#define LCD_COLOR_LIGHT_GRAY        RGB888toRGB565(0xEFEFEF)    //浅灰（更浅）
//#define LCD_COLOR_LIGHT_GRAY        RGB888toRGB565(0xBFBFBF)    //浅灰
#define LCD_COLOR_DARK_GRAY         RGB888toRGB565(0x3F3F3F)    //深灰

#define LCD_COLOR_RED               RGB888toRGB565(0xFF0000)    //红色
#define LCD_COLOR_LIGHT_RED         RGB888toRGB565(0xFF8080)    //浅红色
#define LCD_COLOR_DARK_RED          RGB888toRGB565(0x800000)    //深红色

#define LCD_COLOR_GREEN             RGB888toRGB565(0x00FF00)
#define LCD_COLOR_LIGHT_GREEN       RGB888toRGB565(0x80FF80)    //浅绿色
#define LCD_COLOR_DARK_GREEN        RGB888toRGB565(0x008000)    //深绿色

#define LCD_COLOR_BLUE              RGB888toRGB565(0x0000FF)
#define LCD_COLOR_LIGHT_BLUE        RGB888toRGB565(0x8080FF)    //浅蓝色
#define LCD_COLOR_DARK_BLUE         RGB888toRGB565(0x000080)    //深蓝色

#define LCD_COLOR_YELLOW            RGB888toRGB565(0xFFFF00)    //黄色
#define LCD_COLOR_LIGHT_YELLOW      RGB888toRGB565(0xFFFF80)
#define LCD_COLOR_DARK_YELLOW       RGB888toRGB565(0x808000)

#define LCD_COLOR_PURPLE            RGB888toRGB565(0xFF00FF)    //紫色
#define LCD_COLOR_LIGHT_PURPLE      RGB888toRGB565(0xFF80FF)
#define LCD_COLOR_DARK_PURPLE       RGB888toRGB565(0x800080)

#define LCD_COLOR_CYAN              RGB888toRGB565(0x00FFFF)    //青色
#define LCD_COLOR_LIGHT_CYAN        RGB888toRGB565(0x80FFFF)
#define LCD_COLOR_DARK_CYAN         RGB888toRGB565(0x008080)

#define LCD_COLOR_BROWN             RGB888toRGB565(0x804000)    //棕色
#define LCD_COLOR_LIGHT_BROWN       RGB888toRGB565(0xFF8000)    //浅棕色



/*
//lcd颜色参数
#define LCD_COLOR_BLACK             0x0000
#define LCD_COLOR_WHITE             0xFFFF

#define LCD_COLOR_GRAY              0x8430
#define LCD_COLOR_LIGHT_GRAY        0xC618  //浅灰

#define LCD_COLOR_RED               0xF800

#define LCD_COLOR_GREEN             0x0640
#define LCD_COLOR_LIGHT_GREEN       0X841F  //浅绿色

#define LCD_COLOR_BLUE              0x001F
#define LCD_COLOR_LIGHT_BLUE        0X7D7C  //浅蓝色
#define LCD_COLOR_DARKB_BLUE        0X01CF  //深蓝色

#define LCD_COLOR_YELLOW            0xEF40
#define LCD_COLOR_PURPLE            0xF8EF  //紫色
#define LCD_COLOR_BROWN             0xBC40  //棕色
#define LCD_COLOR_LIGHT_BROWN_BLUE  0x2B12  //浅棕蓝色(选择条目的反色)
*/

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 

#define LIGHTGREEN     	 0X841F //浅绿色
//#define LIGHTGRAY        0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)



void LCD_WR_REG(vu16 regval);
void LCD_WR_DATA(vu16 data);
u16 LCD_RD_DATA(void);
void ILI93xx_WriteReg(uint16_t r, uint16_t d);
uint16_t ILI93xx_ReadReg(uint16_t r);
void BLOnOff(int st);
void BLInit(void);
void ILI93xxInit(void);
void LCDSetWindow(short x, short xe, short y, short ye);
void LCDSetCursor(int x, int y);
void LCDWriteData(uint16_t *dat, int len);
void LCDClear(uint16_t color);
void LCD_DrawPixel(short x, short y, uint32_t color);
void LCDrawLineH(int x0, int x1, int y0, int color);
//void LCDDrawAsciiDot12x24_1(int x, int y, char ch, int color, int bc);
//void LCDDrawAsciiDot12x24(int x, int y, char *str, int color, int bc);
void LCDDrawGB_16_1(int x, int y, char *gb2, int color, int bc) ;
void LCDDrawGB_24_1(int x, int y, char *gb2, int color, int bc) ;
void LCDDrawFnt16(int x, int y, int xs, int xe,char *str, int color, int bc);
void LCDDrawFnt24(int x, int y, char *str, int color, int bc);
void LCD_File(short x1,short y1,short x2,short y2,uint16_t color);

void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u8 *color);
//void ILI93xx_LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
//void ILI93xx_LCDShowFont16(u16 x,u16 y,char* p,u16 width,u16 color,u16 Bcolor);
//void LcdShowIcon(u16 x,u16 y,u8* dat,u8* value);

#endif
