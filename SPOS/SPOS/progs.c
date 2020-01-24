// Test prog for snake
#include "led_snake.h"
#include "joystick.h"
#include "os_core.h"
#include "defines.h"
#include "util.h"
#include "lcd.h"
#include "os_process.h"
#include "os_scheduler.h"
#include "led_paneldriver.h"
#include <util/delay.h>
#include <stdlib.h>
PROGRAM(1, AUTOSTART){
	os_setSchedulingStrategy(OS_SS_RUN_TO_COMPLETION);
	js_init();
	panel_init();
	panel_initTimer();
	panel_startTimer();
	ledSnake_startNewGame();
}