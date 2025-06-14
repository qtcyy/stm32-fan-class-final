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

/* lcd ʹ��PD7�� NE1���ܻ���ַΪ0x60000000, PD13����A18 ��Ϊrsѡ��*/
#ifdef ZXBEE_PLUSE
#define ILI93xx_REG (*((volatile uint16_t *)(0x60000000)))
#define ILI93xx_DAT (*((volatile uint16_t *)(0x60000000 | (1<<(17+1)))))
#else
//NE4���ܻ���ַΪ0x6C000000, rs A6
#define ILI93xx_REG (*((volatile uint16_t *)(0x6C000000)))
#define ILI93xx_DAT (*((volatile uint16_t *)(0x6C000000 | (1<<(6+1)))))
#endif


#define RGB888toRGB565(c24) ((unsigned short)(((c24&0x0000f8)>>3)+((c24&0x00fc00)>>5)+((c24&0xf80000)>>8)))

//lcd��ɫ����
#define LCD_COLOR_BLACK             0x0000
#define LCD_COLOR_WHITE             0xFFFF

#define LCD_COLOR_GRAY              RGB888toRGB565(0x7F7F7F)    //��ɫ
#define LCD_COLOR_LIGHT_GRAY        RGB888toRGB565(0xEFEFEF)    //ǳ�ң���ǳ��
//#define LCD_COLOR_LIGHT_GRAY        RGB888toRGB565(0xBFBFBF)    //ǳ��
#define LCD_COLOR_DARK_GRAY         RGB888toRGB565(0x3F3F3F)    //���

#define LCD_COLOR_RED               RGB888toRGB565(0xFF0000)    //��ɫ
#define LCD_COLOR_LIGHT_RED         RGB888toRGB565(0xFF8080)    //ǳ��ɫ
#define LCD_COLOR_DARK_RED          RGB888toRGB565(0x800000)    //���ɫ

#define LCD_COLOR_GREEN             RGB888toRGB565(0x00FF00)
#define LCD_COLOR_LIGHT_GREEN       RGB888toRGB565(0x80FF80)    //ǳ��ɫ
#define LCD_COLOR_DARK_GREEN        RGB888toRGB565(0x008000)    //����ɫ

#define LCD_COLOR_BLUE              RGB888toRGB565(0x0000FF)
#define LCD_COLOR_LIGHT_BLUE        RGB888toRGB565(0x8080FF)    //ǳ��ɫ
#define LCD_COLOR_DARK_BLUE         RGB888toRGB565(0x000080)    //����ɫ

#define LCD_COLOR_YELLOW            RGB888toRGB565(0xFFFF00)    //��ɫ
#define LCD_COLOR_LIGHT_YELLOW      RGB888toRGB565(0xFFFF80)
#define LCD_COLOR_DARK_YELLOW       RGB888toRGB565(0x808000)

#define LCD_COLOR_PURPLE            RGB888toRGB565(0xFF00FF)    //��ɫ
#define LCD_COLOR_LIGHT_PURPLE      RGB888toRGB565(0xFF80FF)
#define LCD_COLOR_DARK_PURPLE       RGB888toRGB565(0x800080)

#define LCD_COLOR_CYAN              RGB888toRGB565(0x00FFFF)    //��ɫ
#define LCD_COLOR_LIGHT_CYAN        RGB888toRGB565(0x80FFFF)
#define LCD_COLOR_DARK_CYAN         RGB888toRGB565(0x008080)

#define LCD_COLOR_BROWN             RGB888toRGB565(0x804000)    //��ɫ
#define LCD_COLOR_LIGHT_BROWN       RGB888toRGB565(0xFF8000)    //ǳ��ɫ



/*
//lcd��ɫ����
#define LCD_COLOR_BLACK             0x0000
#define LCD_COLOR_WHITE             0xFFFF

#define LCD_COLOR_GRAY              0x8430
#define LCD_COLOR_LIGHT_GRAY        0xC618  //ǳ��

#define LCD_COLOR_RED               0xF800

#define LCD_COLOR_GREEN             0x0640
#define LCD_COLOR_LIGHT_GREEN       0X841F  //ǳ��ɫ

#define LCD_COLOR_BLUE              0x001F
#define LCD_COLOR_LIGHT_BLUE        0X7D7C  //ǳ��ɫ
#define LCD_COLOR_DARKB_BLUE        0X01CF  //����ɫ

#define LCD_COLOR_YELLOW            0xEF40
#define LCD_COLOR_PURPLE            0xF8EF  //��ɫ
#define LCD_COLOR_BROWN             0xBC40  //��ɫ
#define LCD_COLOR_LIGHT_BROWN_BLUE  0x2B12  //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)
*/

//������ɫ
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
#define BROWN 			 0XBC40 //��ɫ
#define BRRED 			 0XFC07 //�غ�ɫ
#define GRAY  			 0X8430 //��ɫ
//GUI��ɫ

#define DARKBLUE      	 0X01CF	//����ɫ
#define LIGHTBLUE      	 0X7D7C	//ǳ��ɫ  
#define GRAYBLUE       	 0X5458 //����ɫ
//������ɫΪPANEL����ɫ 

#define LIGHTGREEN     	 0X841F //ǳ��ɫ
//#define LIGHTGRAY        0XEF5B //ǳ��ɫ(PANNEL)
#define LGRAY 			 0XC618 //ǳ��ɫ(PANNEL),���屳��ɫ

#define LGRAYBLUE        0XA651 //ǳ����ɫ(�м����ɫ)
#define LBBLUE           0X2B12 //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)



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
