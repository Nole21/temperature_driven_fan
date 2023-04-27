#ifndef __KS0108_H__
#define __KS0108_H__
#define  E   0
#define	RW  1 
#define  RS  2
#define  CS1 3
#define	CS2 4
#define	RES 5
#define CTRL P4OUT

#define uchar unsigned char 
#define uint  unsigned int


#define uchar unsigned char
#define uint unsigned int 



unsigned char CurOffset,CurRow,CurPage,CurCol; 


#define LCD_STATUS_BUSY 0x80 
#define  START_LINE0   0xc0 
#define  DISPLAY_ON    0x3f 
#define  DISPLAY_OFF   0x3e 
#define  PARA1         0x40 


#define LCD_IP_PORT P1OUT   
#define LCD_OP_PORT P1DIR   


#define Bit(x,y,z)    x=(z)?x|(1<<y):x&(~(1<<y))  
 
#define SET_LCD_E           Bit(CTRL,E,1)    
#define CLEAR_LCD_E         Bit(CTRL,E,0)  


#define SET_LCD_DATA        Bit(CTRL,RS,1)    
#define SET_LCD_CMD         Bit(CTRL,RS,0)   

#define SET_LCD_READ        Bit(CTRL,RW,1)    
#define SET_LCD_WRITE       Bit(CTRL,RW,0)   

#define SET_LCD_CS2         Bit(CTRL,CS2,1)  
#define CLEAR_LCD_CS2       Bit(CTRL,CS2,0) 

#define SET_LCD_CS1         Bit(CTRL,CS1,1)  
#define CLEAR_LCD_CS1       Bit(CTRL,CS1,0) 

#define LEFT 0 
#define RIGHT 1 
#define CMD 0 
#define DATA 1 
extern void delayms(uint);
uchar PINtem;
void LCD_BUSY(unsigned char lr) 
{ 
	if(lr==LEFT) 
	{ 

	    CLEAR_LCD_CS2; //cs2=0 
	    SET_LCD_CS1;   //cs1=1 
	} 
	else 
	{ 

	SET_LCD_CS2;   //cs2=1 
	     CLEAR_LCD_CS1; //cs1=0 
	} 
	SET_LCD_CMD;
	   
	LCD_OP_PORT = 0x00;
	SET_LCD_READ;
	SET_LCD_E;
	 
	while((P6IN)&(0x80))
	{ 
		CLEAR_LCD_E;   
		SET_LCD_E;
	} 
	CLEAR_LCD_E; 
	SET_LCD_WRITE;
	// LCD_IP_PORT=0xFF;
   // LCD_OP_PORT = 0x00;
   
} 
unsigned char FindWord(unsigned int add)
{
	 uchar ChineseH,ChineseL,i,f;
	 ChineseH=add/256;
	 ChineseL=add%256;
	 for(i=0;i<255;i++)
	 {
	 	if(CHfont[i].Index[1]==ChineseL&&CHfont[i].Index[0]==ChineseH)
		{
			f=i;
			break;
		}
	 }
	 if(i==255)
	 return	 0xFF;
	 else
	 return f;

}
void write_LCD(unsigned char lr,unsigned char cd,unsigned char Data) 
{ 
	//LCD_BUSY(lr);
  
	if(cd==CMD) 
		SET_LCD_CMD; 
	else 
		SET_LCD_DATA;
        LCD_OP_PORT = 0xff; 
	SET_LCD_WRITE;
	LCD_IP_PORT = Data;  
	SET_LCD_E; 
	CLEAR_LCD_E; 
	LCD_OP_PORT = 0x00; 
} 
unsigned char read_LCD(unsigned char lr)  
{ 
	unsigned char Data; 
	LCD_BUSY(lr);  
	SET_LCD_DATA;
	LCD_OP_PORT = 0x00;
     //PORTD=0xFF; 
	SET_LCD_READ; 
    // delayms(1); 
	SET_LCD_E;

        Data=P1IN; 
	CLEAR_LCD_E;
	 


	LCD_BUSY(lr);
	SET_LCD_DATA; 
	LCD_OP_PORT = 0x00;
    //PORTD=0xFF;
	SET_LCD_READ; 
	SET_LCD_E;
    // delayms(1);  
	Data=P1IN;
	CLEAR_LCD_E; 

	return Data; 

} 

void set_start_line_L(unsigned char line) 
{ 
	write_LCD(LEFT,CMD,0xc0|line);  
} 

void set_start_line_R(unsigned char line) 
{ 
	write_LCD(RIGHT,CMD,0xc0|line);  
} 

void set_page_L(unsigned char page)
{ 
	write_LCD(LEFT,CMD,0xb8|page);                      
} 
void set_page_R(unsigned char page)
{ 
	write_LCD(RIGHT,CMD,0xb8|page);                      
} 

void set_col_addr_L(unsigned char col) 
{ 
	write_LCD(LEFT,CMD,0x40|col);                      
} 

void set_col_addr_R(unsigned char col) 
{ 
	write_LCD(RIGHT,CMD,0x40|col);                      
} 

