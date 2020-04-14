#ifndef SAMPLE_H
#define SAMPLE_H

struct Phaser{
    uint8_t latch_status;
    uint16_t index;
    uint32_t accumulator;
    uint16_t speed = 128;
};


class SamplePlayback{
    public:
        SamplePlayback(uint16_t l, uint8_t p, uint8_t n){ 
            length = l; 
            button_pin = p;
            midi_note = n;
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

        uint16_t getSampleLength(){ return length; }

        void setTriggerFlag(uint8_t incoming_midi_note){
            trigger_flag = fell() || incoming_midi_note == midi_note;
        }
        uint8_t getTriggerFlag(){ return trigger_flag; }

        void setAccumulator(uint16_t a, uint8_t p = 0){ 
            if(a != 0) phaser[p].accumulator += a; 
            else phaser[p].accumulator = 0;
        }

        void trigger(uint8_t p = 0){ 
            phaser[p].accumulator = 0;
            phaser[p].latch_status = 1;
            phaser[p].index = 0;  
        }
        
        void reset(uint8_t p = 0){
            phaser[p].accumulator = 0;
            phaser[p].latch_status = 0;
            phaser[p].index = 0;
        }

        bool latched(uint8_t p = 0){ return phaser[p].latch_status; }
        void latch(uint8_t p = 0){ phaser[p].latch_status = 1; }
        void unlatch(uint8_t p = 0){ phaser[p].latch_status = 0; }

        void setSpeed(uint16_t s, uint8_t p = 0){ phaser[p].speed = s; }
        uint16_t getSpeed(uint8_t p = 0){ return phaser[p].speed; }

        void update(uint8_t p){
            if (phaser[p].latch_status == 1) {
                phaser[p].accumulator += phaser[p].speed;
                phaser[p].index = (phaser[p].accumulator >> 6);
                if (phaser[p].index > length) reset(p);
            }
        }
        void update(){
            update(0);
            update(1);
        }

        uint16_t getIndex(uint8_t p = 0){ return phaser[p].index; }    

        void setStep(uint8_t index, uint8_t value, uint16_t s){ 
            sequence[index] = value; 
            if (speed_sequence != nullptr) speed_sequence[index] = s;
        }
        uint8_t getStep(uint8_t index){ return sequence[index]; }

        void setSpeedSequence(uint16_t *s){ speed_sequence = s; }
        void setSpeedStep(uint8_t index, uint16_t value){ speed_sequence[index] = value; }
        uint16_t getSpeedStep(uint8_t index){ return speed_sequence[index]; }

        void setLoopTrigger(uint8_t index){ loop_trigger = sequence[index]; }
        uint8_t getLoopTrigger(){ return loop_trigger; }
        uint8_t getMidiNote(){ return midi_note; }

    private:
        // Sample
        uint16_t length;
        Phaser phaser[2]; // 2nd phaser is used for variable speed sequencing

        // MIDI
        uint8_t midi_note;

        // Trigger Button
        uint8_t button_pin = 0;
        Bounce button_debouncer = Bounce();
        uint8_t trigger_flag;

        // Sequencer
        uint8_t sequence[128] = {};
        uint16_t *speed_sequence; // not enough RAM for 4x speed sequences...
        uint8_t loop_trigger;
};


#endif