
#include "stm32f1xx_hal.h"
#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"
#include "stdarg.h"
#include "stdio.h"

//OLED���Դ�
//��Ÿ�ʽ����.
//[0]0 1 2 3 ... 127    
//[1]0 1 2 3 ... 127    
//[2]0 1 2 3 ... 127    
//[3]0 1 2 3 ... 127    
//[4]0 1 2 3 ... 127    
//[5]0 1 2 3 ... 127    
//[6]0 1 2 3 ... 127    
//[7]0 1 2 3 ... 127     
uint8_t OLED_GRAM[128][8]={0};//OLED�Դ�

#if OLED_MODE==1
//��SSD1106д��һ���ֽڡ�
//dat:Ҫд�������/����
//cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
void OLED_WR_Byte(uint8_t dat,uint8_t cmd)
{
    DATAOUT(dat);        
    if(cmd)
      OLED_DC_Set();
    else 
      OLED_DC_Clr();           
    OLED_CS_Clr();
    OLED_WR_Clr();     
    OLED_WR_Set();
    OLED_CS_Set();      
    OLED_DC_Set();     
}                 
#else
//��SSD1106д��һ���ֽڡ�
//dat:Ҫд�������/����
//cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
void OLED_WR_Byte(uint8_t dat,uint8_t cmd)
{    
    uint8_t i;              
    if(cmd)
      OLED_DC_Set();
    else 
      OLED_DC_Clr();          
    OLED_CS_Clr();
    for(i=0;i<8;i++)
    {              
        OLED_SCLK_Clr();
        if(dat&0x80)
           OLED_SDIN_Set();
        else 
           OLED_SDIN_Clr();
        OLED_SCLK_Set();
        dat<<=1;   
    }                           
    OLED_CS_Set();
    OLED_DC_Set();         
} 
#endif
    void OLED_Set_Pos(uint8_t x, uint8_t y) 
{ 
    OLED_WR_Byte(0xb0+y,OLED_CMD);
    OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
    OLED_WR_Byte((x&0x0f)|0x01,OLED_CMD); 
}         
//����OLED��ʾ    
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC����
    OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
    OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//�ر�OLED��ʾ     
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC����
    OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
    OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}                        
//��������,������,������Ļ�Ǻ�ɫ��!��û����һ��!!!      
void OLED_Clear(void)  
{  
    uint8_t i,n;            
    for(i=0;i<8;i++)  
    {  
        OLED_WR_Byte (0xb0+i,OLED_CMD);    //����ҳ��ַ��0~7��
        OLED_WR_Byte (0x02,OLED_CMD);      //������ʾλ�á��е͵�ַ
        OLED_WR_Byte (0x10,OLED_CMD);      //������ʾλ�á��иߵ�ַ   
        for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA); 
    } //������ʾ
}


//��ָ��λ����ʾһ���ַ�,���������ַ�
//x:0~127
//y:0~63
//mode:0,������ʾ;1,������ʾ                 
//size:ѡ������ 16/12 
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr)
{          
    uint8_t c=0,i=0;    
    c=chr-' ';//�õ�ƫ�ƺ��ֵ            
    if(x>Max_Column-1)
    {
        x=0;
        y=y+2;
    }
    if(SIZE ==16)
    {
        OLED_Set_Pos(x,y);    
        for(i=0;i<8;i++)
        OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
        OLED_Set_Pos(x,y+1);
        for(i=0;i<8;i++)
        OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
    }
    else
    {    
        OLED_Set_Pos(x,y);  //12������ 6*8 �����y����+1
        for(i=0;i<6;i++)
        {
            OLED_WR_Byte(F6x8[c][i],OLED_DATA);
        }
    }
}

/*OLED�ַ���ʾ����*/
void OLED_DisplayChar(uint8_t x,uint8_t y,uint8_t chr)
{
    static int8_t col = 0;
    if(SIZE == 12)
    {
        for(col = 0;col < 6;col++)
        {
            x = x > 20?20:x;
            OLED_GRAM[x*6+col][y] |= F6x8[chr - 0x20][col];
        }
    }
}

