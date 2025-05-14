#include "MP3Player.h"

MP3Player::MP3Player(){}

MP3Player::MP3Player(HardwareSerial& serial, int RX, int TX) {
  mp3Serial = &serial;
  mp3Serial->begin(9600, SERIAL_8N1, RX, TX);
  delay(500);

  reset();
  sendCommand(CMD_SEL_DEV, DEV_TF);
  delay(500);
}

void MP3Player::playerLoop() {
  if (mp3Serial->available())
  {
    handleMp3Answer();
  }
}

void MP3Player::reset() {
  sendCommand(CMD_RESET);
}

void MP3Player::registerTrackEndedHandler(void (*trackEndedHandler)()){
  this->trackEndedHandler = trackEndedHandler;
}

int MP3Player::getTrackCount() {
  return hexToInt(sendCommandWithResponse(CMD_QUERY_TOT_TRACKS));
}

int MP3Player::getPlayerStatus() {
  return hexToInt(sendCommandWithResponse(CMD_QUERY_STATUS) & 0X00FF);
}

int MP3Player::hexToInt(int16_t hexValue) {
  return static_cast<int>(hexValue);
}

void MP3Player::setVolume(uint8_t volume) {
  if(volume > 30) 
    volume = 30;
  else if(volume < 0)
    volume = 0;

  sendCommand(CMD_SET_VOLUME, volume);
}

void MP3Player::playFolderTrack(uint8_t folderNumber, uint8_t trackNumber) {
  debugMessage("Playing track: " + String(trackNumber));
  sendCommand(CMD_PLAY_FOLDER_FILE, folderNumber, trackNumber);
}

void MP3Player::sleep() {
  sendCommand(CMD_SLEEP_MODE);
}

void MP3Player::wake() {
  sendCommand(CMD_WAKE_UP);
}

void MP3Player::sendCommand(uint8_t command, uint8_t highParam, uint8_t lowParam) {
  static uint8_t sendBuffer[8] = {0};

  sendBuffer[0] = COMMAND_BEGIN;
  sendBuffer[1] = COMMAND_VERSION;
  sendBuffer[2] = COMMAND_LENGTH;
  sendBuffer[3] = command;
  sendBuffer[4] = COMMAND_NO_FEEDBACK;
  sendBuffer[5] = highParam;
  sendBuffer[6] = lowParam;
  sendBuffer[7] = COMMAND_END;

  for (uint8_t i = 0; i < 8; i++) {
    mp3Serial->write(sendBuffer[i]);
  }
}

void MP3Player::sendCommand(uint8_t command, uint8_t lowParam) {
  sendCommand(command, 0, lowParam);
}

void MP3Player::sendCommand(uint8_t command) {
  sendCommand(command, 0, 0);
}

int16_t MP3Player::sendCommandWithResponse(uint8_t command) {
  sendCommand(command);
  unsigned long timeOut = millis() + 8000;

  uint8_t index = 0;
  static uint8_t answerBuffer[10] = {0};

  while (true) {
    if(millis() > timeOut)
      return -1;

    if (!waitForAnswer(2000))
      return -2;

    uint8_t dataByte = 0;
    if(mp3Serial->available()) {
      dataByte = mp3Serial->read();
      answerBuffer[index++] = dataByte;
    }

    if (index < 10 && dataByte != COMMAND_END)
      continue;

    index = 0;

    if(answerBuffer[3] != command){
      debugMessage("Response to unexpected command " + String(answerBuffer[3], HEX));
      memset(answerBuffer, 0, sizeof(answerBuffer));
      continue;
    }

    uint16_t response = (uint16_t)answerBuffer[5] << 8 | answerBuffer[6];
    return response;
  }
}

void MP3Player::debugMessage(String message) {
  Serial.println(message);
}

bool MP3Player::waitForAnswer(unsigned long duration) {
  unsigned long timeOut = millis() + duration;

  while(!mp3Serial->available()){
    if (millis() > timeOut) {
      return false;
    }
  }
  return true;
}

void MP3Player::handleMp3Answer() {
  static uint8_t answerBuffer[10] = {0};

  uint8_t i = 0;
  while (mp3Serial->available() && (i < 10))
  {
    uint8_t b = mp3Serial->read();
    answerBuffer[i++] = b;
  }

  switch (answerBuffer[3]) {
    case 0x3A:
      debugMessage("Card Inserted");
      break;
    case 0x3B:
      debugMessage("Card Removed");
      break;
    case 0x3D:
      debugMessage("Track ended: " + String(answerBuffer[6], DEC));
      if(trackEndedHandler) trackEndedHandler();
      break;
    case 0x40:  // Error
      debugMessage("Error from player");
      break;
    default:
      debugMessage("Unexpected answer from the player" + String(answerBuffer[3], HEX));
      return;
  }
}
