//motor A connected between A01 and A02
//motor B connected between B01 and B02

int STBY = 10; //standby

//Motor A
int PWMA = 3; //Speed control
int AIN1 = 9; //Direction
int AIN2 = 8; //Direction

//Motor B
int PWMB = 5; //Speed control
int BIN1 = 11; //Direction
int BIN2 = 12; //Direction

int PWMThrottle = 0;

void ControlSingleMotor(int motor, int speed, int direction)
{
    //Move specific motor at speed and direction
    //motor: 0 for B 1 for A
    //speed: 0 is off, and 255 is full speed
    //direction: 0 clockwise, 1 counter-clockwise

    digitalWrite(STBY, HIGH); //disable standby

    boolean inPin1 = LOW;
    boolean inPin2 = HIGH;

    if (direction == 1)
    {
        inPin1 = HIGH;
        inPin2 = LOW;
    }

    if (motor == 1)
    {
        digitalWrite(AIN1, inPin1);
        digitalWrite(AIN2, inPin2);
        analogWrite(PWMA, speed);
    }
    else
    {
        digitalWrite(BIN1, inPin1);
        digitalWrite(BIN2, inPin2);
        analogWrite(PWMB, speed);
    }
}

void ControlDualMotors(int ThrottleSignal, int LeftRightSignal)
{
    //Move specific motor at speed and direction
    //motor: 0 for B 1 for A
    //speed: 0 is off, and 255 is full speed
    //direction: 0 clockwise, 1 counter-clockwise

    digitalWrite(STBY, HIGH); //disable standby
    PWMThrottle = ThrottleSignal + 127;
    if (PWMThrottle < 15)
    {
        PWMThrottle = 0;
    }

    boolean inPin1 = LOW;
    boolean inPin2 = HIGH;

    if (LeftRightSignal >= 0)
    {
        inPin1 = HIGH;
        inPin2 = LOW;
    }
    else if (LeftRightSignal < 0)
    {
        inPin1 = LOW;
        inPin2 = HIGH;
    }
    

    if (LeftRightSignal != 0)
    {
        digitalWrite(AIN1, inPin1);
        digitalWrite(AIN2, inPin2);
        analogWrite(PWMA, PWMThrottle);
        digitalWrite(BIN1, !inPin1);
        digitalWrite(BIN2, !inPin2);
        analogWrite(PWMB, PWMThrottle);
    }
    else
    {
        digitalWrite(AIN1, inPin1);
        digitalWrite(AIN2, inPin2);
        analogWrite(PWMA, PWMThrottle);
        digitalWrite(BIN1, inPin1);
        digitalWrite(BIN2, inPin2);
        analogWrite(PWMB, PWMThrottle);
    }
}

void FullStop(void)
{
    //enable standby
    digitalWrite(STBY, LOW);
}