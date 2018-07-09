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

void ad_con(void);
void int_imia0(void);
void int_adi(void);

int main(void){
  
  mode = STRAIGHT;
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

      ad_status();
      
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
  
  ENINT()
}

int ad_status(void){
  int counter;
  int an1_ave,an2_ave;

  an1_ave = an2_ave = 0;
  
  for(counter=0; counter<ADNUM; counter++){
    an1_ave += adbuf[0][(adbufcounter+counter)%10];
    an2_ave += adbuf[1][(adbufcounter+counter)%10];
  }
  
  an1_ave = an1_ave / ADNUM;
  an2_ave = an2_ave / ADNUM;
  
  an1_ave%=1000;
  an2_ave%=1000;
  
  if(an1_ave > 200 && an2_ave > 200){
    PBDR = next_move;
  }else if(an1_ave/40 > an2_ave/40){
    PBDR = next_move = 0x0D;
  }else if(an1_ave/40 < an2_ave/40){
    PBDR = next_move = 0x07;
  }else{
    PBDR = 0x05;
  }

  DISINT();
  wait1ms(5);
  ENINT();
  
  lcd_cursor(0,0);
  lcd_printch('0' + an1_ave/100);
  lcd_cursor(1,0);
  lcd_printch('0' + (an1_ave - (an1_ave/100)*100)/10);
  lcd_cursor(2,0);
  lcd_printch('0' + an1_ave%10);
  
  lcd_cursor(0,1);
  lcd_printch('0' + an2_ave/100);
  lcd_cursor(1,1);
  lcd_printch('0' + (an2_ave - (an2_ave/100)*100)/10);
  lcd_cursor(2,1);
  lcd_printch('0' + an2_ave%10);
  
  return 0;
}
