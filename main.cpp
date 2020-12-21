#include <Arduino.h>

long current_speed = 0;
const char interruptPin = 3;
const long rpm_vactual = 1236;
unsigned char rotation_state = 0;
unsigned char toggle_rotation_intent = 0;
unsigned long last_interrupt = 0;

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

unsigned char calculate_crc(unsigned char data[], int len){
    unsigned char crc = 0;
    for(int i=0; i<len; i++){
        crc = crc ^ data[i];
        for(int j =0; j<8; j++){
            if(crc & 0b00000001){
                crc = (crc >> 1) ^ 0b11100000;
            }else{
                crc = (crc >> 1);
            }
        }
    }
    crc = reverse(crc);
    return crc;
}

void tmc_write(unsigned char target_register, long data){
  target_register = target_register | 0b10000000;
  unsigned char buf[8] = {0x05, 0x00, target_register, 0x00, 0x00, 0x00, 0x00, 0x00};
  buf[6] = data;
  data = data >> 8;
  buf[5] = data;
  data = data >> 8;
  buf[4] = data;
  data = data >> 8;
  buf[3] = data;

  buf[7] = calculate_crc(buf, 7);

  Serial.write(buf, 8);
}

void set_speed(long speed){
  current_speed = speed;
  tmc_write(0x22, speed);
}

void ramp_speed(long target_speed){
  int subdivisions = 100;
  int time_period_ms = 2000;
  long step = (target_speed - current_speed) / (long) subdivisions;

  if(step>=0){
    for(long speed = current_speed + step; speed < target_speed; speed += step){
      set_speed(speed);
      delay(time_period_ms/subdivisions);
    }
  }else{
    for(long speed = current_speed + step; speed > target_speed; speed += step){
      set_speed(speed);
      delay(time_period_ms/subdivisions);
    }
  }

  set_speed(target_speed);
}

void set_configuration(){
  tmc_write(0x10, 0x00011F00); //set IHOLD_IRUN
  tmc_write(0x00, 0x000001E1); //set GCONF enable MSTEP
  tmc_write(0x70, 0xC81D0E24); //set PWMCONF for freewheeling
  tmc_write(0x6C, 0x15000053); //set microstep 8
}

void toggle_rotation(){
  rotation_state = !rotation_state;
  if(rotation_state){
    ramp_speed(rpm_vactual);
  }else{
    ramp_speed(0);
  }
}

void interrupt_handler(){
  if(millis()>last_interrupt+ 1000){
    toggle_rotation_intent = 1;
  }
  last_interrupt = millis();
}

void setup() {
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), interrupt_handler, FALLING);
  Serial.begin(112500);
  delay(100);
  set_configuration();
}

void loop() {
  if(toggle_rotation_intent){
    toggle_rotation_intent=0;
    toggle_rotation();
  }
}