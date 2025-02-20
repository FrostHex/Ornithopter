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
int slider_ratio = 0; // 电机转速信号
int joystick_x = 0; // 摇杆X轴信号
int joystick_y = 0; // 摇杆Y轴信号
byte data[3] = {0x00, 0x00, 0x00}; // 接收数据包


void setup() 
{
  Serial.begin(9600); // 初始化串口
  Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n==============================");

  radio.begin(); // 初始化radio对象
  radio.openReadingPipe(1, address); // 设置地址，0-5指定打开的管道
  radio.setPALevel(RF24_PA_LOW); // 设置功率放大器级别，RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_2MBPS); // 设置发送速率，RF24_250KBPS, RF24_1MBPS, RF24_2MBPS
  radio.startListening(); // 设置为接收端

  // servo_main.attach(servo_main_pin);
  // delay(1);
  // set_throttle(100); // 设置油门最大
  // Serial.println("Please connect the battery to the ESC in 5 seconds");
  // wait(5);
  // set_throttle(0); // 设置油门最小
  // wait(5);
  // Serial.println("ESC is ready");
}


void loop() 
{
  // 读取天线数据
  if (radio.available())
  {
    radio.read(&data, sizeof(data));
    slider_ratio = data[0];
    joystick_x = data[1];
    joystick_y = data[2];
  }
  // else
  // {
  //   Serial.println("Listening...");
  // }


  // 读取串口设置的油门比例
  // if (Serial.available())
  // {
  //   slider_ratio = Serial.parseInt();
  //   while (Serial.available() > 0) // 清空串口缓冲区
  //   {
  //     Serial.read();
  //   }
  // }

  // 设置油门和舵机
  set_throttle(slider_ratio);
  Serial.print("Slider ratio: ");
  Serial.println(slider_ratio);
  // Serial.print("Joystick X: ");
  // Serial.println(joystick_x);
  // Serial.print("Joystick Y: ");
  // Serial.println(joystick_y);

  delay(10); // 延时ms
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
 * @param ratio 油门比例, 0 ~ 255
 */
void set_throttle(int ratio)
{
  // Serial.print("Input throttle ratio: ");
  // Serial.println(ratio);
  ratio = constrain(ratio, 0, 255); // 限制油门比例在0到255之间
  servo_main.write((int)map(ratio, 0, 255, 8, 199)); // 将油门比例转换为对应的计数值，ratio的范围0到255，对应的数值范围8到199
  // Serial.print("Setting value: ");
  // Serial.println((int)map(ratio, 0, 255, 8, 199));
}