#include <avr/sleep.h>
#include "MessageHandler.h"
#include "DCMotorControl.h"

#define PowerPin 2

volatile long CurrentTimer = 0;
volatile long LastReceivedMessageTimer = 0;

u8 PowerButtonState = 1;
u8 PowerButtonCounter = 0;

int ThrottlePWMValue = 0;
int LeftRightPWMValue = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(57600); //Rx-Tx Unit
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

  if ( (micros() - CurrentTimer > 10000 ) )
  {
    ReadSticksAndButtons();
  }

  if ( (micros() - LastReceivedMessageTimer < 25000) )
  {
    ControlDualMotors(ThrottlePWMValue, LeftRightPWMValue);
  }
  else
  {
    FullStop();
    ThrottlePWMValue = 0;
    LeftRightPWMValue = 0;
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
  // AB56FE21,6,156,#127,0
  // Header,Size,Counter,Body
  int HeaderIndex = MessageToParse.indexOf(SticksMessageHeader);
  int EndOfMessageIndex = MessageToParse.indexOf('~');
  if ( (HeaderIndex != -1) && (EndOfMessageIndex != -1) )
  {
    int MessageSizeEndIndex = MessageToParse.indexOf(",",HeaderIndex + 8);
    String MessageSizeString = MessageToParse.substring(HeaderIndex + 8,MessageSizeEndIndex);
    long MessageSize = MessageSizeString.toInt();
    int MessageBodyBegining = MessageToParse.indexOf("#",HeaderIndex + 8);
    String MessageBody = MessageToParse.substring(MessageBodyBegining+1,EndOfMessageIndex);
    int MessageBodySeperatorIndex = MessageToParse.indexOf(",",MessageBodyBegining + 1);
    String MessageBodyThrottleString = MessageToParse.substring(MessageBodyBegining+1,MessageBodySeperatorIndex);
    String MessageBodyLeftRightString = MessageToParse.substring(MessageBodySeperatorIndex+1,EndOfMessageIndex);

    ThrottlePWMValue = MessageBodyThrottleString.toInt();
    LeftRightPWMValue = MessageBodyLeftRightString.toInt();
  }
}

void serialEvent1() 
{
  LastReceivedMessageTimer = micros();
  TextFromPort = Serial1.readStringUntil(terminator);
  if (!TextFromPort.equals("\r\n\n"))
  {
    //Serial.println("aaa");
    Serial.println(TextFromPort);
    //Serial1.println(TextFromPort);
    ParseRFMessage(TextFromPort);
    //GeneralMessage.concat(TextFromPort);
    //SendMEssageRF(GeneralMessageHeader,GeneralMessage);
  }
}