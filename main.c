#include <MSP430x24x.h>
#include <string.h>
#include <float.h>
#include <stdio.h>
#include <math.h>
#include "font.h"
#include "lcd12864.h"
#define uchar unsigned char
#define uint unsigned int

/**************LCD************************/
#define LCDIO     P3OUT
#define LCD1602_RS_1  P2OUT|=1  
#define LCD1602_RS_0  P2OUT&=~1 
#define LCD1602_RW_1  P2OUT|=2
#define LCD1602_RW_0  P2OUT&=~2 
#define LCD1602_EN_1   P2OUT|=4
#define LCD1602_EN_0   P2OUT&=~4


/**************LED************************/
#define LOW_THRESHOLD 60
#define HIGH_THRESHOLD 80

//  DS18B20 Defines to configure pins
#define DS18B20_PIN BIT7


// Commands to handle the sensor
#define DS18B20_SKIP_ROM             0xCC
#define DS18B20_READ_SCRATCHPAD      0xBE
#define DS18B20_CONVERT_T            0x44
#define DS1820_READ_ROM              0x33

#define CPU_CLOCK       8000000
#define delay_us(us)    __delay_cycles(CPU_CLOCK/1000000*(us))
#define delay_ms(ms)    __delay_cycles(CPU_CLOCK/1000*(ms))






typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
 
volatile float temperature = 0.0;
int cnt = 0;
int flag = 0;
int cycle = 0;
int fan = 0;
char record[10][10];
void delayms(uint t)
{
    uint i;
    while(t--)
      for(i=1330;i>0;i--);//进过参数的调整 
}

void ds18b20_init(void){
  // change to transmit mode
  P2DIR |= DS18B20_PIN;
  P2OUT &= ~DS18B20_PIN;
  delay_us(500);
  // release the bus
  P2DIR &= ~DS18B20_PIN;
  delay_us(240);
}


void ds18b20_write_byte(uint8_t byte) {
    uint8_t bit;
    // 逐位写入字节
    for (bit = 0; bit < 8; bit++) {
        // pull low
        P2DIR |= DS18B20_PIN;
        P2OUT &= ~DS18B20_PIN;
        delay_us(1);
        // 如果该位为1，输出高电平
        if (byte & (1 << bit)) {
          // release the pin
           P2OUT |= DS18B20_PIN; 
        }
        // minimum 60us
        delay_us(60);
        // release 
        P2DIR &= ~ DS18B20_PIN;
        delay_us(2);
    }
}

uint8_t ds18b20_read_byte(void) {
    uint8_t byte = 0;
    uint8_t bit;
    // 从低位开始
    for (bit = 0; bit < 8; bit++) {
        //  pull low master bus
        P2DIR |= DS18B20_PIN;
        P2OUT &= ~ DS18B20_PIN;
        delay_us(2);
        // release the bus
        P2DIR &= ~ DS18B20_PIN;
        // 读取该位数据
        delay_us(10);
        if (P2IN & DS18B20_PIN) {
            byte |= (1 << bit);
        }
        delay_us(60);
        delay_us(5);
    }
    return byte;
}

uint16_t ds18b20_read_temperature(void) {
    char rom[9];
    uint16_t temperature;
    // 复位DS18B20
    ds18b20_init();
    // 发送跳过ROM命令
    ds18b20_write_byte(DS18B20_SKIP_ROM);
    // 发送启动温度转换命令
    ds18b20_write_byte(DS18B20_CONVERT_T);
    // 等待转换完成
    delayms(5);
    // 复位
    ds18b20_init();
    // skip rom
    ds18b20_write_byte(DS18B20_SKIP_ROM);
    // 发送读取温度命令
    ds18b20_write_byte(DS18B20_READ_SCRATCHPAD);
    // 读取scratchpad 9个字节的数据
    for(int i=0;i<9;i++){
      rom[i] = ds18b20_read_byte();
    }
    // 将两个字节的温度值合并成一个16位的整数
    temperature = ((uint16_t)rom[1] << 8) | rom[0];
    return temperature;
}

