// 下位机

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

RF24 radio(7,8); // 创建RF24对象，CE对应7号引脚, CSN对应8号引脚
const byte address[6] = "00006"; // 创建地址，用于识别发送端和接收端

Servo servo_main; // 主翼电机
const int servo_main_pin = 9; // 主翼电机对应的引脚
const int servo_main_max = 199; // 主翼电机转速信号最大值，对应占空比11.57%
const int servo_main_min = 8; // 主翼电机转速信号最小值，对应占空比3.15%
int servo_main_val = 0; // 电机转速信号


void setup() 
{
  Serial.begin(9600); // 初始化串口
  Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n==============================");

  radio.begin(); // 初始化radio对象
  radio.openReadingPipe(1, address); // 设置地址，0-5指定打开的管道
  radio.setPALevel(RF24_PA_LOW); // 设置功率放大器级别，RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_250KBPS); // 设置发送速率，RF24_250KBPS, RF24_1MBPS, RF24_2MBPS
  radio.startListening(); // 设置为接收端

  servo_main.attach(servo_main_pin);
  delay(1);
  set_throttle(100); // 设置油门最大
  Serial.println("Please connect the battery to the ESC in 5 seconds");
  wait(5);
  set_throttle(0); // 设置油门最小
  wait(5);
  Serial.println("ESC is ready");
}


void loop() 
{
  // 检天线查是否有数据可读
  char text[32] = "";
  radio.read(&text, sizeof(text));
  Serial.println(text);
  Serial.println("Listening...");
  delay(500); // 延时500ms

  // 读取串口数据并设置油门
  if (Serial.available())
  {
    int ratio = Serial.parseInt();
    set_throttle(ratio);
    while (Serial.available() > 0) 
    {
      Serial.read();
    }
  }
}


void wait(int s)
{
  for (int i = 0; i < s; i++)
  {
    Serial.print("Waiting: ");
    Serial.println(s - i);
    delay(1000); // 等待1s
  }
}


/*
 * @brief 设置油门
 * @param ratio 油门比例
 */
void set_throttle(int ratio)
{
  Serial.print("Input ratio: ");
  Serial.println(ratio);
  servo_main_val = (int)map(ratio, 0, 100, 8, 199); // 将油门比例转换为对应的计数值，ratio的范围0到100，对应的数值范围8到199
  servo_main.write(servo_main_val);
  // Serial.print("Setting value: ");
  // Serial.println(servo_main_val);
}