#include <modules/state/state_handler.h>
#include <modules/sd_card/sd_handler.h>

extern SD_Handler sd_card;

State_Handler::State_Handler(){};

void State_Handler::reset() {
    event_number = 1;
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