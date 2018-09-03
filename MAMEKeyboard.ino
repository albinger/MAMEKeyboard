/*
** Author: Andrew Albinger
**
** Intended for use with https://www.thingiverse.com/thing:3082120
** to remotely control a MAME installation.
**
** Based on the HIDKeyboard example from Adafruit.
** For use with the Adafruit Feather 32u4 Bluefruit LE.
** I recommend putting the HIDKeyboard sketch on your board
** to begin as it does a factory reset and all the tweaks.
** Very little error checking in the board setup here.
** It takes several seconds for HID Keyboard functionality
** after boot up and bluetooth connectivity.
**
** I take no responsibility for your implementation!
*/


#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

const int numbutts = 10;  //heh, he said butts...

int buttons[10] = {2,3,5,13,18,19,23,6,9,10};

char *keys[20]= {  // build an array with unshifted and shifted hex values
  "-1E",//1  1 (player 1 start)
  "-22",//2  5 (coin1)
  "-13",//3  p (pause)
  "-1D",//4  z (button6)
  "-1B",//5  x (button5)
  "-2C",//6  Space (button3)
  "-51",//7  Down
  "-52",//8  Up
  "-50",//9  Left
  "-4F", //10 Right
  "-1F",//11  1->2 (player 2 start)
  "-23",//12  5->6 (coin2)
  "-29",//13  p->escape (end game)
  "-2B",//14  z->Tab (in game menu)
  "-28",//15  x->Enter 
  "-2C",//16  Space
  "-51",//17  Down
  "-52",//18  Up
  "-50",//19  Left
  "-4F" //20 Right
  };

int shft, ctl, alt = 0;
int idle=1;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// Create the bluefruit object using SPI
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


void setup(void)
{
//initialize the bluefruit le    
  ble.begin(VERBOSE_MODE);
  //ble.factoryReset();
  ble.echo(false);

  //ble.info();
  ble.sendCommandCheckOK(F( "AT+GAPDEVNAME=MAME Keyboard" ));
  ble.sendCommandCheckOK(F( "AT+BleHIDEn=On" ));
  ble.reset(); // required if we change state like the previous line

// initialize the pushbutton pins as input:
  for(int i=0;i<numbutts;i++){
    pinMode(buttons[i], INPUT_PULLUP);     
  }
  pinMode(22,INPUT_PULLUP);
  pinMode(21,INPUT_PULLUP);
  pinMode(20,INPUT_PULLUP);
  pinMode(12,INPUT_PULLUP);
}


void loop(void)
{
  int pressed=0;
  int state=0;
  int i = 0;
  int mod, shft = 0;
  char thestring[6];

  shft=digitalRead(12) ? 0:10; // our shift key, not sent to bluetooth
  mod+=digitalRead(22) ? 0:1;  //left control (button1)
  mod+=digitalRead(20) ? 0:2;  //left shift   (button4)
  mod+=digitalRead(21) ? 0:4;  //left alt     (button2)
  sprintf(thestring,"%02d-00",mod); //build our mod header

  for(i=0;i<numbutts;i++){
   
    state = digitalRead(buttons[i]) ? 0:1; // read all buttons one time
    {
       if ((state) && (millis() - lastDebounceTime > debounceDelay)) //this is kind of wrong I'm not checking the debounce state of each key
       {
            if(!pressed)
            {
              ble.print("AT+BLEKEYBOARDCODE=");
              ble.print(thestring);
              pressed++;
            }
            ble.print(keys[i+shft]); //find unshifted or shifted value in array
          lastDebounceTime = millis();
          
       }  
    }

 
  }
      
  if(pressed)
    {
      ble.println();         //send the codes, something was pressed
      idle = 0;              // we are not idle
    }else if(idle && mod)   //modifiers are pressed but no other keys generated output
    {
      ble.print("AT+BLEKEYBOARDCODE=");   
      ble.println(thestring);     //send the modifiers
      idle = 0;                   //we are not idle
      
    } else if(!idle && !pressed){  // not currently idle but nothing has been pressed
      ble.println("AT+BLEKEYBOARDCODE=00-00");    // send an all clear
      idle=1;                                     // we are idle idle
    }
  
  delay(100);    //slow down son!
}