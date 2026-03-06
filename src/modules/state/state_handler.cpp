#include <modules/state/state_handler.h>

State_Handler::State_Handler(){};

void State_Handler::reset() {
    event_number = 1;
    logging = false;
}

void State_Handler::set_logging(bool state) {
    logging = state;
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