// 上位机

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// 创建RF24对象，CE对应7号引脚, CSN对应8号引脚
RF24 radio(7,8);

// 创建一个地址，用于识别发送端和接收端
const byte address[6] = "00006";

void setup() 
{
  // 初始化串口
  Serial.begin(9600);
  // 初始化radio对象
  radio.begin();

  // pinMode(10, OUTPUT);
  // 设置地址
  radio.openWritingPipe(address);
  // 设置功率放大器级别
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  
  // 设置发送速率
  radio.setDataRate(RF24_250KBPS); // RF24_250KBPS, RF24_1MBPS, RF24_2MBPS
  
  // 设置为发送端
  radio.stopListening();
}

void loop() 
{
  // 发送数据，最多32字节
  const char text[] = "Hello World";
  Serial.println("Sending...");
  Serial.println(radio.write(&text, sizeof(text)));
  delay(1000);
}
