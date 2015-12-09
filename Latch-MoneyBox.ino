/*
 * MoneyBox - FingerPrintScanner - Latch
 * 
 * Copyright (C) 2015 Eleven Paths
 * 
 * This software is free; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
*/

#ident "HOST:172.26.0.137" // Utilizado para realizar el upload del sketch por SSH.
#include <climits>
#include <aJSON.h>   // https://github.com/interactive-matter/aJson en .cpp eliminar #include <pgmspace.h>
#include <Adafruit_Fingerprint.h> // Para utilizar un virutal Serial Port en .h y.ccp eliminar #include <SoftwareSerial.h> y <util/delay.h>, cambiar el tipo SoftwareSerial por tipo Stream, eliminar begin() y delay() en el constructor
#include "mb_json.h" // MoneyBox Json Library

extern "C" {
  #include "latch.h"  // https://github.com/ElevenPaths/latch-sdk-c
}

// Virtual Serial Port interactive Input-Output
RingBuffer    rx_buffer_V1;
TTYUARTClass  SerialV1(&rx_buffer_V1, 3, false); 

// Virtual Serial Port moneybox 11 paths
RingBuffer    rx_buffer_V2;
TTYUARTClass  SerialV2(&rx_buffer_V2, 3, false); 

// Virtual Serial Port FingerPrintScanner
RingBuffer    rx_buffer_V3;
TTYUARTClass  SerialV3(&rx_buffer_V3, 3, false); 

aJsonObject* jsonConfig  = NULL;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&SerialV3);

// Sustituir por AppId Y Secret personal
char* latchAppId  = "hyZ-------------ceQu";
char* latchSecret = "kBPf---------------------------8PZhJ68";

// función principal invocada constantemente desde loop()
// Escanea una huella y si es está almacenada (enroll) llama a id_process() con el id
int getFingerprintIDez() {

   uint8_t p = finger.getImage();

   if (p != FINGERPRINT_OK)  return -1;

   p = finger.image2Tz();
   if (p != FINGERPRINT_OK)  return -1;

   p = finger.fingerFastSearch();
   if (p != FINGERPRINT_OK){
     SerialV1.println("No found! FastSearch");
     SerialV2.write("5");
     delay(1000);
     return -1;
   }
   // found a match!
   SerialV1.print("Found ID #"); SerialV1.print(finger.fingerID);
   SerialV1.print(" with confidence of "); SerialV1.println(finger.confidence);
   
   id_process(jsonConfig,finger.fingerID);
   delay(1000);
   return finger.fingerID;
}

// Devuelve true/false del estado de los latch del ID de indice index
boolean latch_status(aJsonObject* jsonConfig, int index){
  
  int num_latches = -1;
  int j=0;  
  boolean latch_result=false;

  char b_latchid[512];
  char b_status_result[512];

  char* latchid = b_latchid; 
  char* status_result=b_status_result; 

  num_latches=num_latchids(jsonConfig,index);
  
  if (num_latches>0){
      SerialV1.print("Numero de Latch Accounts ");
      SerialV1.println(num_latches);
      while ((j<num_latches) and (latch_result==false)){
        
        SerialV1.print("Comprobando el estado de ");
       
        memset(b_latchid, 0, sizeof(char)*512);
        latchid=get_latchid(jsonConfig,index,j);    
        SerialV1.println(latchid);

        memset(b_status_result, 0, sizeof(char)*512);
        status_result=status(latchid);

    if (status_result!=NULL){
        if (strstr(status_result, "\"status\":\"on\"") != NULL){
          SerialV1.println("Estado del Latch on");
           if (latch_and(jsonConfig,index)==false){
            SerialV1.println("No es necesrio comprobar otros laches (OR con alguno en on)");
            latch_result=true;
          }else{ 
            if ((j+1)==num_latches){
                latch_result=true;
                SerialV1.println("No es necesrio comprobar otros laches (AND todos on)");
            }else{}
          }
        }else{
          SerialV1.println("Estado del Latch off");
          if (latch_and(jsonConfig,index)){
            SerialV1.println("No es necesrio comprobar otros laches (AND con alguno en off)");
            j=INT_MAX;
          }      
        }
    }else{
      SerialV1.println("Network error o timeout, se asume off");
      if (latch_and(jsonConfig,index)){
         SerialV1.println("No es necesrio comprobar otros laches (AND con alguno en off)");
         j=INT_MAX;
       }  
    }
        // Next latch account
        j++;
      }
    }else{
      SerialV1.print("Error en config latch ");
      SerialV1.println(num_latches);
    }
  return latch_result;
}

