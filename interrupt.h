#ifndef interrupt_h
#define interrupt_h
    #include <Arduino.h>
    #include "driver/gpio.h"

    #define COININ 35
    #define MODESW 39
    #define BILLIN  19
    #define INPUT_PIN ( (1ULL<<COININ)|(1ULL<<BILLIN) )

  
    void IRAM_ATTR gpio_isr_handler1(void* arg);
    void gpio_task1(void *arg);
    void interrupt();

    int getCoin();
    void resetCoin();
    int getBill();
    void resetBill();

   
    
#endif