#include "driver/rtc_io.h"

#define BUTTON_PIN_BITMASK(GPIO) (1ULL << GPIO)

uint64_t get_bitmask(gpio_num_t wakeUpGPIOs[], int size);

/**
* Enable wake up through passed GPIOs
*/
void hibernateSetup(gpio_num_t wakeUpGPIOs[], int size){
  uint64_t bitmask = get_bitmask(wakeUpGPIOs, size);
  esp_sleep_enable_ext1_wakeup_io(bitmask, ESP_EXT1_WAKEUP_ANY_HIGH);

  for (int i = 0; i < size; i++) {
    rtc_gpio_pulldown_en(wakeUpGPIOs[i]);
    rtc_gpio_pullup_dis(wakeUpGPIOs[i]);
  }
}


/**
* Begin ESP deep sleep
*/
void hibernate() {
    esp_deep_sleep_start();
}


/**
* Get the number of the GPIO that caused wake up
*
* @return number of GPIO or -1 if GPIO wasn't the cause of the wake up
*/
int8_t getWakeUpGPIONumber(){
  if(esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_EXT1){
    return -1;
  }

  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  int8_t GPIO_number = log(GPIO_reason)/log(2);
  return GPIO_number;
}

/**
* Get bitmask for multiple GPIOs in the passed array
*
* @return getenrated bitmask
*/
uint64_t get_bitmask(gpio_num_t wakeUpGPIOs[], int size) {
  uint64_t bitmask = 0;

  for (int i = 0; i < size; i++) {
    bitmask = bitmask | BUTTON_PIN_BITMASK(wakeUpGPIOs[i]);
  }
  return bitmask;
}