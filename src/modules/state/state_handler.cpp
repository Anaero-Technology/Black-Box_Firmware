#include <modules/state/state_handler.h>
#include <modules/sd_card/sd_handler.h>

extern SD_Handler sd_card;

State_Handler::State_Handler(){};

void State_Handler::reset() {
    event_number = -1;
    logging = false;
}

bool State_Handler::set_logging(bool state) {
    logging = state;
    if (state) {
        if (strlen(sd_card.get_logging_file()) < 1) {
            return false;
        }
    } else {
        sd_card.reset_logging_file();
    }
    return sd_card.set_setup(state);
}

bool State_Handler::get_logging() {
    return logging;
}

void State_Handler::set_event_number(int new_number) {
    event_number = new_number;
}

void State_Handler::increment_event_number() {
    event_number = event_number + 1;
}

int State_Handler::get_event_number() {
    return event_number;
}

void State_Handler::reset_tip_counters() {
    for (int index = 0; index < 15; index = index + 1) {
        tip_counters[index] = 0;
    }
}

void State_Handler::increment_tip_counter(int channel) {
    if (channel > 0 && channel < 16) {
        tip_counters[channel - 1] = tip_counters[channel - 1] + 1;
    }
}

int State_Handler::get_tip_counter(int channel) {
    if (channel > 0 && channel < 16) {
        return tip_counters[channel - 1];
    }
    return -1;
}

void State_Handler::output_tip_counters() {
    Serial.print("counts ");
    for (int index = 0; index < 15; index = index + 1){
        if (index < 14) {
            Serial.print(tip_counters[index]);
            Serial.print(" ");
        } else {
            Serial.println(tip_counters[index]);
        }
    }
}