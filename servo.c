#include <wiringPi.h>
#include <softPwm.h>


// void servo_open(){
//   softPwmWrite(17,)
// }

int servo_position(int position);

int pin = 22;
int main(void){
  wiringPiSetupGpio();
  
  pinMode(22,PWM_OUTPUT);

  softPwmCreate(pin,servo_position(2),100);
  delay(500);

  softPwmWrite(pin,servo_position(0));
  delay(500);

  softPwmWrite(pin,servo_position(1));
  delay(500);

    softPwmWrite(pin,servo_position(2));
  delay(500);

  softPwmWrite(pin,servo_position(3));
  delay(500);

  softPwmWrite(pin,servo_position(4));
  delay(500);

  softPwmStop(pin);
  
  return 0;
}

int servo_position(int position){ //0 25 90 135 180
  int pulse;
  switch (position)
  {
    case 0:
      pulse = 1;
      break;
    case 1:
      pulse = 5;
      break;
    case 2:
      pulse = 9.5;
      break;
    case 3:
      pulse = 15;
      break;
    case 4:
      pulse = 21;
      break;
    default:
      pulse = 1;
      break;
    return pulse;
  }

}