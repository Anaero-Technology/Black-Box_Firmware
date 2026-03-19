#include <modules/rtc/rtc_handler.h>
#include <modules/sd_card/sd_handler.h>
#include <modules/state/state_handler.h>

extern SD_Handler sd_card;
extern State_Handler state;

RTC_Handler::RTC_Handler(){};

bool RTC_Handler::begin() {
    return rtc.begin();
}

void RTC_Handler::store_time(){
    /*Set the start time to the current time so that it can be used to retrieve seconds since start*/
    set_experiment_start(get_seconds_since_epoch());
    sd_card.set_time_file(get_experiment_start());
}

void RTC_Handler::get_unix_time(){
    unsigned long unix_time = rtc.now().unixtime();
    Serial.print("time ");
    Serial.println(unix_time);
}

char* RTC_Handler::get_time_stamp(){
    /*Send the timestamp over the serial connection*/
    //Char buffer to hold timestamp
    //Get the current time
    DateTime time_now = rtc.now();
    char delimeter = ' ';
    Serial.print("time ");
    char time_buffer[22] = "YYYY.MM.DD.hh.mm.ss";
    time_now.toString(time_buffer);
    strcpy(time_stamp_output, time_buffer);
    return time_stamp_output;
}

void RTC_Handler::set_time_seconds(unsigned long seconds){
    /*Set the current time on the real time clock using seconds since unix epoch*/
    rtc.adjust(DateTime(seconds));
}

uint32_t RTC_Handler::get_seconds_since_epoch(){
    /*Returns the number of seconds since the unix epoch*/
    uint32_t epochTime = rtc.now().unixtime();
    return epochTime;
}

void RTC_Handler::set_experiment_start(unsigned long new_time) {
    experiment_start = new_time;
}

unsigned long RTC_Handler::get_experiment_start() {
    return experiment_start;
}

void RTC_Handler::store_current_hour_start() {
    hour_start_time = millis();
}

bool RTC_Handler::update_hour_time() {
    if(state.get_logging()){
        unsigned long current_time = millis();
        unsigned long elapsed = current_time - hour_start_time;
        if (current_time < hour_start_time){
            elapsed = (ULONG_MAX - hour_start_time) + current_time;
        }
        if (elapsed >= HOUR_LENGTH){
            hour_start_time = hour_start_time + HOUR_LENGTH;
            return true;
        }
    }
    return false;
}

bool RTC_Handler::check_elapsed(unsigned long start, unsigned long current, unsigned long duration) {
    unsigned long elapsed = current - start;
    if (start > current) {
        elapsed = current + (ULONG_MAX - start);
    }
    return elapsed >= duration;
}