void init_lcd(void) 
{ 
	Bit(CTRL,RES,0);
	delayms(10);
	Bit(CTRL,RES,1);
	set_start_line_L(0); 
	set_start_line_R(0);
	write_LCD(LEFT,CMD,DISPLAY_ON); 
	write_LCD(RIGHT,CMD,DISPLAY_ON); 
} 

void Clr_LCD(void) 
{ 
	unsigned char pages,i; 
	for(pages=0;pages<8;pages++) 
	{ 
		set_page_L(pages); 
		set_page_R(pages);  
		for(i=0;i<64;i++)    
		{ 
			set_col_addr_L(i);//Y 
			set_col_addr_R(i);//Y 
			write_LCD(LEFT,DATA,0x0); 
			write_LCD(RIGHT,DATA,0x0); 
		} 
	} 
} 


void Clr_row(){
        unsigned char pages,i; 
	for(pages=2;pages<8;pages++) 
	{ 
		set_page_L(pages); 
		set_page_R(pages);  
		for(i=0;i<64;i++)    
		{ 
			set_col_addr_L(i);//Y 
			set_col_addr_R(i);//Y 
			write_LCD(LEFT,DATA,0x0); 
			write_LCD(RIGHT,DATA,0x0); 
		} 
	} 
}


void pixel(unsigned char xx,unsigned char yy,unsigned char flag) 
{ 
	unsigned int y,ch; 
	ch=yy%8;  
	
	y=1; 
	for(;ch!=0;) 
	{ 
		y=y*2; 
		ch--; 
	} 
	if(xx<64) 
	{ 
		set_page_L(yy/8); 
		set_col_addr_L(xx); 
		ch=read_LCD(LEFT); 
		set_col_addr_L(xx); 
		if(flag) 
			write_LCD(LEFT,DATA,ch|y); 
		else 
		{ 
			y=~y; 
			ch&=y; 
			write_LCD(LEFT,DATA,ch); 
		} 
	} 
	else 
	{ 
		set_page_R(yy/8); 
		set_col_addr_R(xx-64); 
		ch=read_LCD(RIGHT); 
		set_col_addr_R(xx-64); 
		if(flag) 
			write_LCD(RIGHT,DATA,ch|y); 
		else 
		{ 
			y=~y; 
			ch&=y; 
			write_LCD(RIGHT,DATA,ch); 
		} 
	} 
} 



unsigned char GetPage(void) 
{ 
	return CurPage; 
} 

unsigned char GetCol(void)
{ 
	return CurCol; 
} 

void SetPageCol(unsigned char upage, unsigned char ucol) 
{ 

	if(ucol<64) 
	{ 
		set_page_L(upage); 
		set_col_addr_L(ucol); 
	} 
	else 
	{ 
		set_page_R(upage); 
		set_col_addr_R(ucol-64); 
	} 
} 

/* 璁剧疆褰撳墠鏄剧ず鐨勯〉鍜屽垪 */ 
void SetRowCol(unsigned char ucol, unsigned char urow)  
{ 

	SetPageCol(urow,ucol); 
} 
void Writedisdata8uchar(unsigned char x,unsigned char y,unsigned Data,uchar color)
{
	// unsigned char temp;
	 uchar LR;
	 LR=LEFT;
	
	 SetRowCol(x,y);
	
	 if(x>=64)
	 {
	 
	  LR=RIGHT;
	 }
	if(color==0)
	  Data=~Data;
	  
	 write_LCD(LR,DATA,Data); 
	 
	 //temp=read_LCD(LEFT);     

}
void Writedisdata4uchar(unsigned char x,unsigned char y,unsigned char Data,uchar color)
{
	 unsigned char temp,temp1,temp2;
	 uchar LR;
	 LR=LEFT;
	 temp=y/2;
	 if(color==0)
	  Data=~Data;
	  if(x>=64)
	 {
	  
	  LR=RIGHT;
	 }
	 
	 SetRowCol(x,temp);
	 temp1=read_LCD(LR); 
	 
	 if(y%2==0)
		temp2=(temp1&0xF0)|(Data&0x0F);	
	 else
		temp2=(temp1&0x0F)|(Data<<4);
	 SetRowCol(x,temp);
	 write_LCD(LR,DATA,temp2); 
	 //temp=read_LCD(LEFT);
	      

}
void DrawBmp(unsigned char x1,unsigned char y1,unsigned char x2,unsigned char y2, char const  *p,uchar color)
{
	unsigned char L,H,i,j;
	L=x2-x1;
	H=y2-y1;
	for(i=0;i<=H;i++)
	for(j=0;j<=L;j++)
	Writedisdata8uchar(x1+j,y1+i,*(p++),color);
} 

void displayonechar(unsigned char x,unsigned char y, uchar Data,uchar color)
{
	  unsigned char i;
      char const  *p;
	  x=x<<2;
		 if(y==4)
	 y=13;
	 else
	 y=y*3;
	 p=(ENfont+(unsigned int)(Data-32)*16);
	  for(i=0;i<8;i++)
	  {
		 Writedisdata4uchar(x+i,y,*(p+i),color);
		 Writedisdata4uchar(x+i,y+1,(*(p+i))>>4,color);
		 Writedisdata4uchar(x+i,y+2,*(p+8+i),color);

	  }
}

