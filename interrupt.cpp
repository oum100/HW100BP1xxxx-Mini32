#include "interrupt.h"
#include <Arduino.h>

#include "driver/gpio.h"

gpio_config_t io_config;
xQueueHandle gpio_evt_queue = NULL;
int coin=0;
int bill=0;

void IRAM_ATTR gpio_isr_handler(void* arg)
{
  long gpio_num = (long) arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

int getCoin(){
    return coin;
}

void resetCoin(){
    coin =0;
    //stateflag=1?0:1;
}

int getBill(){
    return bill;
}

void resetBill(){
    coin = 0;
}

void gpio_task(void *arg){
    gpio_num_t io_num;  

    for(;;){
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {  
            Serial.printf("\nGPIO[%d] intr, val: %d \n", io_num, gpio_get_level(io_num));
        } 

        switch (io_num){
            case COININ:
                (gpio_get_level(io_num) == 0)?coin++:coin=coin;   
                break;
            case BILLIN:
                (gpio_get_level(io_num) == 0)?bill++:bill=bill; 
                break;
            case MODESW:
                break;
        }  
    }
}

void interrupt(){
    //gpio_config_t io_conf;

    //io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_config.intr_type = GPIO_INTR_NEGEDGE;
    //io_conf.intr_type = GPIO_INTR_POSEDGE;
    //io_conf.intr_type = GPIO_INTR_LOW_LEVEL;
    //io_conf.intr_type = GPIO_INTR_HIGH_LEVEL;
    io_config.pin_bit_mask = INPUT_PIN;
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pull_up_en = (gpio_pullup_t)1;

    //configure GPIO with the given settings
    gpio_config(&io_config);

    /*********** create a queue to handle gpio event from isr ************/
    gpio_evt_queue = xQueueCreate(10, sizeof(long));

    /*********** Set GPIO handler task ************/
    xTaskCreate(gpio_task, "gpio_task", 1024, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

    #ifdef HAIERDRYER123
        gpio_isr_handler_add((gpio_num_t)BILLIN, gpio_isr_handler, (void*) BILLIN); 
        gpio_isr_handler_add((gpio_num_t)COININ, gpio_isr_handler, (void*) COININ);
        gpio_isr_handler_add((gpio_num_t)MODESW, gpio_isr_handler, (void*) MODESW);
    #endif
}