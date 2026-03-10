#pragma once

class State_Handler {
    public:
        State_Handler();
        void reset();
        bool set_logging(bool state);
        bool get_logging();
        void set_event_number(int new_number);
        void increment_event_number();
        int get_event_number();
    private:
        bool logging = false;
        int event_number = 1;
};