void displayoneword(uchar x,uchar y,  const  char *p,uchar color)
{
	 unsigned char i;
	 x=x<<2;
	 if(y==4)
	 y=13;
	 else
	 y=y*3;
     for(i=0;i<12;i++)
	  {
		 Writedisdata4uchar(x+i,y,*(p+i),color);
		 Writedisdata4uchar(x+i,y+1,(*(p+i))>>4,color);
		 Writedisdata4uchar(x+i,y+2,*(p+12+i),color);

	  }

}
void DisplayStrings(uchar x,uchar y,   const   char *t,uchar color)
{
	uint len,i;
    const  char  *z;
    const  uchar  *p;
	len=strlen(t);
    p=(const  uchar *)t;
    while(len)
	{
		
		if(*p>=161)
		{
			  if(x*4>116)
		      break;
               i=FindWord((*p)*256+*(p+1));
			  z=(const  char *)CHfont[i].Msk;
			  displayoneword(x, y, z,color);
		      x+=3;
			 
			  p++;
			  p++;
		      
			  len-=2;	
		}else
		{
		      if(x*4>120)
		      break;
			  displayonechar(x,y, *p,color);
			  x+=2;
			  p++;
			  len-=1;
		}
	

	}

}

void GUI_Line(unsigned int x0,unsigned int y0,unsigned int x1,unsigned int y1,unsigned int color)
{  
	unsigned int t;  
	int xerr=0,yerr=0,delta_x,delta_y,distance;  
	int incx,incy;  
	unsigned int row,col;
	if( x0>128||
		x1>128||
		y0>64||
		y1>64	)
	return;

	delta_x = x1-x0;					
	delta_y = y1-y0;  
	col = x0;  
	row = y0;

	if(delta_x>0)						 
		incx=1;
	else   
		{  
		if( delta_x==0)
			incx=0;						
		else
			{
			incx=-1;
			delta_x=-delta_x;
			}  
		}

	if(delta_y>0)
		incy=1;  
	else  
		{  
		if( delta_y==0)
			incy=0;						 
		else
			{
			incy=-1;
			delta_y=-delta_y;
			}
		}

	if(delta_x> delta_y )
		distance=delta_x;				
	else
		distance=delta_y;  

	for( t=0;t <= distance+1; t++ )  
		{								 
		pixel(col, row, color);
 
		xerr +=delta_x;
		yerr +=delta_y;

		if(xerr > distance)  
			{  
			xerr -= distance;  
			col += incx;  
			}  
		if(yerr > distance)  
			{  
			yerr -= distance;  
			row += incy;  
			}  
		}  
}

void plotC(int x,int y,int xc,int yc,unsigned char colour) 
{ 
    pixel(xc+x,yc+y,colour); 
    pixel(xc+x,yc-y,colour); 
    pixel(xc-x,yc+y,colour); 
    pixel(xc-x,yc-y,colour); 
    pixel(xc+y,yc+x,colour); 
    pixel(xc+y,yc-x,colour); 
    pixel(xc-y,yc+x,colour); 
    pixel(xc-y,yc-x,colour); 
} 


void GUI_Circle(int xc,int yc,int r,unsigned char colour) 
{ 
    int x,y,d; 
    y = r; 
    d = 3 - (r + r); 
    x = 0; 
    while(x <= y) 
    { 
        plotC(x,y,xc,yc,colour); 
        if(d < 0) 
            d += (x + x + x + x) + 6; 
        else 
        { 
            d+=((x - y) + (x - y) + (x - y) + (x - y)) + 10; 
            y = y - 1; 
        } 
        x = x + 1; 
    } 
} 



void GUI_Full(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char colour) 
{ 
    unsigned char i,j; 
for(j = y0;j <= y1;j ++) 
    for(i = x0;i <= x1;i ++) 
    pixel(i,j,colour); 
} 

void donghua(void)
{
    unsigned char i,j;
	for(i=36;i<16+36;i++)
	{						 
		pixel(11,i,1);
		pixel(113,i,1);
    }
	for(i=0;i<101;i++)
	{
		pixel(i+12,36,1);
		pixel(i+12,36+15,1);
    }
	for(i=0;i<100;i++)
	{
		if(i%5==0)
		{
			
         displayonechar(12,2,i/10|0x30,1);
		displayonechar(14,2,i%10|0x30,1);
		displayonechar(16,2,'%',1);
			continue;
		}			 
		for(j=0;j<12;j++)
		pixel(i+12,j+38,1);
	     
	
	}
	displayonechar(10,2,1|0x30,1);
	displayonechar(12,2,0|0x30,1);
	displayonechar(14,2,0|0x30,1);
	displayonechar(16,2,'%',1);
	delayms(100);
		
} 
	
//*/
#endif 
 