// busca el index en el archivo de configuración json que corresponde a la huella id
// si id tiene correspondencia con un index (está enrolado), comprueba el estado de los latches
// si están en estado "on" (true), llama a get_money() para la dispensación configurada
void id_process(aJsonObject* jsonConfig, int id){
   
   int index =-1;
   index = id_index(jsonConfig,id);
   
   if (index>=0){
     SerialV1.print("ID ok con index: ");SerialV1.println(index);
     if(latch_status(jsonConfig,index)==true){
        SerialV1.print("Money: ");
        SerialV1.println(get_money(jsonConfig,index));
        
        SerialV2.write(get_money(jsonConfig,index));
        SerialV2.write("5");
     }else{
      SerialV1.println("Latch off");
      SerialV2.write("5");SerialV2.write("5");
     }
   }else{
    SerialV1.print("ID no registrada. Error ");
    SerialV1.println(index);   
    SerialV2.write("5");SerialV2.write("5");SerialV2.write("5");
   }
}

// the setup function runs once when you press reset or power the board
void setup() {

// Virtual Serial Port interactive Input-Output   // Para crear el virtual null-modem pasado en pseudo terminales virtuales:
   SerialV1.init_tty("/dev/tty.arduino0");        // socat PTY,link=/dev/tty.arduino0,raw,echo=0 PTY,link=/dev/tty.screen0,raw,echo=0 &
   SerialV1.begin(9600);                          // Para conectarse: screen /dev/tty.screen0 9600

// Virtual Serial Port moneybox 11 paths
   SerialV2.init_tty("/dev/ttyACM0");             // USB to Serial Device (MoneyBox 11Paths)
   SerialV2.begin(9600);
   
// Virtual Serial Port FingerPrintScanner
   SerialV3.init_tty("/dev/ttyUSB0");             // USB to Serial Device (Adafruit FingerPrint Scanner)
   SerialV3.begin(57600);

   SerialV1.println("\r\nInit!");

   SerialV1.print("Waiting for MoneyBox... ");
   SerialV2.write("9");
   delay(100);
   SerialV1.write(SerialV2.read());
   while (SerialV2.available()){
      SerialV1.write(SerialV2.read());
    }
   SerialV1.println(" OK!");

   SerialV1.print("Checking json file");
   jsonConfig = read_json_file(FILE_JSON);
   
   while (jsonConfig==NULL){
      SerialV1.println(" ERROR json file!");
      delay(10000);
      jsonConfig = read_json_file(FILE_JSON);
   }
   
   SerialV1.println(" read json file ok!");
   SerialV1.print("Numero de IDs en el json:"); SerialV1.println(num_ids(jsonConfig));

   SerialV1.println("Waiting for FingerPrint Scanner..");
   while (finger.verifyPassword()==false){
      SerialV1.println("Did not find fingerprint sensor :(");
      delay(10000);
   }
   SerialV1.println("Found FingerPrint Scanner!");

   // latch init
   SerialV1.println("Latch init..");
   setNoSignal(1);
   init(latchAppId,latchSecret); 

   SerialV1.println("Run!!");
   system("echo $(date '+%Y %b %d %H:%M:%S') Sketch setup done! $(hostname) >> /tmp/arduino.log");
}
// the loop function runs over and over again forever
void loop() {
   getFingerprintIDez();
   delay(100);          
}
