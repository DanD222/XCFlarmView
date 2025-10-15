#pragma once

#include <driver/gpio.h>
#include <list>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef MAIN_SWITCH_H_
#define MAIN_SWITCH_H_

class SwitchObserver;

// Parameters
#define TASK_PERIOD_MS      10      // Task every 10 ms
#define DEBOUNCE_MS         50      // debounce time
#define LONG_PRESS_MS       300     // Long Press from 300 ms
#define LONG_LONG_PRESS_MS  2000    // Long-Long Press (or hold) 2 s

typedef enum { B_MODE, B_UP, B_DOWN } t_button;
typedef enum { B_IDLE, B_PRESSED, B_PRESSED_STILL } t_button_state;

class Switch {
public:
    Switch();
    virtual ~Switch();

    void begin(gpio_num_t sw, t_button mode = B_MODE);
    bool isClosed();
    bool isOpen();
    void tick();   // Aufruf automatisch durch Task

    static void attach(SwitchObserver* obs);
    static void detach(SwitchObserver* obs);

    void sendPress(int dur);
    void sendLongPress(int dur);
    void sendLongLongPress(int dur);

    static void startTask();

private:
    static void switchTask(void* pvParameters);
    void notifyObserversPress();

    static std::list<SwitchObserver*> observers;
    static std::list<Switch*> instances;

    gpio_num_t _sw;
    t_button_state _state;
    t_button _mode;

    int _holddown;          // Debounce Counter
    long p_time;            // Pressed time
    long repeat_timer;      // Millis until next
    bool repeating;         // Repeat mode active

    static TaskHandle_t pid; // Task Handle
};

class SwitchObserver {
public:
    virtual void press() = 0;
    virtual void up(int count) = 0;
    virtual void down(int count) = 0;
    virtual void longPress() = 0;
    virtual void longLongPress() = 0;
    virtual ~SwitchObserver() {};

    void attach(SwitchObserver* instance) { Switch::attach(instance); }
    void detach(SwitchObserver* instance) { Switch::detach(instance); }
};

#endif /* MAIN_SWITCH_H_ */
