#include "h8-3069-int.h"
#include "h8-3069-iodef.h"
#include "loader.h"
#include "timer.h"
#include "sci2.h"
#include "ad.h"
#include "lcd.h"

#define THROUGH  0
#define RIGHTTURN 1
#define LEFTTURN  2
#define ADNUM 5

#define CONTROLTIME 8

int move_switch_flag;
int control_timer;
int mode;

int an1_ave,an2_ave;
int adbuf[2][10];
int adbufcounter = 0;

void ad_read(void);
void int_imia0(void);
void int_adi(void);
void control_motor(void);
void disp_lcd(void);

int main(void){
  
  mode = THROUGH;
  move_switch_flag = 0;
  control_timer = 0;
  PBDDR = 0xFF;
  P6DDR = 0xFE;

  ROMEMU();
  timer_init();
  ad_init();
  lcd_init();  
  timer_set(0,1000);
  timer_intflag_reset(0);
  timer_start(0);
  ENINT();  

  while(1){
    control_motor();
    disp_lcd();
  }
}

#pragma interrupt
void int_imia0(void){

  if(control_timer >= CONTROLTIME){
    ad_scan(0,1);
  }else{
    control_timer++;
  }
  timer_intflag_reset(0);
  ENINT();
}

#pragma interrupt
void int_adi(void){
  ad_stop();
  
  adbuf[0][adbufcounter] = ADDRBH;
  adbuf[1][adbufcounter] = ADDRCH;
  
  adbufcounter++;
  adbufcounter %= 10;
  
  ENINT();
}

void ad_read(void){
  int counter;

  an1_ave = an2_ave = 0;
  
  for(counter=0; counter<ADNUM; counter++){
    an1_ave += adbuf[0][(adbufcounter+counter)%10];
    an2_ave += adbuf[1][(adbufcounter+counter)%10];
  }
  
  an1_ave = an1_ave / ADNUM;
  an2_ave = an2_ave / ADNUM;
  
  an1_ave %= 1000;
  an2_ave %= 1000;
}

void control_motor(void)
{
  ad_read();
  
  if(an1_ave > 220 && an2_ave > 220){
    PBDR = 0x00;
  }else if(an1_ave/60 > an2_ave/60){ //60はPWMの誤差削除
    PBDR = 0x07;
  }else if(an1_ave/60 < an2_ave/60){
    PBDR = 0x0D;
  }else{
    PBDR = 0x05;
  }

  ENINT();
}

void disp_lcd(void)
{  
  lcd_cursor(0,0);
  lcd_printch(an1_ave/100+'0');
  lcd_cursor(1,0);
  lcd_printch((an1_ave-(an1_ave/100)*100)/10+'0');
  lcd_cursor(2,0);
  lcd_printch(an1_ave%10+'0');
  
  lcd_cursor(0,1);
  lcd_printch(an2_ave/100+'0');
  lcd_cursor(1,1);
  lcd_printch((an2_ave-(an2_ave/100)*100)/10+'0');
  lcd_cursor(2,1);
  lcd_printch(an2_ave%10+'0');
}