//m^n����
uint32_t oled_pow(uint8_t m,uint8_t n)
{
    uint32_t result=1;     
    while(n--)result*=m;    
    return result;
}                  
//��ʾ2������
//x,y :�������     
//len :���ֵ�λ��
//size:�����С
//mode:ģʽ    0,���ģʽ;1,����ģʽ
//num:��ֵ(0~4294967295);               
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size)
{             
    uint8_t t,temp;
    uint8_t enshow=0;                           
    for(t=0;t<len;t++)
    {
        temp=(num/oled_pow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                OLED_ShowChar(x+(size/2)*t,y,' ');
                continue;
            }else enshow=1; 
              
        }
         OLED_DisplayChar(x+(size/2)*t,y,temp+'0'); 
    }
} 
//��ʾһ���ַ��Ŵ�
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr)
{
    uint8_t j=0;
    while (chr[j]!='\0')
    {    
        OLED_DisplayChar(x,y,chr[j]);
        if(SIZE == 12)
        {
            //����OLED_DisplayChar���x+1����
            x+=1;
            //����OLED_ShowChar���x+=6
            //x+=6;//һ����ʾ21������
        }
        else if(SIZE == 16)
        {
            x+=8;
        }
        if(x>120)
        {
            x=0;
            y+=2;
        }
        j++;
    }
}
//��ʾ����
void OLED_ShowCHinese(uint8_t x,uint8_t y,uint8_t no)
{                      
    uint8_t t,adder=0;
    OLED_Set_Pos(x,y);    
    for(t=0;t<16;t++)
        {
                OLED_WR_Byte(Hzk[2*no][t],OLED_DATA);
                adder+=1;
     }    
        OLED_Set_Pos(x,y+1);    
    for(t=0;t<16;t++)
            {    
                OLED_WR_Byte(Hzk[2*no+1][t],OLED_DATA);
                adder+=1;
      }                    
}
/***********������������ʾ��ʾBMPͼƬ128��64��ʼ������(x,y),x�ķ�Χ0��127��yΪҳ�ķ�Χ0��7*****************/
void OLED_DrawBMP(uint8_t x0, uint8_t y0,uint8_t x1, uint8_t y1,uint8_t BMP[])
{     
 uint16_t j=0;
 uint8_t x,y;
  
  if(y1%8==0) y=y1/8;      
  else y=y1/8+1;
    for(y=y0;y<y1;y++)
    {
        OLED_Set_Pos(x0,y);
    for(x=x0;x<x1;x++)
        {      
            OLED_WR_Byte(BMP[j++],OLED_DATA);            
        }
    }
} 


