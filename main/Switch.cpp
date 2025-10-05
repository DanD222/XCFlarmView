/*
 * Switch.cpp
 *
 *  Created on: Nov 11, 2023
 *      Author: iltis
 *
 *      Static class for a single push buttom
 *
 *      Example:
 *      sw = Switch();
 *      sw.begin( GPIO_NUM_1 );
 *      if( sw.state() )
 *         ESP_LOGI(FNAME,"Open");
 *
 */

#include "esp32-hal.h"
#include "driver/gpio.h"
#include "Switch.h"
#include <logdef.h>
#include "Setup.h"
#include "average.h"
#include "vector.h"
#include "Units.h"
#include "flarmview.h"



std::list<SwitchObserver *> Switch::observers;
TaskHandle_t Switch::pid;

#define TASK_PERIOD 10

Switch::Switch( ) {
	_sw = GPIO_NUM_0;
	_mode = B_MODE;
	_closed = false;
	_holddown = 0;
	_tick = 0;
	_closed_timer = 0;
	_long_timer = 0;
	pid = 0;
	_click_timer=0;
	_clicks=0;
	_state=B_IDLE;
	p_time = 0;
	r_time = 0;
}

Switch::~Switch() {
}


void Switch::attach(SwitchObserver *obs) {
	// ESP_LOGI(FNAME,"Attach obs: %p", obs );
	observers.push_back(obs);
}

void Switch::detach(SwitchObserver *obs) {
	// ESP_LOGI(FNAME,"Detach obs: %p", obs );
	/*
        auto it = std::find(observers.begin(), observers.end(), obs);
        if ( it != observers.end() ) {
                observers.erase(it);
        }
	 */
}

void Switch::switchTask(void *pvParameters){
	while(1){
		swUp.tick();
		swDown.tick();
		swMode.tick();
		delay(TASK_PERIOD);
	}
}

void Switch::startTask(){
	ESP_LOGI(FNAME,"taskStart");
	xTaskCreatePinnedToCore(&switchTask, "Switch", 6096, NULL, 9, &pid, 0);
}

void Switch::begin( gpio_num_t sw, t_button mode ){
	ESP_LOGI(FNAME,"Switch::begin GPIO: %d", sw);
	_sw = sw;
	_mode = mode;
	gpio_set_direction(_sw, GPIO_MODE_INPUT);
	gpio_set_pull_mode(_sw, GPIO_PULLUP_ONLY);

}

bool Switch::isClosed() {

	int level = gpio_get_level(_sw );
	if( level )
		return false;
	else
		return true;
}

bool Switch::isOpen() {
	return( !isClosed() );
}


void Switch::tick() {
	unsigned long currentMillis = millis();
	// ESP_LOGI(FNAME,"tick %ld millis", currentMillis );
	if( _holddown){
		_holddown--;
	}
	else{
		switch (_state) {
		case B_IDLE:
			if (isClosed()) {  // Button pressed
				_state = B_PRESSED;
				p_time = currentMillis;
				ESP_LOGI(FNAME,"PRESSED" );
			}
			break;

		case B_PRESSED:
			if (isOpen()) {
				int dur = currentMillis - p_time;
				ESP_LOGI(FNAME,"OPEN, dur=%d",dur );
				// Button pressed longer as 300 mS
				if (dur < 300) {
					sendPress();
					_holddown = 5;  // 50 mS debounce
				}
				else if (dur > 300 && dur < 2000 ) {
					sendLongPress();
					_holddown = 5;
				}
				_state = B_IDLE;
			}
			else{
				int dur = currentMillis - p_time;
				ESP_LOGI(FNAME,"STILL CLOSED, but dur=%d",dur );
				if (dur > 2000) {   // we implement this as a long long click
					sendLongLongPress();
					_holddown = 5;
					_state = B_PRESSED_STILL;
				}
			}
			break;

		case B_PRESSED_STILL:
			if (isOpen()) {
				_state = B_IDLE;
				_holddown = 5;
			}
			break;
		}
	}
}

void Switch::sendPress(){
	ESP_LOGI(FNAME,"send press");
	for (auto &observer : observers){
		if( _mode == B_MODE ){
			observer->press();
		}
		else if( _mode == B_UP ){
			observer->up(1);
		}
		else if( _mode == B_DOWN ){
			observer->down(1);
		}
	}
	// ESP_LOGI(FNAME,"End pressed action");
}

void Switch::sendLongPress(){
	ESP_LOGI(FNAME,"send longPress");
	for (auto &observer : observers)
		if( _mode == B_MODE ){
			observer->longPress();
		}
		else if( _mode == B_UP ){
			observer->up(1);
		}
		else if( _mode == B_DOWN ){
			observer->down(1);
		}
	// ESP_LOGI(FNAME,"End long pressed action");
}

void Switch::sendLongLongPress(){
	ESP_LOGI(FNAME,"send long long press");
	for (auto &observer : observers)
		if( _mode == B_MODE ){
			observer->longLongPress();
		}
		else if( _mode == B_UP ){
			observer->up(1);
		}
		else if( _mode == B_DOWN ){
			observer->down(1);
		}
	// ESP_LOGI(FNAME,"End long pressed action");
}


