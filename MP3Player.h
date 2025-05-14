#include "HardwareSerial.h"
#ifndef MP3Player_h

#define MP3Player_h

/************ Commands ************/
#define CMD_SET_VOLUME    0X06

#define CMD_SEL_DEV       0X09
#define CMD_SLEEP_MODE    0X0A
#define CMD_WAKE_UP       0X0B
#define CMD_RESET         0X0C
#define CMD_PLAY_FOLDER_FILE 0X0F

#define CMD_QUERY_STATUS      0x42
#define CMD_QUERY_TOT_TRACKS  0x48

#define DEV_TF            0X0002

#define COMMAND_BEGIN     0X7E
#define COMMAND_VERSION   0XFF
#define COMMAND_LENGTH    0X06
#define COMMAND_NO_FEEDBACK 0X00
#define COMMAND_FEEDBACK  0X01
#define COMMAND_END       0XEF


class MP3Player {
  public:
    MP3Player();
    MP3Player(HardwareSerial& serial, int RX, int TX);

    /**
    * Execute the player loop.
    */
    void playerLoop();

    /**
    * Resets the device.
    */
    void reset();

    /**
    * Register function for handling track ended message. 
    * For handler to work you need to execute player loop in the main program loop.
    */
    void registerTrackEndedHandler(void (*trackEndedHandler)());

    /**
    * @return number of available tracks.
    */
    int getTrackCount();

    /**
    * @return player status(0=stopped, 1=play, 2=paused).
    */
    int getPlayerStatus();

    /**
    * Sets the playback volume of the device.
    *
    * @param volume: value between 0 and 30.
    */
    void setVolume(uint8_t volume);

    /**
    * Plays the track with the specified number located in the folder with the specified number.
    */
    void playFolderTrack(uint8_t folderNumber, uint8_t trackNumber);

    /**
    * Puts the device to sleep.
    * WARNING: Remember to wake up the device after sleep.
    */
    void sleep();

    /**
    * Wakes up the device.
    */
    void wake();


  private:
    HardwareSerial *mp3Serial;
    void (*trackEndedHandler)() = NULL;

    void sendCommand(uint8_t command, uint8_t highParam, uint8_t lowParam);
    void sendCommand(uint8_t command, uint8_t lowParam);
    void sendCommand(uint8_t command);
    int16_t sendCommandWithResponse(uint8_t command);

    bool isDebugging = true;

    void debugMessage(String message);

    /**
    * Waiting for a response from the device for a specified duration.
    *
    * @return True if the device answerd, False otherwise.
    */
    bool waitForAnswer(unsigned long duration);

    void handleMp3Answer();

    int hexToInt(int16_t hexValue);
};

#endif