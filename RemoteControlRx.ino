#include <avr/sleep.h>
#include "MessageHandler.h"

#define PowerPin 2

volatile long CurrentTimer = 0;

u8 PowerButtonState = 1;
u8 PowerButtonCounter = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Serial1.begin(57600); //Rx-Tx Unit
  pinMode(PowerPin,INPUT_PULLUP);

  pinMode(LED_BUILTIN,OUTPUT);
  detachInterrupt(digitalPinToInterrupt(PowerPin));

  SendMessageRF(GeneralMessageHeader,"#Hello Serial 1");

  CurrentTimer = micros();

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN,HIGH);
  if (micros() < CurrentTimer)
  {
    CurrentTimer = micros();
    Serial.println("Timer Overflow protection.");
  }

  if (micros() - CurrentTimer > 10000 )
  {
    ReadSticksAndButtons();
    //TODO: Parse message
  }

  if (!PowerButtonState)
  {
    PowerButtonCounter++;
  }
  else
  {
    PowerButtonCounter = 0;
  }

  if (PowerButtonCounter >= 20)
  {
    GoToSleep();
  }
}

void ReadSticksAndButtons(void)
{
  PowerButtonState = digitalRead(PowerPin);

  // Serial.print("Power Stick: ");
  // Serial.println(PowerButtonState);
}

void GoToSleep(void)
{
  Serial.println("Go To Sleep.");
  sleep_enable();
  digitalWrite(LED_BUILTIN,LOW); //TODO: Change button LED behaviour
  
  while (!PowerButtonState)
  {
    PowerButtonState = digitalRead(PowerPin);
    delay(100);
  }
  delay(2000);
  // Serial.print("Power Stick before attach: ");
  // Serial.println(PowerButtonState);
  // delay(1000);
  attachInterrupt(digitalPinToInterrupt(PowerPin),WakeUp,LOW );
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  /*Set Sticks to Idle */
  sleep_cpu();
  Serial.println("Woke Up from interrupt.");

}
void WakeUp(void)
{
  PowerButtonState = digitalRead(PowerPin);
  while (!PowerButtonState)
  {
    PowerButtonState = digitalRead(PowerPin);
    delay(100);
  }
  delay(500);
  Serial.println("Interrupt Detected.");
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(PowerPin));
  while (!PowerButtonState)
  {
    PowerButtonState = digitalRead(PowerPin);
    delay(100);
  }
  delay(500);
  PowerButtonCounter = 0;
}

int ConvertStickToPWMSignal(u16 Center, int StickMeassurement)
{
  u32 TempStick = (StickMeassurement / 2) * 2;
  // Serial.print("TempStick Stick: ");
  // Serial.println(TempStick);
  int PWMSignalValueCenter = (Center*127 ) /512;
  int PWMSignalValue = PWMSignalValueCenter;
  // Serial.print("PWMSignalValueCenter: ");
  // Serial.println(PWMSignalValueCenter);
  if ( (TempStick > Center - 10) && (TempStick < Center + 10) )
  {
    TempStick = Center;
  }
  PWMSignalValue = (int)((TempStick * 127 ) /512 - PWMSignalValueCenter);
  // Serial.print("PWMSignalValue: ");
  // Serial.println(PWMSignalValue);
  return (int)PWMSignalValue;
}

void ParseRFMessage(String MessageToParse)
{
  int HeaderIndex = MessageToParse.indexOf(SticksMessageHeader);
  if (HeaderIndex != -1)
  {
    int MessageSizeEndIndex = MessageToParse.indexOf(",",HeaderIndex + 7);
    String MessageSizeString = MessageToParse.substring(HeaderIndex + 7,MessageSizeEndIndex);
    long MessageSize = MessageSizeString.toInt();
    String MessageBody = MessageToParse.substring()
  }
}
