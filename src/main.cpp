#include <Arduino.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно
// RF24 radio(9,53); // для Меги
// массив для данных которые принимаю
byte recieved_data[2];

// пины для мотора
int motorIn1 = 4; // направление 1  0  0  1
int motorIn2 = 6; // направление 0  1  0  1
int motorEn = 5;  // скорость  0-255
int motorSpeed;

#include <Servo.h>
Servo myservo;
int ServoPos;

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

void setup()
{
  myservo.attach(3);

  pinMode(motorEn, OUTPUT); //
  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);

  Serial.begin(9600);       //открываем порт для связи с ПК
  radio.begin();            //активировать модуль
  radio.setAutoAck(1);      //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);  //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload(); //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32); //размер пакета, в байтах

  radio.openReadingPipe(1, address[0]); //хотим слушать трубу 0
  radio.setChannel(0x66);               //выбираем канал (в котором нет шумов!)

  radio.setPALevel(RF24_PA_MAX);   //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp();        //начать работу
  radio.startListening(); //начинаем слушать эфир, мы приёмный модуль
}

void loop()
{
  byte pipeNo;
  while (radio.available(&pipeNo))
  {                                                    // слушаем эфир со всех труб
    radio.read(&recieved_data, sizeof(recieved_data)); // чиатем входящий сигнал
   // Serial.print(recieved_data[0]);                    // подать на реле сигнал с 0 места массива
    //Serial.print("  ");
    //Serial.println(recieved_data[1]); //то бишь 0 или 1

    if (recieved_data[1] == 125)
    {
      motorSpeed = 0;
      digitalWrite(motorIn1, LOW);
      digitalWrite(motorIn2, LOW);
      // Serial.println(motorSpeed);
    }
    else if (recieved_data[1] > 125) // едем вперед
    {
      digitalWrite(motorIn1, HIGH);
      digitalWrite(motorIn2, LOW);
      motorSpeed = map(recieved_data[1], 125, 255, 0, 255);
      // Serial.println(motorSpeed);
      Serial.print("backward  ");
      Serial.println(motorSpeed);
    }
    else // едем назад
    {
      motorSpeed = map(recieved_data[1], 255, 125, 0, 255)-255;
      digitalWrite(motorIn1, LOW);
      digitalWrite(motorIn2, HIGH);
      // Serial.println(motorSpeed);
      Serial.print("forward  ");
      Serial.println(motorSpeed);
    }
    
    analogWrite(motorEn, motorSpeed);
    ServoPos = map(recieved_data[0], 0, 255, 0, 150); // положение серводвигателя
    myservo.write(recieved_data[0]);
  }
}
