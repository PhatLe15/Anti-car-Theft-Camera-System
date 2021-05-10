//#include <wiringPi.h>
//#include <softPwm.h>

#include "servo.h"

//int pin = 17;

//move servo to a specific location
int servo_position(int position){ //0 25 90 135 or 180 degree
  int pulse;
  switch (position)
  {
    case 0:
      pulse = 5;
      break;
    case 1:
      pulse = 10;
      break;
    case 2:
      pulse = 15;
      break;
    case 3:
      pulse = 20;
      break;
    case 4:
      pulse = 25;
      break;
    default:
      pulse = 5;
      break;
    return pulse;
  }
}

/* sample main file

int main(void){ //tesing main

  wiringPiSetupGpio();

  pinMode(pin,PWM_OUTPUT);

  digitalWrite(pin, LOW);

  softPwmCreate(pin,servo_position(2),200);
  delay(1000);

  softPwmWrite(pin,servo_position(4));
  delay(500);

  // softPwmWrite(pin,servo_position(1));
  // delay(500);

  // softPwmWrite(pin,servo_position(2));
  // delay(500);

  // softPwmWrite(pin,servo_position(3));
  // delay(500);

  // softPwmWrite(pin,servo_position(4));
  // delay(500);


  softPwmStop(pin);

  
  return 0;
}

*/