//��ʼ��SSD1306                        
void OLED_Init(void)
{
 
  OLED_RST_Set();
    HAL_Delay(100);
    OLED_RST_Clr();
    HAL_Delay(100);
    OLED_RST_Set(); 
                      
    OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
    OLED_WR_Byte(0x02,OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
    OLED_WR_Byte(0xCF,OLED_CMD); // Set SEG Output Current Brightness
    OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0���ҷ��� 0xa1����
    OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0���·��� 0xc8����
    OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
    OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
        OLED_WR_Byte(0x81,OLED_CMD); //�Աȶ�����
    OLED_WR_Byte(0xfF,OLED_CMD); //1~255;Ĭ��0X7F (��������,Խ��Խ��)
    OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset    Shift Mapping RAM Counter (0x00~0x3F)
    OLED_WR_Byte(0x00,OLED_CMD);//-not offset
    OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
    OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
    OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
    OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
    OLED_WR_Byte(0x12,OLED_CMD);
    OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
    OLED_WR_Byte(0x40,OLED_CMD);//Set VCOM Deselect Level
    OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
    OLED_WR_Byte(0x02,OLED_CMD);//
    OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
    OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
    OLED_WR_Byte(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
    OLED_WR_Byte(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7) 
    OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel
    
    OLED_WR_Byte(0xAF,OLED_CMD); /*display ON*/ 
    OLED_Clear();
    OLED_Set_Pos(0,0);     
}  

/*OLED���㺯����writeΪ1����䣬Ϊ0�����*/
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t write)
{
    uint8_t page,temp_page,sub_page_data = 0;    //������Ҳ����x�᷽���Ϊ128���������귽���Ϊ8ҳ��0~7����ÿСҳ�����Ϊ8�����ص��ö���������ʾ
    if((x > 127) || (y > 63))
    {
        return;   //������Χ������ʧ��
    }
    page = 7 - y / 8;
    temp_page = y % 8;
    sub_page_data = 1 << (7 - temp_page);
    OLED_Set_Pos(x,page);
    if(write == 1)
    {
        OLED_GRAM[x][page] |= sub_page_data;
    }
    else
    {
        OLED_GRAM[x][page] &= ~sub_page_data;
    }
}

/*OLED���ߺ���*/
void OLED_DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2)
{
    static int8_t x_width = 0;
    static int8_t y_height = 0;
    static int8_t x_add = 0;
    static int8_t y_add = 0;
    static int8_t current_remainder = 0;
    uint8_t start = 0,end = 0;
    uint8_t i;
    if(y1 == y2)//����ˮƽ��
    {
        if(x1 > x2)
        {
            start = x2;
            end = x1;
        }
        else 
        {
            start = x1;
            end = x2;
        }
        for(i = start;i < end;i++)
        {
            OLED_DrawPoint(i,y1,1);
        }
    }
    else if(x1 == x2)//����Ǧ����
    {
        if(y1 > y2)
        {
            start = y2;
            end = y1;
        }
        else
        {
            start = y1;
            end = y2;
        }
        for(i = start;i < end;i++)
        {
            OLED_DrawPoint(x1,i,1);
        }
    }
    else//������������
    {
        x_width = x2 - x1;
        y_height = y2 - y1;
        if(x_width < 0)
        {
            x_width = 0 - x_width;
        }
        if(y_height < 0)
        {
            y_height = 0 - y_height;
        }
        x_add = (x2 > x1)?1:-1;
        y_add = (y2 > y1)?1:-1;
        if(x_width >= y_height)
        {
            current_remainder = x_width / 2;
            for(;x1 != x2;x1 += x_add)
            {
                OLED_DrawPoint(x1,y1,1);
                current_remainder += y_height;
                if(current_remainder >= x_width)
                {
                    current_remainder -= x_width;
                    y1 += y_add;
                }
            }
        }
        else
        {
            current_remainder = y_height / 2;
            for(;y1 != y2;y1 += y_add)
            {
                OLED_DrawPoint(x1,y1,1);
                current_remainder += x_width;
                if(current_remainder >= y_height)
                {
                    current_remainder -= y_height;
                    x1 += x_add;
                }
            }
        }
    }
}

/*OLED���麯��*/
void OLED_DrawBlock(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2)
{
    OLED_DrawLine(x1,y1,x1,y2);
    OLED_DrawLine(x2,y1,x2,y2);
    OLED_DrawLine(x1,y1,x2,y1);
    OLED_DrawLine(x1,y2,x2,y2);
}

/*OLED�����κ���*/
void OLED_DrawRect(uint8_t x1,uint8_t y1,uint8_t width,uint8_t height,uint8_t fill)
{
    uint8_t rect_x1 = x1;
    uint8_t rect_y1 = y1;
    uint8_t rect_x2 = x1 + width -1;
    uint8_t rect_y2 = y1 + height -1;
    
    uint8_t i = 0,j = 0;
    
    OLED_DrawBlock(rect_x1,rect_y1,rect_x2,rect_y2);
    
    if(fill == 1)
    {
        for(i = x1;i < (rect_x2 +1);i++)
        {
            for(j = y1;j < (rect_y2 + 1);j++)
            {
                OLED_DrawPoint(i,j,1);
            }
        }
    }
}

