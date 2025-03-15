// 上位机

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

RF24 radio(7,8); // 创建RF24对象，CE对应7号引脚, CSN对应8号引脚
const byte address[6] = "00006"; // 创建地址，用于识别发送端和接收端
LiquidCrystal_I2C lcd(0x27, 16, 2); // 初始化 LCD，地址 0x27，16 列 2 行

#define PIN_SLIDER A0 // 滑杆
#define PIN_POTENTIOMETER A1 // 电位器
#define PIN_JOYSTICK_X A2 // 摇杆X轴
#define PIN_JOYSTICK_Y A3 // 摇杆Y轴
#define PIN_JOYSTICK_SW 4 // 摇杆按键
#define ANALOG_MAX 1023 // 模拟信号最大值
#define ANALOG_MIN 0 // 模拟信号最小值
#define THROTTLE_LEVELS 191 // 油门级别总数
#define THROTTLE_LEVEL_MIN 8 // 油门最小值
int slider_val = 0; // 滑杆信号
int joystick_x = 0; // 摇杆X轴信号
int joystick_y = 0; // 摇杆Y轴信号
int joystick_btn = 0; // 摇杆按钮信号, 0: 未按下, 1: 按下
int potentiometer_val = 0; // 电位器信号
uint32_t packet = 0; // 发送数据包

// 数据包地址由小到大:
// 0-9: 摇杆X轴信号 (10位)
// 10-19: 摇杆Y轴信号 (10位)
// 20: 摇杆按钮信号 (1位)
// 21-28: 滑杆信号 (8位)
// 29-31: 电位器信号 (3位)


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
  radio.setRetries(0, 0);  // 设置重发间隔(ms)和最大重发次数

  // 初始化摇杆按钮
  pinMode(PIN_JOYSTICK_SW, INPUT_PULLUP); // 启用内部上拉电阻

  // 初始化显示屏
  Wire.begin(); // 初始化I2C，不指定引脚，使用默认的 A4 (SDA) 和 A5 (SCL)
  lcd.init();    // 初始化 LCD
  lcd.backlight();  // 开启背光

  Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n==============================");
}


void loop() 
{
  // 读取数据
  slider_val = constrain(analogRead(PIN_SLIDER), ANALOG_MIN, ANALOG_MAX);
  slider_val = map(slider_val, ANALOG_MIN, ANALOG_MAX, THROTTLE_LEVEL_MIN, THROTTLE_LEVEL_MIN + THROTTLE_LEVELS);
  potentiometer_val = constrain(analogRead(PIN_POTENTIOMETER), ANALOG_MIN, ANALOG_MAX);
  potentiometer_val = map(potentiometer_val, ANALOG_MIN, ANALOG_MAX, 0, 7);
  joystick_x = constrain(analogRead(PIN_JOYSTICK_X), ANALOG_MIN, ANALOG_MAX);
  joystick_y = constrain(analogRead(PIN_JOYSTICK_Y), ANALOG_MIN, ANALOG_MAX);
  joystick_btn = !digitalRead(PIN_JOYSTICK_SW);
  packet = ((uint32_t)(joystick_x & 0x3FF))
          | ((uint32_t)(joystick_y & 0x3FF) << 10)
          | ((uint32_t)(joystick_btn & 0x1) << 20)
          | ((uint32_t)(slider_val & 0xFF) << 21);
          // | ((uint32_t)(potentiometer_val & 0x7) << 29); // 暂时使用电位器信号

  // 发送数据包
  radio.write(&packet, sizeof(packet));
  Serial.print("Sending packet... ");

  // 打印调试信息
  Serial.print(" joystick_x: ");
  Serial.print(packet & 0x3FF);
  Serial.print(" joystick_y: ");
  Serial.print(packet >> 10 & 0x3FF);
  Serial.print(" button: ");
  Serial.print(packet >> 20 & 0x1);
  Serial.print(" slider: ");
  Serial.println(packet >> 21 & 0xFF);
  // Serial.print(" potentiometer: ");
  // Serial.println(packet >> 29 & 0x7);

  // 设置显示屏
  lcd.clear(); // 清空显示屏
  lcd.setCursor(0, 0);  // 设置光标位置
  lcd.print("^");
  lcd.print(slider_val - 8);
  lcd.setCursor(6, 0);
  lcd.print("b");
  lcd.print(joystick_btn);
  lcd.setCursor(10, 0);
  lcd.print("x");
  lcd.print(joystick_x - 527);
  lcd.setCursor(6, 1);
  lcd.print("p");
  lcd.print(potentiometer_val + 1);
  lcd.setCursor(10, 1);
  lcd.print("y");
  lcd.print(488 - joystick_y);

  // lcd.setCursor(2, 0);  // 设置光标位置
  // lcd.print("Hello World!");  // 显示文字
  // lcd.setCursor(6, 1);  // 设置光标位置
  // lcd.print("OwO");  // 显示文字

  delay(10); // 延时ms
}