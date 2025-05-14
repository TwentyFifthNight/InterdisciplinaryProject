#include "hibernateUtils.h"
#include <MP3Player.h>
#include <map>

#define ESP_RX 16 // Should connect to TX of the Serial MP3 Player module
#define ESP_TX 17 // Connect to RX of the module
#define POT_PIN 27 // Connect to the potentiometer

#define MIN_VOL 0
#define MAX_VOL 30

HardwareSerial mp3Serial(2);
MP3Player mp3Player;
uint8_t currentVolume = 30;

/*************** WakeUp GPIO ***************/
gpio_num_t wakeUpGPIOs[] = {GPIO_NUM_33, GPIO_NUM_34};
std::map<int8_t, int> GPIOTrack = { {GPIO_NUM_33, 1}, {GPIO_NUM_34, 2} }; // Make sure that tracks are assigned to the valid GPIOs

void setup() {
  Serial.begin(115200);
  delay(500);

  hibernateSetup(wakeUpGPIOs, std::end(wakeUpGPIOs)-std::begin(wakeUpGPIOs));

  mp3Player = MP3Player(mp3Serial, ESP_RX, ESP_TX);

  if(mp3Player.getTrackCount() < 3){
    Serial.println("Not enough tracks on the SD card.");
  }

  mp3Player.registerTrackEndedHandler(handleTrackEnded);

  int8_t wakeUpGPIO = getWakeUpGPIONumber();
  if(wakeUpGPIO == -1) {
    Serial.println("Going to sleep now.");
    sleep();
  } else {
    Serial.println("Waking up.");
    mp3Player.wake();
    mp3Player.playFolderTrack(1, GPIOTrack[wakeUpGPIO]);
    
    delay(500);
    int status = mp3Player.getPlayerStatus();
    if(status != 1){
      Serial.println("Player status: " + String(status));
      Serial.println("Going to sleep.");
      sleep();
    }
  }
}

void loop() {
  mp3Player.playerLoop();
  checkForVolumeChange();
  checkForButtonClick();
  delay(100);
}


void checkForVolumeChange(){
  uint16_t potValue = analogRead(POT_PIN);
  uint8_t newVolume = map(potValue, 0, 4095, MIN_VOL, MAX_VOL);
  if(newVolume != currentVolume){
    mp3Player.setVolume(newVolume);
    currentVolume = newVolume;
  }
}

void checkForButtonClick() {
  for(gpio_num_t gpio : wakeUpGPIOs) {
    if(digitalRead(gpio) == HIGH) {
      mp3Player.playFolderTrack(1, GPIOTrack[gpio]);
      delay(500);

      int status = mp3Player.getPlayerStatus();
      if(status != 1){
        Serial.println("Player status: " + String(status));
        Serial.println("Going to sleep.");
        sleep();
      }
    }
  }
}

void handleTrackEnded(){
  Serial.println("Going to sleep now.");
  sleep();
}

void sleep() {
  mp3Player.sleep();
  hibernate();
}

