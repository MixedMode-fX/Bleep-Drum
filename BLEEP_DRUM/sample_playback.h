#ifndef SAMPLE_H
#define SAMPLE_H

// #include "Arduino.h"


class SamplePlayback{
    public:
        SamplePlayback(uint16_t l){ 
            length = l; 
        };
        SamplePlayback(uint16_t l, uint8_t p){ 
            length = l; 
            button_pin = p;
        };

        void setup(){
            if (button_pin != 0) {
                pinMode(button_pin, INPUT_PULLUP);
                button_debouncer.attach(button_pin);
                button_debouncer.interval(2);
            }
        }

        Bounce button(){ return button_debouncer; }
        void btn_update(){ button_debouncer.update(); }
        bool read(){ return button_debouncer.read(); }
        bool fell(){ return button_debouncer.fell(); }
        bool rose(){ return button_debouncer.rose(); }

        uint16_t getIndex(){ return index; }    
        uint16_t getSampleLength(){ return length; }
        void setAccumulator(uint16_t a){ 
            if(a != 0) accumulator += a; 
            else accumulator = 0;
        }
        void trigger(){ 
            accumulator = 0;
            latch_status = 1;
            index = 0;  
        }

        void reset(){
            accumulator = 0;
            latch_status = 0;
            index = 0;
        }

        bool latched(){ return latch_status; }
        void latch(){ latch_status = 1; }
        void unlatch(){ latch_status = 0; }
        void setSpeed(uint16_t s){ speed = s; }
        uint16_t getSpeed(){ return speed; }

        void update(){
            if (latch_status == 1) {
                accumulator += speed;

                index = (accumulator >> 6);

                if (index > length) {
                    index = 0;
                    accumulator = 0;
                    latch_status = 0;
                }
            }
        }


    private:
        uint16_t length;
        uint16_t index;
        uint32_t accumulator;
        uint8_t latch_status;
        uint16_t speed = 128;

        uint8_t button_pin = 0;
        Bounce button_debouncer = Bounce(); 
};


#endif