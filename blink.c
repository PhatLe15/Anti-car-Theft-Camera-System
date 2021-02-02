#include <wiringPi.h>

int main(void){
  wiringPiSetupGpio();
  //wiringPiSetupSys();
  
  pinMode(17,OUTPUT);

  while(1){
    digitalWrite(17,HIGH);
    delay(1.5);
    digitalWrite(17,LOW);
    delay(1.5);
  }
  return 0;
}
