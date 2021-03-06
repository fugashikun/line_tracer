#include "h8-3069-int.h"
#include "h8-3069-iodef.h"
#include "loader.h"
#include "timer.h"
#include "sci2.h"
#include "ad.h"
#include "lcd.h"

/*タイマー割り込み間隔*/
#define TIMER0 1000
/* 割り込み処理で各処理を行う頻度を決める定数 */
#define DISPTIME 100
#define KEYTIME 1
#define ADTIME  2
#define CONTROLTIME 10
/* A/D変換関連 */
/* A/D変換のチャネル数とバッファサイズ */
#define ADCHNUM   4
#define ADBUFSIZE 8
/* 平均化するときのデータ個数 */
#define ADAVRNUM 4
/* チャネル指定エラー時に返す値 */
#define ADCHNONE -1

#define RIGHTPWM 190
#define LEFTPWM 190

#define AN1STOP 0x00
#define AN1NR 0x01
#define AN1RR 0x02
#define AN1BLAKE 0x03

#define AN2STOP 0x00
#define AN2NR 0x04
#define AN2RR 0x08
#define AN2BLAKE 0x0c

/* 割り込み処理に必要な変数は大域変数にとる */
volatile int disp_time, key_time, ad_time, pwm_time, control_time;

/* A/D変換関係 */
volatile unsigned char adbuf[ADCHNUM][ADBUFSIZE];
volatile int adbufdp;

unsigned char an1,an2;

int ad_read(int ch);
void int_imia0();
void int_adi();
void control_motor(void);
void disp_lcd(void);

int main(void){

  ROMEMU();
  PBDDR = 0x0F;
  P6DDR = 0xFE;
  timer_init();
  ad_init();
  lcd_init();  
  timer_set(0,TIMER0);
  timer_start(0);
  ENINT();  
  
  while(1){
    if(P6DR & 0x01 == 0) break;
  }
  while(1){
    disp_lcd();
  }
}

#pragma interrupt
void int_imia0(void)
{
  ad_time++;
  if(ad_time == ADTIME){
    ad_scan(0,1);
    ad_time = 0;
  }
  control_time++;
  if(control_time == CONTROLTIME){
    control_motor();
    control_time = 0;
  }
  timer_intflag_reset(0);
  ENINT();
}

#pragma interrupt
void int_adi(void){
  ad_stop();
  if(adbufdp < ADBUFSIZE-1) adbufdp++;
  else adbufdp = 0;
  adbuf[0][adbufdp] = ADDRBH;
  adbuf[1][adbufdp] = ADDRCH;
  ENINT();
}

void control_motor(void)
{
  an1 = ad_read(0);
  an2 = ad_read(1);

  PBDR = AN1STOP | AN2STOP;
  PBDR = AN1NR | AN2NR;
  
  if(an1 >= LEFTPWM){
    PBDR &= (AN2STOP | AN1NR);
  }
  else{
    PBDR |= AN1NR;
  }
  if(an2 >= RIGHTPWM){
    PBDR &= (AN1STOP | AN2NR);
  }
  else{
    PBDR |= AN2NR;
  }
}

int ad_read(int ch)
{
  
  int i,ad,bp;
  int counter;
  /*
  if ((ch > ADCHNUM) || (ch < 0)) ad = ADCHNONE;
  else {
    bp = adbufdp;
    ad = 0;

    for(i = 0;i < ADAVRNUM;i++){
      if(bp <= 0){
        bp = ADBUFSIZE-1;
      }else{
        bp--;
      }
      ad += adbuf[ch][bp];
    }
    ad /= ADAVRNUM;
  }
  return ad; /* データの平均値を返す */
  
  for(counter=0;counter<ADAVRNUM;counter++){
    ad = adbuf[ch][(adbufdp+counter)%10];
    ad = adbuf[ch][(adbufdp+counter)%10];
  }
  ad = ad/ADAVRNUM;
  ad %= 1000; 
  return ad;
}

