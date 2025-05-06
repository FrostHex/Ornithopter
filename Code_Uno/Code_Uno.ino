// 上位机

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

RF24 Radio(7,8); // 创建RF24对象，CE对应7号引脚, CSN对应8号引脚
const byte address[6] = "00006"; // 创建地址，用于识别发送端和接收端
LiquidCrystal_I2C lcd(0x27, 16, 2); // 初始化 LCD，地址 0x27，16 列 2 行

#define PIN_SLIDER A0 // 滑杆
#define PIN_POTENTIOMETER A1 // 电位器
#define PIN_JOYSTICK_X A2 // 摇杆X轴
#define PIN_JOYSTICK_Y A3 // 摇杆Y轴
#define PIN_JOYSTICK_B 4 // 摇杆按键
#define ANALOG_MAX 1023 // 模拟信号最大值
#define ANALOG_MIN 0 // 模拟信号最小值
#define THROTTLE_MAX 199 // 油门级别总数
#define THROTTLE_MIN 8 // 油门最小值
#define JOYSTICK_X_MID 527 // 摇杆X轴中位值
#define JOYSTICK_Y_MID 488 // 摇杆Y轴中位值
int Slider_Val = 0; // 滑杆信号
int Joystick_X = 0; // 摇杆X轴信号
int Joystick_Y = 0; // 摇杆Y轴信号
int Joystick_B = 0; // 摇杆按钮信号, 0: 未按下, 1: 按下
int Angle_X = 90; // 俯仰舵机位置
int Angle_Y = 90; // 滚转舵机位置
int Potentiometer_Val = 0; // 电位器信号
uint32_t Packet = 0; // 发送数据包

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
  Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n==============================");

  // 初始化天线
  Radio.begin(); // 初始化Radio对象
  Radio.openWritingPipe(address); // 设置地址
  Radio.stopListening(); // 设置为发送端
  Radio.setPALevel(RF24_PA_HIGH); // 设置功率放大器级别, RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  Radio.setDataRate(RF24_1MBPS); // 设置发送速率, RF24_250KBPS, RF24_1MBPS, RF24_2MBPS
  Radio.setRetries(0, 0);  // 设置重发间隔(ms)和最大重发次数

  // 初始化摇杆按钮
  pinMode(PIN_JOYSTICK_B, INPUT_PULLUP); // 启用内部上拉电阻

  // 初始化显示屏
  Wire.begin(); // 初始化I2C，不指定引脚，使用默认的 A4 (SDA) 和 A5 (SCL)
  lcd.init();    // 初始化 LCD
  lcd.backlight();  // 开启背光
}


void loop() 
{
  GetValue(); // 读取数据
  Radio.write(&Packet, sizeof(Packet)); // 发送数据包
  // Serial.print("Sending Packet... ");
  // PrintInfo(); // 串口打印调试信息

  if (Joystick_Y >= JOYSTICK_Y_MID)
  {
    Angle_Y = map(Joystick_Y, JOYSTICK_Y_MID, ANALOG_MAX, 90, 180);
  }
  else
  {
    Angle_Y = map(Joystick_Y, ANALOG_MIN, JOYSTICK_Y_MID, 0, 90);
  }
  if (Joystick_X >= JOYSTICK_X_MID)
  {
    Angle_X = map(Joystick_X, JOYSTICK_X_MID, ANALOG_MAX, 90, 180);
  }
  else
  {
    Angle_X = map(Joystick_X, ANALOG_MIN, JOYSTICK_X_MID, 0, 90);
  }

  SetScreen(); // 设置显示屏
  delay(10); // 延时ms
}


/*
 * @brief 读取滑块、摇杆、电位器的数据并写入Packet
 */
void GetValue()
{
  Potentiometer_Val = constrain(analogRead(PIN_POTENTIOMETER), ANALOG_MIN, ANALOG_MAX);
  Potentiometer_Val = map(Potentiometer_Val, ANALOG_MIN, ANALOG_MAX, 0, 7);
  Slider_Val = constrain(analogRead(PIN_SLIDER), ANALOG_MIN, ANALOG_MAX);
  if (Potentiometer_Val < 3)
  {
    Slider_Val = map(Slider_Val, ANALOG_MIN, ANALOG_MAX, THROTTLE_MIN, THROTTLE_MAX - 131); // [0, 60]
  }
  else if (Potentiometer_Val < 5)
  {
    Slider_Val = map(Slider_Val, ANALOG_MIN, ANALOG_MAX, THROTTLE_MIN, THROTTLE_MAX - 111); // [0, 80]
  }
  else if (Potentiometer_Val < 7)
  {
    Slider_Val = map(Slider_Val, ANALOG_MIN, ANALOG_MAX, THROTTLE_MIN, THROTTLE_MAX - 91); // [0, 100]
  }
  else
  {
    Slider_Val = map(Slider_Val, ANALOG_MIN, ANALOG_MAX, THROTTLE_MIN, THROTTLE_MAX - 71); // [0, 120]
  }
  Joystick_X = constrain(analogRead(PIN_JOYSTICK_X), ANALOG_MIN, ANALOG_MAX);
  Joystick_Y = constrain(analogRead(PIN_JOYSTICK_Y), ANALOG_MIN, ANALOG_MAX);
  Joystick_B = !digitalRead(PIN_JOYSTICK_B);
  Packet = ((uint32_t)(Joystick_X & 0x3FF))
         | ((uint32_t)(Joystick_Y & 0x3FF) << 10)
         | ((uint32_t)(Joystick_B & 0x1) << 20)
         | ((uint32_t)(Slider_Val & 0xFF) << 21)
         | ((uint32_t)(Potentiometer_Val & 0x7) << 29); // 暂时不使用电位器信号
}


/*
 * @brief 串口打印控制信息
 */
void PrintInfo()
{
  Serial.print("Slider: ");
  Serial.print(Slider_Val);
  Serial.print(" | Joystick X: ");
  Serial.print(Joystick_X);
  Serial.print(" | Joystick Y: ");
  Serial.print(Joystick_Y);
  Serial.print(" | Joystick B: ");
  Serial.print(Joystick_B);
  Serial.print(" | Potentiometer: ");
  Serial.println(Potentiometer_Val);
}


/*
 * @brief 在显示屏输出控制信息
 */
void SetScreen()
{
  lcd.clear(); // 清空显示屏
  lcd.setCursor(0, 0);  // 设置光标位置
  lcd.print("^");
  lcd.print(Slider_Val - 8);
  lcd.setCursor(6, 0);
  lcd.print("b");
  lcd.print(Joystick_B);
  lcd.setCursor(10, 0);
  lcd.print("x");
  lcd.print((Angle_X >= 90) ? "+" + String(Angle_X - 90) : String(Angle_X - 90));
  lcd.setCursor(6, 1);
  lcd.print("p");
  lcd.print(Potentiometer_Val);
  lcd.setCursor(10, 1);
  lcd.print("y");
  lcd.print((Angle_Y >= 90) ? "+" + String(Angle_Y - 90) : String(Angle_Y - 90));
}