void convert_temperature(uint16_t temp, char t[]){
    // 负数
    if((temp>>11)==0x001F){
       temp = ~temp + 0x0001;
    }
    int num[8];
    for(int i=0;i<8;i++) num[i]=0;
    if(temp & (1)){         // 0.0625
       num[6] += 5;
       num[5] += 2;
       num[4] += 6;
    }
    if(temp &(1<<1)){       // 0.125
      num[3] += 1;
      num[4] += 2;
      num[5] += 5;
    }
    if(temp &(1<<2)){         // 0.25
      num[3] += 2;
      num[4] += 5;
    }
    if(temp &(1<<3)){            // 0.5
      num[3] += 5;
    }
    if(temp &(1<<4)) num[2]+=1; // 1
    if(temp &(1<<5)) num[2]+=2; // 2
    if(temp &(1<<6)) num[2] +=4; // 4
    if(temp &(1<<7)) num[2] +=8; // 8
    if(temp &(1<<8)){   // 16
       num[1] += 1;
       num[2] += 6;
    }
    if(temp &(1<<9)){    //32
      num[1] += 3;
      num[2] += 2;
    }
    if(temp &(1<<10)){  //64
      num[1] += 6;
      num[2] += 4;
    }
    for(int i=6;i>0;i--){
      num[i-1] += num[i]/10;
      num[i] = num[i]%10;
    }
    for(int i=0;i<3;i++) t[i]=num[i]+'0';
    t[3]='.';
    for(int i=4;i<8;i++) t[i]=num[i-1]+'0';
}


float char2float(uint16_t temp,char num[]){
  float t=0.0;
  for(int i=0;i<3;i++) t = t*10 + (num[i]-'0');
  for(int i=1;i<5;i++) t += pow(10,-i)*(num[i+3]-'0');
  if(temp & (1<<1)) return -t;
  else return t;
}

void build_info(uint16_t temp,char num[],char info[],char fan[]){
  info[0] = 'T';
  info[1] = 'e';
  info[2] = 'm';
  info[3] = 'p';
  info[4] = '(';
  info[5] = 'C';
  info[6] = ')';
  info[7] = ':';
  int index=8;
  if(temp & (1<<11)) info[index++]='-';
  for(int i=0;i<7;i++){
      if(i==0&&num[i]=='0') continue;
      if(i==1&&num[1]=='0'&&num[0]=='0') continue;
      info[index++] = num[i];
  }
  fan[0]='F';
  fan[1]='A';
  fan[2]='N';
  fan[3]=':';
  if(temperature<(float)LOW_THRESHOLD){
    fan[4]='i';
    fan[5]='d';
    fan[6]='l';
    fan[7]='e';
    fan[8]=fan[9]=fan[10]=' ';
  }
  else if(temperature>=(float)HIGH_THRESHOLD){
    fan[4]='3';
    fan[5]='0';
    fan[6]='0';
    fan[7]='0';
    fan[8]='r';
    fan[9]='p';
    fan[10]='m';
  }
  else{
    float percentage = (float) (temperature-LOW_THRESHOLD)/(HIGH_THRESHOLD-LOW_THRESHOLD);
    int speed = (int)3000*percentage;
    if(speed>=1000){
      fan[7]= speed%10 + '0';
      speed /= 10;
      fan[6] = speed %10 +'0';
      speed /= 10;
      fan[5] = speed %10 + '0';
      speed /= 10;
      fan[4] = speed %10 + '0';
      fan[8]='r';
      fan[9]='p';
      fan[10]='m';
    }
    else if(speed>=100){
      fan[6] = speed % 10 + '0';
      speed /= 10;
      fan[5] = speed %10 + '0';
      speed /= 10;
      fan[4] = speed % 10 + '0';
      fan[7]='r';
      fan[8]='p';
      fan[9]='m';
      fan[10]=' ';
    }
    else{
      fan[5] = speed % 10 + '0';
      speed /= 10;
      fan[4] = speed %10+ '0';
      fan[6]='r';
      fan[7]='p';
      fan[8]='m';
      fan[9]=fan[10]=' ';
    }
  }
}


