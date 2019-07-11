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

// Steering data -------------------------
int rawByteData = 0;
int rawByteData2 = 0;

// Global Estop Check
bool ESTOP_TRIGGERED;

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
    ESTOP_TRIGGERED = false;


    //CANBUS Setup
    //Can Mask
    CAN_filter_t canMask;
    canMask.id = 0xFFFFFF;
    canMask.rtr = 0;
    canMask.ext = 0;

    //Can Filter
    CAN_filter_t canFilter;

    //Begin CANBUS
    CANbus.begin();

//    canFilter.id = CANID_STEERING_RX;
//    CANbus.setFilter(canFilter, 0);
//    CANbus.setFilter(canFilter, 1);
//    CANbus.setFilter(canFilter, 2);
//    CANbus.setFilter(canFilter, 3);
//    CANbus.setFilter(canFilter, 4);
//    CANbus.setFilter(canFilter, 5);
//    CANbus.setFilter(canFilter, 6);
//    CANbus.setFilter(canFilter, 7);

//    txmsg_error_steering.id = CANID_STEERING_TX;


    

    

}

void loop() {
    
    if(CANbus.read(rxmsg)) {
//        Serial.println(rxmsg.id,HEX);   
//        Serial.println(rxmsg.buf[0]);    
        // check if the msg id belongs to steering_rx, check for the rc mode, check for estop press
        if(rxmsg.id == CANID_REMOTE_ESTOP )                        //---------------// autonomous enable bit is false
        {
          Serial.println("Dealing with estop");
          if(rxmsg.buf[0] == 0){
            //disable relay
            digitalWrite(led, LOW);
            digitalWrite(Relay, LOW);
            ESTOP_TRIGGERED = true;
        } else if (rxmsg.buf[0] != 0){
          ESTOP_TRIGGERED = false;
        }
        }
        else  if (ESTOP_TRIGGERED == false)
        {
                
        if(rxmsg.id == CANID_STEERING_RX && rxmsg.buf[0] == 1 && digitalRead(stop_button) == LOW) 
        {      
                digitalWrite(led, HIGH);
                //enable relay 
                digitalWrite(Relay, HIGH);
                
                //get data from canbus
                rawByteData = rxmsg.buf[1];
                rawByteData -= 128;

                //send data to motor controller using UART
                if(rawByteData > -2 && rawByteData < 2)      // straight
                {
                    Serial2.write(170);
                    Serial2.write(224);
                    Serial2.write(1);
                }
                else if(rawByteData <= -2)                     // left
                {
                    // turning anticlockwise (set target low resolution reverse)
                    Serial2.write(170);
                    Serial2.write(224);
                    Serial2.write(abs(rawByteData));
                }
                else if(rawByteData >= 2)                     // right
                {
                    // turning clockwise    (set target low resolution forward)
                    Serial2.write(170);
                    Serial2.write(225);
                    Serial2.write(abs(rawByteData));
                }
            }
        if(rxmsg.id == CANID_BRAKING_RX && rxmsg.buf[0] == 1 && digitalRead(stop_button) == LOW) 
        {   
          Serial1.write(rawByteData2 );      
                digitalWrite(led, HIGH);
                //enable relay 
                digitalWrite(Relay, HIGH);
                
                //get data from canbus
                rawByteData2 = rxmsg.buf[1];
                rawByteData2 -= 128;

                //send data to motor controller using UART
                if(rawByteData2 > -2 && rawByteData2 < 2)      // straight
                {
                    Serial1.write(170);
                    Serial1.write(224);
                    Serial1.write(1);
                }
                else if(rawByteData2 <= -2)                     // left
                {
                    // turning anticlockwise (set target low resolution reverse)
                    Serial1.write(170);
                    Serial1.write(224);
                    Serial1.write(abs(rawByteData2));
                }
                else if(rawByteData2 >= 2)                     // right
                {
                    // turning clockwise    (set target low resolution forward)
                    Serial1.write(170);
                    Serial1.write(225);
                    Serial1.write(abs(rawByteData2));
                }
            }
           
        if(digitalRead(stop_button) == HIGH || (rxmsg.buf[0] != 1 && rxmsg.id == CANID_STEERING_RX))                        //---------------// autonomous enable bit is false
        {
            //disable relay
            digitalWrite(led, LOW);
            digitalWrite(Relay, LOW);
        }
        
        }

   }

       

         if(digitalRead(stop_button) == HIGH)                        //---------------// autonomous enable bit is false
        {
            //disable relay
            digitalWrite(led, LOW);
            digitalWrite(Relay, LOW);
        }
        
    }
