#include <Arduino.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9, 53); // "создать" модуль на пинах 9 и 10 Для Уно
//RF24 radio(9,53); // для Меги

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

int joystickX_current;
int joystickX_previous;

int joystickY_current;
int joystickY_previous;

byte transmit_data[2]; // массив, хранящий передаваемые данные
byte latest_data[2];   // массив, хранящий последние переданные данные
bool flag;             // флажок отправки данных

void dataRead()
{
    // считывание показания джойстика с оси ОХ
    joystickX_current = analogRead(A0);
    // смотрим отклоняемся ли мы от центра
    if ((joystickX_current < 500 || joystickX_current > 512))
    {
        // изменилось ли показание
        if (joystickX_current != joystickX_previous)
        {
            // предыдущие показание равняется новому
            joystickX_previous = joystickX_current;
            flag = 1;
        }
    }
    // смотрим находимся ли мы в центре
    else if (joystickX_previous != 505)
    {
        joystickX_previous = 505;
        flag = 1;
    }

    // считывание показания джойстика с оси ОY
    joystickY_current = analogRead(A1);
    // смотрим не находимся ли мы в центре
    if ((joystickY_current < 500 || joystickY_current > 510))
    {
        // изменилось ли показание
        if (joystickY_current != joystickY_previous)
        {
            flag = 1; // предыдущие показание равняется новому
            joystickY_previous = joystickY_current;
        }
    }

    // смотрим находимся ли мы в центре
    else if (joystickY_previous != 505)
    {
        joystickY_previous = 505;
        flag = 1;
    }
}

void setup()
{
    Serial.begin(9600); //открываем порт для связи с ПК

    radio.begin();            //активировать модуль
    radio.setAutoAck(1);      //режим подтверждения приёма, 1 вкл 0 выкл
    radio.setRetries(0, 15);  //(время между попыткой достучаться, число попыток)
    radio.enableAckPayload(); //разрешить отсылку данных в ответ на входящий сигнал
    radio.setPayloadSize(32); //размер пакета, в байтах

    radio.openWritingPipe(address[0]); //мы - труба 0, открываем канал для передачи данных
    radio.setChannel(0x66);            //выбираем канал (в котором нет шумов!)

    radio.setPALevel(RF24_PA_MAX);   //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
    radio.setDataRate(RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
    //должна быть одинакова на приёмнике и передатчике!
    //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

    radio.powerUp();       //начать работу
    radio.stopListening(); //не слушаем радиоэфир, мы передатчик
}

void loop()
{

    dataRead();
    transmit_data[0] = map(joystickY_previous,0,1023,0,255); // инвертированный (!) сигнал с кнопки
    transmit_data[1] = map(joystickX_previous,0,1023,0,255);; // получить значение
    //  transmit_data[2] = map(analogRead(slider), 0, 1023, 0, 255);

    if (flag)
    {
        Serial.print("X  ");
        Serial.print(transmit_data[0]);
        Serial.print("  Y  ");
        Serial.println(transmit_data[1] );
        flag = 0;
        radio.powerUp();                                    // включить передатчик
        radio.write(&transmit_data, sizeof(transmit_data)); // отправить по радио
        Serial.println("transmit");
        flag = 0;          //опустить флаг
        radio.powerDown(); // выключить передатчик
    }
}