/*******检查忙函数*************/
void LCD_check_busy()      //实践证明，在我的LCD1602上，检查忙指令通过率极低，以
{                                          //至于不能正常使用LCD。因此我没有再用检查忙函数。而使
 
        P3DIR=0x00;
        LCDIO=0xFF;
        LCD1602_RS_0;                 //要用200次循环便能完成。    
        LCD1602_RW_1;
        LCD1602_EN_1;
        while(P6IN&0x80);
        LCD1602_EN_0;
        P3OUT=0x00;
        P3DIR=0xFF;
}
/******************************/

/**************写指令函数********************************/  
void LCD_write_command(unsigned char command)
{
      
        //LCD_check_busy(); //加上这句仿真无法通过
        
        LCD1602_RS_0;   
        LCDIO=command;
        LCD1602_EN_1;
        
        //delayms(1);
        LCD1602_EN_0;
        delayms(1);
  
}
/***************************************************/
/****************写数据函数************************/
void LCD_write_dat( unsigned char dat)
{
      //LCD_check_busy();  //加上这句仿真无法通过
      LCD1602_RS_1;
      LCDIO=dat;
      LCD1602_EN_1;
      
      //delayms(1);
      LCD1602_EN_0;
      delayms(1);
      LCD1602_RS_0;


}
/****************************************************/
/***************设置显示位置**************************/
void LCD_set_xy( unsigned char x, unsigned char y )
{
    unsigned char address;
    if (y == 1) 
      address = 0x80+x;
    else if (y == 2)
    {
       address=0x80+0x40+x;        
    }
    LCD_write_command(address); 
}
/***************************************************/
/****************显示一个字符**********************/
void LCD_dsp_char( unsigned char x,unsigned char y, char dat)
{
  LCD_set_xy( x, y ); 
  LCD_write_dat(dat);
}
/**********************************************/
/***************显示字符串函数***************/
void LCD_dsp_string(unsigned char X,unsigned char Y,const char *s)
{
       uchar len,List;
       len=strlen(s);
       LCD_set_xy( X, Y ); 
       for(List=0;List<len;List++)
       //LCD_dsp_char(X+List,Y,s[List]);
       LCD_write_dat(s[List]);
      
}
/***********************************************/
/********** 延时**********************/
void delay_nms(unsigned int n)      
{
       unsigned int i=0,j=0;
       for (i=n;i>0;i--)
       for (j=0;j<10;j++);  
}
/**************************************/
/************初始化函数****************/
void LCD_init(void)
{
      LCD1602_RW_0;
      LCD1602_EN_0;
      //CLEARSCREEN;//clear screen 
      LCD_write_command(0x38);//set 8 bit data transmission mode 
      delayms(1);
      LCD_write_command(0x38);//set 8 bit data transmission mode 
      delayms(1);
      LCD_write_command(0x38);//set 8 bit data transmission mode 
      delayms(1);
      LCD_write_command(0x06);//open display (enable lcd display)
      delayms(1);
      LCD_write_command(0x0C);//set lcd first display address 
      delayms(1);
      LCD_write_command(0x01);//clear screen
      delayms(1);
      //LCD_write_command(0x80);//clear screen
      //delayms(1);
}
/****************************************************/




void timer_init(){
    // Timer A0
    TA0CTL = TASSEL_2 + MC_1 + ID_3;  // TASSEL_2 selects SMCLK as the clock source, and MC_1 tells  it to count up to the value in TA0CCR0
    TA0CCR0 = 500; //Set the period in the Timer A0 Capture/Compare 0 register to 1000 us.
    TA0CCTL0 =  CCIE;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    TA0CCR0 = 500 - 250 * (int)(temperature - LOW_THRESHOLD)/(HIGH_THRESHOLD-LOW_THRESHOLD);
    if(fan) P5OUT ^= BIT1;          // 每次中断翻转P5.1引脚的电平
    else P5OUT &= ~BIT1;
}

