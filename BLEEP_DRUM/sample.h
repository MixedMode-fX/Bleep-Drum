#ifndef SAMPLE_H
#define SAMPLE_H


class Sample{
    public:
        Sample(byte **t, uint16_t l){ 
            table = t; 
            length = l; 
        };

        uint16_t getIndex(){ return index; }    
        byte getSample(){ 
            return pgm_read_byte(&table[index]) - 127; 
        }
        byte getReverseSample(){
            // same applies here /!?
            return pgm_read_byte(&table[(length - index)]) - 127;
        }
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
        byte **table;
        uint16_t length;
        uint16_t index;
        uint32_t accumulator;
        
        uint8_t latch_status;

        uint16_t speed = 128;
};


#endif