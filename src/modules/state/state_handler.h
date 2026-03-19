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
        void reset_tip_counters();
        void increment_tip_counter(int channel);
        int get_tip_counter(int channel);
        void output_tip_counters();
    private:
        bool logging = false;
        int event_number = 1;
        int tip_counters[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};