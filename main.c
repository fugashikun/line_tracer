#include "h8-3069-int.h"
#include "h8-3069-iodef.h"
#include "loader.h"
#include "timer.h"
#include "sci2.h"
#include "ad.h"
#include "lcd.h"

#define TURNLEFT  0
#define TURNRIGHT 1
#define STRAIGHT  2

#define CONTROLTIME 8

int move_switch_flag;
int control_timer;
int mode;

void int_imia0(void);

int main(void){

  int sw0;

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

  DISINT();  

  while(1){

    sw0 = P6DR & 0x01;

    if(sw0 == 0x00){
      ENINT();
      ad_status(sw0);
    }

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