void record_fan(){
  int time = 10 * cycle;
  record[cnt][0] = cnt+1 + '0';
  record[cnt][1] = ':';
  if(time>=1000){
    record[cnt][6] = 's';
    record[cnt][5]= time%10+'0';
    time /= 10;
    record[cnt][4] = time %10 +'0';
    time /= 10;
    record[cnt][3] = time %10 +'0';
    time /= 10;
    record[cnt][2] =time %10 +'0';
  }
  else if(time>=100){
    record[cnt][5] = 's';
    record[cnt][4]= time%10+'0';
    time /= 10;
    record[cnt][3] = time %10 +'0';
    time /= 10;
    record[cnt][2] = time %10 +'0';
  }
  else if(time>=10){
    record[cnt][4] = 's';
    record[cnt][3] = time % 10 +'0';
    time /= 10;
    record[cnt][2] = time + '0';
  }
  else{
    record[cnt][4] = ' ';
    record[cnt][3] = 's';
    record[cnt][2] = time + '0';
  }
  
}

void main(void)
{
    
    WDTCTL=WDTPW + WDTHOLD; // 关闭看门狗
    P2DIR=0xFF;            // 设置方向
    P2SEL=0;            // 设置为普通I/O 口
    P4DIR = 0xFF;
    P4SEL = 0;
    P4OUT = 0x00;
    P2OUT=0x00;
    P3DIR = 0xFF;
    P3OUT=0xFF;
    P3SEL = 0;
    P2DIR &= ~ DS18B20_PIN;
    //P1DIR = 0xFF;
   // P1SEL = 0;
    //P1OUT = 0xFF;
    P5DIR = 0xFF;
    P5SEL = 0;
    P5OUT = 0;
    
    // button 
    P6DIR &=~(BIT0|BIT1);
    P6REN = BIT0|BIT1;
    P6OUT = BIT0|BIT1;
    timer_init();
    _BIS_SR(GIE);  // 允许中断
    delayms(50);  
    init_lcd();
    Clr_LCD();
    LCD_init();
    // current record
    int cur = -1;
    while(1)
    {
     cycle++;
     char info[14];
     char buffer[10];
     char fan_info[12];
     __disable_interrupt();
     uint16_t temp = ds18b20_read_temperature();
     __enable_interrupt();
     __disable_interrupt();
     convert_temperature(temp,buffer);
     temperature = char2float(temp,buffer);
     build_info(temp,buffer,info,fan_info);
     __enable_interrupt();
     if(temperature >= (float)LOW_THRESHOLD){
          fan = 1;
          P5OUT |= BIT0;
          //__enable_interrupt();
          P5OUT |= BIT1;
          if(flag==0){
            flag=1;
            record_fan();
            cnt++;
            char info[15];
            info[0] = cnt + '0';
            info[1] = 'R';
            info[2] = 'E';
            info[3] = 'C';
            info[4]=info[5]=info[6]=info[7]=info[8]=info[9]=info[10]=info[11]=info[12]=info[13]=info[14]=' ';
            DisplayStrings(1,0,info,1);
          }
     }
     else{
          fan = 0;
          P5OUT &= ~ BIT0;
          flag = 0;
          //__disable_interrupt();
          P5OUT &= ~ BIT1;
     }
     // display temperature
     
     __disable_interrupt();
     LCD_dsp_string(1,1,info);
     LCD_dsp_string(1,2,fan_info);   
     __enable_interrupt();
     
     // review the record
     // previous record
     if((P6IN&BIT0)){
       if(cnt==0){
         Clr_row();
         DisplayStrings(1,2,"No record.",1);
       }
        else{
          if(cur==0||cur==-1) cur = cnt-1;
          else cur = cur-1;
          Clr_row();
          DisplayStrings(1,2,record[cur],1);
        }
     }
     // next record
     if((P6IN&BIT1)){
       if(cnt==0){
          Clr_row();
         DisplayStrings(1,2,"No record.",1);
       }
        else{
          cur = (cur+1)%cnt;
           Clr_row();
          DisplayStrings(1,2,record[cur],1);
        }
     }
     
     delayms(10);
    }
}




