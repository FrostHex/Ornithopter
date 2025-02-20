// 上位机

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7,8); // 创建RF24对象，CE对应7号引脚, CSN对应8号引脚
const byte address[6] = "00006"; // 创建地址，用于识别发送端和接收端

const int slider_pin = A0; // 滑杆对应的引脚
const int slider_max = 653; // 滑杆信号最大值
const int slider_min = 0; // 滑杆信号最小值
int slider_val = 0; // 滑杆信号
int slider_ratio = 0; // 滑杆信号比例, 0-255
int joystick_x = 0; // 摇杆X轴信号
int joystick_y = 0; // 摇杆Y轴信号
byte data[3] = {0x00, 0x00, 0x00}; // 发送数据包

void setup() 
{
  Serial.begin(9600); // 初始化串口
  Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n==============================");

  radio.begin(); // 初始化radio对象
  radio.openWritingPipe(address); // 设置地址
  radio.stopListening(); // 设置为发送端
  radio.setPALevel(RF24_PA_LOW); // 设置功率放大器级别, RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_2MBPS); // 设置发送速率, RF24_250KBPS, RF24_1MBPS, RF24_2MBPS
  // radio.setAutoAck(false);  // 禁用自动重发
  radio.setRetries(3, 10);  // 设置最大重发次数和重发间隔(ms)
}

void loop() 
{
  // 发送数据，最多32字节
  // const char text[] = "Hello World!";
  // Serial.println("Sending...");
  // Serial.println(radio.write(&text, sizeof(text)));
  // delay(1000);

  // 读取滑杆数据
  slider_val = constrain(analogRead(slider_pin), slider_min, slider_max);
  slider_ratio = (int) map(slider_val, slider_min, slider_max, 0, 255);
  // Serial.println(slider_ratio);
  data[0] = slider_ratio;

  // 发送数据包
  Serial.print("Sending: ");
  for (int i = 0; i < sizeof(data); i++)
  {
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print("\nResult: ");
  Serial.println(radio.write(&data, sizeof(data)));

  delay(10); // 延时ms
}