void disp_lcd(void)
{
  lcd_cursor(0,0);
  lcd_printch(an1/100+'0');
  lcd_cursor(1,0);
  lcd_printch((an1-(an1/100)*100)/10+'0');
  lcd_cursor(2,0);
  lcd_printch(an1%10+'0');
  
  lcd_cursor(0,1);
  lcd_printch(an2/100+'0');
  lcd_cursor(1,1);
  lcd_printch((an2-(an2/100)*100)/10+'0');
  lcd_cursor(2,1);
  lcd_printch(an2%10+'0');
}
/*
  if(turn_flag==0){
    if(leftval > LEFTPWM && rightval < RIGHTPWM){
      mode = 0x05;
    }else if(leftval > LEFTPWM && rightval > RIGHTPWM){
      if(mode == 0x0D){
	mode = 0x09;
      }else{
	mode = 0x0D;
      }
    }else if(leftval < LEFTPWM && rightval < RIGHTPWM){
      mode = 0x07;
    }else if(leftval > LEFTPWM && rightval < RIGHTPWM){
      turn_flag=1;
    }
  }
  if(turn_flag==1){
    if(leftval > LEFTPWM && rightval < RIGHTPWM){
      mode = 0x05;
    }else if(leftval > LEFTPWM && rightval > RIGHTPWM){
      if(mode == 0x07){
	mode = 0x06;
      }else{
	mode = 0x07;
      }
    }else if(leftval < LEFTPWM && rightval < RIGHTPWM){
      mode = 0x0D;
    }else if(leftval > LEFTPWM && rightval < RIGHTPWM){
      turn_flag=0;
    }
  }
  */
  /*
  if(leftval > LEFTPWM && rightval < RIGHTPWM){
    new_mode = 0x05;
  }else if(leftval < LEFTPWM && rightval < RIGHTPWM){
    new_mode = 0x07;
  }else if(leftval > LEFTPWM && rightval > RIGHTPWM){
    new_mode = 0x0D;
  }else if(leftval < LEFTPWM && rightval > RIGHTPWM){
    new_mode = 0x07;
  }
  if(old_mode==0x05){
    if(new_mode==0x05) mode = 0x05;
    else if(new_mode==0x0D) mode = 0x0D;
    else if(new_mode==0x07) mode = 0x07;
  }else if(old_mode==0x0D){
    if(new_mode==0x05) mode = 0x05;
    else if(new_mode==0x0D) mode = 0x0D;
    else if(new_mode==0x07) mode = 0x07;
  }else if(old_mode==0x07){
    if(new_mode==0x05) mode = 0x05;
    else if(new_mode==0x0D) mode = 0x0D;
    else if(new_mode==0x07){
      mode = 0x07;
      if(old_old_mode == 0x07) mode = 0x02;
    }
  }
  PBDR = mode;
  old_old_mode = old_mode;
  old_mode = new_mode;
  */
  /*
  if(turn_flag%2==0){
    if(leftval > LEFTPWM && rightval < RIGHTPWM){
      new_mode = 0x05;
    }else if(leftval > LEFTPWM && rightval > RIGHTPWM){
      if(old_mode==0x09 || old_mode==0x0D){
	count++;
      }else count = 0;
      if(mode == 0x0D){
	new_mode = 0x09;
      }else{
	new_mode = 0x0D;
      }
    }else if(leftval < LEFTPWM && rightval < RIGHTPWM){
      if(count > 20){
	turn_flag++;
	count=0;
	new_mode = 0x00;
      }else new_mode = 0x07;
    }
  }
  if(turn_flag%2==1){
    if(leftval < LEFTPWM && rightval > RIGHTPWM){
      new_mode = 0x05;
    }else if(leftval > LEFTPWM && rightval > RIGHTPWM){
      if(old_mode==0x06 || old_mode==0x07){
	count++;
      }else count = 0;
      if(mode == 0x07){
	new_mode = 0x06;
      }else{
	new_mode = 0x07;
      }
    }else if(leftval < LEFTPWM && rightval < RIGHTPWM){
      if(count > 20){
	turn_flag++;
	count=0;
	new_mode = 0x00;
      }else new_mode = 0x0D;
    }
  }
  old_mode = new_mode;
  PBDR = new_mode;
  */
