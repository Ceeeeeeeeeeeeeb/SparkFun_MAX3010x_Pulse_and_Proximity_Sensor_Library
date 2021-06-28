/*************************************************** 
 This is a library written for the Maxim MAX30105 Optical Smoke Detector
 It should also work with the MAX30102. However, the MAX30102 does not have a Green LED.

 These sensors use I2C to communicate, as well as a single (optional)
 interrupt line that is not currently supported in this driver.
 
 Written by Peter Jansen and Nathan Seidle (SparkFun)
 BSD license, all text above must be included in any redistribution.
 *****************************************************/

#pragma once

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>

#define MAX30102_ADDRESS          0x57 //7-bit I2C Address
//Note that MAX30102 has the same I2C address and Part ID

#define I2C_SPEED_STANDARD        100000
#define I2C_SPEED_FAST            400000

//Define the size of the I2C buffer based on the platform the user has�����û�ӵ�е�ƽ̨����I2C�������Ĵ�С
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)

  //I2C_BUFFER_LENGTH is defined in Wire.H
  #define I2C_BUFFER_LENGTH BUFFER_LENGTH

#elif defined(__SAMD21G18A__)

  //SAMD21 uses RingBuffer.h
  #define I2C_BUFFER_LENGTH SERIAL_BUFFER_SIZE

#else

  //The catch-all default is 32ͨ��Ĭ��ֵΪ32
  #define I2C_BUFFER_LENGTH 32

#endif

class MAX30102 {
 public: 
  MAX30102(void);

  boolean begin(TwoWire &wirePort = Wire, uint32_t i2cSpeed = I2C_SPEED_STANDARD, uint8_t i2caddr = MAX30102_ADDRESS);

  uint32_t getRed(void); //Returns immediate red value����������ɫֵ
  uint32_t getIR(void); //Returns immediate IR value��������IRֵ
  bool safeCheck(uint8_t maxTimeToCheck); //Given a max amount of time, check for new data�������ʱ�䣬���������

  // Configuration����
  void softReset();
  void shutDown(); 
  void wakeUp(); 

  void setLEDMode(uint8_t mode);

  void setADCRange(uint8_t adcRange);
  void setSampleRate(uint8_t sampleRate);
  void setPulseWidth(uint8_t pulseWidth);

  void setPulseAmplitudeRed(uint8_t value);
  void setPulseAmplitudeIR(uint8_t value);
  void setPulseAmplitudeProximity(uint8_t value);

  void setProximityThreshold(uint8_t threshMSB);

  //Multi-led configuration mode (page 22)��LED����ģʽ
  void enableSlot(uint8_t slotNumber, uint8_t device); //Given slot number, assign a device to slot������ۺţ����豸��������
  void disableSlots(void);
  
  // Data Collection

  //Interrupts (page 13, 14)
  uint8_t getINT1(void); //Returns the main interrupt group�������ж���
  uint8_t getINT2(void); //Returns the temp ready interrupt������ʱ�����ж�
  void enableAFULL(void); //Enable/disable individual interrupts����/���õ����ж�
  void disableAFULL(void);
  void enableDATARDY(void);
  void disableDATARDY(void);
  void enableALCOVF(void);
  void disableALCOVF(void);
  void enablePROXINT(void);
  void disablePROXINT(void);
  void enableDIETEMPRDY(void);
  void disableDIETEMPRDY(void);

  //FIFO Configuration (page 18)FIFO����
  void setFIFOAverage(uint8_t samples);
  void enableFIFORollover();
  void disableFIFORollover();
  void setFIFOAlmostFull(uint8_t samples);
  
  //FIFO Reading FIFO��ȡ
  uint16_t check(void); //Checks for new data and fills FIFO��������ݲ����FIFO
  uint8_t available(void); //Tells caller how many new samples are available (head - tail)���ߺ������ж��ٸ�����������
  void nextSample(void); //Advances the tail of the sense array�ƽ���֪���е�β��
  uint32_t getFIFORed(void); //Returns the FIFO sample pointed to by tail����β��ָ���FIFO����
  uint32_t getFIFOIR(void); //Returns the FIFO sample pointed to by tail

  uint8_t getWritePointer(void);
  uint8_t getReadPointer(void);
  void clearFIFO(void); //Sets the read/write pointers to zero���ö�/дָ��Ϊ��

  //Proximity Mode Interrupt Threshold�ӽ�ģʽ�ж���ֵ
  void setPROXINTTHRESH(uint8_t val);

  // Die Temperature�¶�
  float readTemperature();
  float readTemperatureF();

  // Detecting ID/Revision ���ID /�޶�
  uint8_t getRevisionID();
  uint8_t readPartID();  

  // Setup the IC with user selectable settings ʹ���û���ѡ��������IC
  void setup(byte powerLevel = 0x1F, byte sampleAverage = 4, byte ledMode = 3, int sampleRate = 400, int pulseWidth = 411, int adcRange = 4096);

  // Low-level I2C communication�ͼ���I2Cͨ�� 
  uint8_t readRegister8(uint8_t address, uint8_t reg);
  void writeRegister8(uint8_t address, uint8_t reg, uint8_t value);

 private:
  TwoWire *_i2cPort; //The generic connection to user's chosen I2C hardware���û�ѡ���I2CӲ����ͨ������
  uint8_t _i2caddr;

  //activeLEDs is the number of channels turned on, and can be 1 to 3. 2 is common for Red+IR. activeLEDs�Ǵ򿪵�ͨ������������1��3��Red+ IR����2��
  byte activeLEDs; //Gets set during setup. Allows check() to calculate how many bytes to read from FIFO�����ù��������á� ����check��������Ҫ��FIFO��ȡ���ٸ��ֽ�
  
  uint8_t revisionID; 

  void readRevisionID();

  void bitMask(uint8_t reg, uint8_t mask, uint8_t thing);
 
   #define STORAGE_SIZE 4 //Each long is 4 bytes so limit this to fit on your microÿ��longΪ4���ֽڣ����������Ϊ�ʺ�����micro
  typedef struct Record
  {
    uint32_t red[STORAGE_SIZE];
    uint32_t IR[STORAGE_SIZE];
    byte head;
    byte tail;
  } sense_struct; //This is our circular buffer of readings from the sensor�������ǴӴ�������ȡ��ѭ��������

  sense_struct sense;

};
