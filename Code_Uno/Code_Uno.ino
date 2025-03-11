// 上位机

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

RF24 radio(7,8); // 创建RF24对象，CE对应7号引脚, CSN对应8号引脚
const byte address[6] = "00006"; // 创建地址，用于识别发送端和接收端
LiquidCrystal_I2C lcd(0x27, 16, 2); // 初始化 LCD，地址 0x27，16 列 2 行

const int slider_pin = A0; // 滑杆对应的引脚
const int potentiometer_pin = A1; // 电位器对应的引脚
const int joystick_x_pin = A2; // 摇杆X轴对应的引脚
const int joystick_y_pin = A3; // 摇杆Y轴对应的引脚
const int joystick_btn_pin = 4; // 摇杆按钮对应的引脚
const int analog_max = 1023; // 模拟信号最大值
const int analog_min = 0; // 模拟信号最小值
int slider_val = 0; // 滑杆信号
int slider_ratio = 0; // 滑杆信号比例, 0-255
int joystick_x = 0; // 摇杆X轴信号
int joystick_y = 0; // 摇杆Y轴信号
int joystick_btn = 0; // 摇杆按钮信号
int potentiometer_val = 0; // 电位器信号
byte data[3] = {0x00, 0x00, 0x00}; // 发送数据包


void setup() 
{
  // 初始化串口
  Serial.begin(9600); 

  // 初始化天线
  radio.begin(); // 初始化radio对象
  radio.openWritingPipe(address); // 设置地址
  radio.stopListening(); // 设置为发送端
  radio.setPALevel(RF24_PA_LOW); // 设置功率放大器级别, RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_1MBPS); // 设置发送速率, RF24_250KBPS, RF24_1MBPS, RF24_2MBPS
  radio.setRetries(3, 10);  // 设置最大重发次数和重发间隔(ms)
  // radio.setAutoAck(false);  // 禁用自动重发

  // 初始化摇杆按钮
  pinMode(joystick_btn_pin, INPUT_PULLUP); // 启用内部上拉电阻

  // 初始化显示屏
  Wire.begin(); // 初始化I2C，不指定引脚，使用默认的 A4 (SDA) 和 A5 (SCL)
  lcd.init();    // 初始化 LCD
  lcd.backlight();  // 开启背光

  Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n==============================");
}


void loop() 
{
  // 读取数据
  slider_val = constrain(analogRead(slider_pin), analog_min, analog_max);
  slider_ratio = (int) map(slider_val, analog_min, analog_max, 0, 255);
  joystick_x = analogRead(joystick_x_pin);
  joystick_y = analogRead(joystick_y_pin);
  joystick_btn = digitalRead(joystick_btn_pin);
  potentiometer_val = analogRead(potentiometer_pin);
  data[0] = slider_ratio;
  data[1] = map(joystick_x, 0, 1023, 0, 255);
  data[2] = map(joystick_y, 0, 1023, 0, 255);

  // 发送数据包
  Serial.print("Sending: ");
  for (int i = 0; i < sizeof(data); i++)
  {
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print("\nResult: ");
  Serial.println(radio.write(&data, sizeof(data)));

  // 打印调试信息
  Serial.print("slider: ");
  Serial.print(data[0]);
  Serial.print(" joystick_x: ");
  Serial.print(data[1]);
  Serial.print(" joystick_y: ");
  Serial.print(data[2]);
  Serial.print(" button: ");
  Serial.print(joystick_btn);
  Serial.print(" potentiometer: ");
  Serial.println(potentiometer_val);

  // 设置显示屏
  lcd.setCursor(2, 0);  // 设置光标位置
  lcd.print("Hello World!");  // 显示文字
  lcd.setCursor(6, 1);  // 设置光标位置
  lcd.print("OwO");  // 显示文字
  // lcd.print("（っ╹◡╹)っ");  // 显示文字

  delay(100); // 延时ms
}