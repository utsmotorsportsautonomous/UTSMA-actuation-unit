/*
   UTSME Autonomous 2019 Actuation Unit
   by Hein Wai Leong and Tim Yan Muk

   CanBus Library Fork from pawelsky / FlexCAN_Library
   https://github.com/pawelsky/FlexCAN_Library
   Arduino library for CAN on Teensy 3.1, 3.2, 3.5 and 3.6
   By Pawelsky

*/

#include "FlexCAN.h"
#include "JrkG2.h"
#include <TimerOne.h>

#define UART_SERIAL Serial1;


// Canbus setup --------------------------
FlexCAN CANbus(1000000);
const int CANID_STEERING_RX = 0x33;
const int CANID_STEERING_TX = 0x34;
const int CANID_BRAKING_RX = 0x35;
const int CANID_BRAKING_TX = 0x36;
const int CANID_REMOTE_ESTOP = 0x191;


// Pin Setup -----------------------------
int led = 13;
const int Relay = 14;
int stop_button = 15;
int buzzer = 6;

// Steering data -------------------------
int steeringRawByteData = 0;

// Braking data -------------------------  
int brakingRawByteData = 0;

// Flags -------------------------
bool CAN_AVAILAIBLE = false;
bool REMOTE_ESTOP_TRIGGERED = false;
bool AUTONOMOUS_MODE = false;


static CAN_message_t txmsg_error_steering, rxmsg;

#include "FlexCAN.h"

#ifndef __MK20DX256__
#error "Teensy 3.2 is required to run"
#endif

void setup() {
    delay(2000);
    Serial.begin(9600);
    Serial1.begin(9600);
    Serial2.begin(9600);
    
    Serial.println(F("Teensy 3.2 - actuation unit "));

    //initialize pins
    pinMode(Relay, OUTPUT);
    pinMode(led, OUTPUT);
    pinMode(stop_button, INPUT_PULLUP);
    pinMode(buzzer, OUTPUT);
   


    //CANBUS Setup -------------------------
    //Can Mask
    CAN_filter_t canMask;
    canMask.id = 0xFFFFFF;
    canMask.rtr = 0;
    canMask.ext = 0;

    //Can Filter -------------------------
    CAN_filter_t canFilter;

    //Begin CANBUS -------------------------
    CANbus.begin();

     Timer1.initialize(10); 
     //Timer1.attachInterrupt(alarm_detect);
     Timer1.attachInterrupt(can_update);
}

//Alarms  -------------------------
void autonomousModeAlarm(){    
    digitalWrite(buzzer, HIGH);
    delay(100);
    digitalWrite(buzzer, LOW);
    delay(1000);  
}

void estopTriggeredAlarm(){
    digitalWrite(buzzer, HIGH);
}
void estopReset(){
  digitalWrite(buzzer, LOW);
}


//Interrrupt alarms  -------------------------
void alarm_detect(){
  if(AUTONOMOUS_MODE==true){
    autonomousModeAlarm();
  }
  else if(REMOTE_ESTOP_TRIGGERED==true){
    estopTriggeredAlarm();
  }
  else{
    estopReset();    
  }
}

//Interrrupt can  -------------------------
void can_update(){
      
    if(CANbus.read(rxmsg)) {
      CAN_AVAILAIBLE = true;
      
      if(rxmsg.id == CANID_REMOTE_ESTOP ){

//        Serial.print(rxmsg.buf[0]);
        
       if(rxmsg.buf[0] == 0){
          REMOTE_ESTOP_TRIGGERED = true;
        }
       else{ 
         REMOTE_ESTOP_TRIGGERED = false;
       }
        
      }
      if(rxmsg.id == CANID_STEERING_RX){
         steeringRawByteData = rxmsg.buf[1];
         steeringRawByteData -= 128;

      }
      if(rxmsg.id == CANID_BRAKING_RX && rxmsg.buf[0] == 1){
        brakingRawByteData = rxmsg.buf[1];
        brakingRawByteData -= 128;

      }      
  }
  else {
    
    CAN_AVAILAIBLE = false;
  
  }
}


void loop() {

//can_update();

  Serial.print("steeringRawByteData");
  Serial.print(" ");
  Serial.println(steeringRawByteData);
//  Serial.print("brakingRawByteData");
//  Serial.print(" ");
//  Serial.println(brakingRawByteData);
//  Serial.print("ESTOP_TRIGGERED");
//  Serial.print(" ");
//  Serial.println(REMOTE_ESTOP_TRIGGERED);


  if(REMOTE_ESTOP_TRIGGERED==false && CAN_AVAILAIBLE==true){// straight
       digitalWrite(led, HIGH);//enable LED       
       digitalWrite(Relay, HIGH);//enable relay 
       if(steeringRawByteData > -2 && steeringRawByteData < 2){
            Serial2.write(170);
            Serial2.write(224);
            Serial2.write(1);
       }
       else if(steeringRawByteData <= -2){// left

            // turning anticlockwise (set target low resolution reverse)
            Serial2.write(170);
            Serial2.write(224);
            Serial2.write(abs(steeringRawByteData));
        }
        else if(steeringRawByteData >= 2){// right        
            // turning clockwise    (set target low resolution forward)
            Serial2.write(170);
            Serial2.write(225);
            Serial2.write(abs(steeringRawByteData));
        }

        if(brakingRawByteData > -2 && brakingRawByteData < 2){
            Serial1.write(170);
            Serial1.write(224);
            Serial1.write(1);
         }
         
        else if(brakingRawByteData <= -2){
            // turning anticlockwise (set target low resolution reverse)
            Serial1.write(170);
            Serial1.write(224);
            Serial1.write(abs(brakingRawByteData));
        }
        else if(brakingRawByteData >= 2){
            // turning clockwise    (set target low resolution forward)
            Serial1.write(170);
            Serial1.write(225);
            Serial1.write(abs(brakingRawByteData));
        }
  }
  else if(REMOTE_ESTOP_TRIGGERED==true){

    
    Serial2.write(170);
    Serial2.write(224);
    Serial2.write(1);
    
    delay(500);
    
    digitalWrite(led, LOW);//disable LED       
    digitalWrite(Relay, LOW);//disable relay 



    
    
  }

  }
    