void OLED_CirclePoint(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2)
{
    OLED_DrawPoint((x1+x2),(y1+y2),1);
    OLED_DrawPoint((x1+y2),(y1+x2),1);
    OLED_DrawPoint((x1-y2),(y1+x2),1);
    OLED_DrawPoint((x1-x2),(y1+y2),1);
    OLED_DrawPoint((x1-x2),(y1-y2),1);
    OLED_DrawPoint((x1-y2),(y1-x2),1);
    OLED_DrawPoint((x1+y2),(y1-x2),1);
    OLED_DrawPoint((x1+x2),(y1-y2),1);
}

/*OLED��Բ�����Σ�����*/
void OLED_DrawCircle(uint8_t x,uint8_t y,uint8_t r)
{
    uint8_t tx = 0, ty = 0;
    double d = 0;
    r = (r>x)?x:r;
    r = (r>y)?y:r;
    tx = 0;
    ty = r;
    d = 1.25 +r;
    OLED_CirclePoint(x,y,tx,ty);
    
    while(tx<ty)
    {
        if(d<0)
        {
            d += 2*tx+3;
        }
        else
        {
            d += 2*(tx-ty)+5;
            ty--;
        }
        tx++;
        OLED_CirclePoint(x,y,tx,ty);
    }
}

void oled_printf(uint8_t x,uint8_t y,const char* format,...)
{
    char buf[50];
    va_list args;
    va_start(args,format);
    vsprintf(buf,format,args);
    va_end(args);
    OLED_ShowString(x,y,(uint8_t*)buf);
}

void OLED_ClearScreen(void)
{
    uint8_t x = 0,y = 0;
    for(y = 0;y < 8;y++)
    {
        for(x = 0;x < 128;x++)
        {
            OLED_GRAM[x][y] = 0x00;
        }
    }
    //OLED_Updata();
}

/*OLED��Ļ���º���*/
void OLED_Updata(void)
{
    uint8_t x = 0,y = 0;
    for(y = 0;y < 8;y++)
    {
        OLED_Set_Pos(0,y);
        for(x = 0;x <128;x++)
        {
            OLED_WR_Byte(OLED_GRAM[x][y],OLED_DATA);
        }
    }
}

void OLED_UpdataPart(uint8_t row1,uint8_t col1,uint8_t row2,uint8_t col2,uint8_t direct)
{
    uint8_t x = 0,y = 0;
    if(direct == 1)
    {
        OLED_WR_Byte(0x20,0);
        OLED_WR_Byte(0x00,0);
        OLED_WR_Byte(0x21,0);
        OLED_WR_Byte(col1,0);
        OLED_WR_Byte(col2,0);
        OLED_WR_Byte(0x22,0);
        OLED_WR_Byte(row1,0);
        OLED_WR_Byte(row2,0);
        for(y = row1;y<=row2;y++)
        {
            for(x = col1;x<=col2;x++)
            {
                OLED_WR_Byte(OLED_GRAM[x][y],1);
            }
        }
    }
    else
    {
        OLED_WR_Byte(0x20,0);
        OLED_WR_Byte(0x01,0);
        OLED_WR_Byte(0x21,0);
        OLED_WR_Byte(col1,0);
        OLED_WR_Byte(col2,0);
        OLED_WR_Byte(0x22,0);
        OLED_WR_Byte(row1,0);
        OLED_WR_Byte(row2,0);
        for(y = col1;y<=col2;y++)
        {
            for(x = row1;x<=row2;x++)
            {
                OLED_WR_Byte(OLED_GRAM[x][y],1);
            }
        }
    }
    OLED_WR_Byte(0x20,0);
    OLED_WR_Byte(0x02,0);
}

























