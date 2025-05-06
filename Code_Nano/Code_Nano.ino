// 下位机

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

RF24 Radio(7,8); // 创建RF24对象，CE对应7号引脚, CSN对应8号引脚
const byte address[6] = "00006"; // 创建地址，用于识别发送端和接收端

Servo Motor; // 主翼电机
Servo Servo_Pitch; // 俯仰舵机
Servo Servo_Roll; // 滚转舵机
#define PIN_MOTOR 3 // 主翼电机对应的引脚, 电调白线插D3
#define PIN_SERVO_PITCH 6 // 俯仰舵机对应的引脚, 黄线插D6
#define PIN_SERVO_ROLL 5 // 滚转舵机对应的引脚, 黄线插D5+++++++++++++++++++++++++++++
#define ANALOG_MAX 1023 // 模拟信号最大值
#define ANALOG_MIN 0 // 模拟信号最小值
#define THROTTLE_MAX 199 // 主翼电机转速信号最大值，对应占空比11.57%
#define THROTTLE_MIN 8 // 主翼电机转速信号最小值，对应占空比3.15%
#define JOYSTICK_X_MID 527 // 摇杆X轴中位值
#define JOYSTICK_Y_MID 488 // 摇杆Y轴中位值
#define PITCH_MAX 140 // 俯仰舵机最大角度
#define ROLL_MAX 130 // 滚转舵机最大角度
int Slider_Val = THROTTLE_MIN; // 电机转速信号
int Joystick_X = JOYSTICK_X_MID; // 摇杆X轴信号
int Joystick_Y = JOYSTICK_Y_MID; // 摇杆Y轴信号
int Joystick_B = 0; // 摇杆按钮信号, 0: 未按下, 1: 按下
int Angle_X = 90; // 俯仰舵机位置
int Angle_Y = 90; // 滚转舵机位置
int Potentiometer_Val = 0; // 电位器信号
uint32_t Packet = 0; // 接收数据包


void setup() 
{
  // 初始化串口
  Serial.begin(9600);
  Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n==============================");

  // 初始化天线
  Radio.begin();
  Radio.openReadingPipe(1, address); // 设置地址，0-5指定打开的管道
  Radio.setPALevel(RF24_PA_MIN); // 设置功率放大器级别，RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  Radio.setDataRate(RF24_1MBPS); // 设置发送速率，RF24_250KBPS, RF24_1MBPS, RF24_2MBPS
  Radio.startListening(); // 设置为接收端
  
  // 启动电调
  ActivateESC();

  // 初始化舵机
  Servo_Pitch.attach(PIN_SERVO_PITCH);
  Servo_Roll.attach(PIN_SERVO_ROLL);
  SetServoAngle(90, 90); // 归位
}


void loop() 
{
  GetValue(); // 读取天线数据
  // PrintInfo(); // 串口打印调试信息
  if (Joystick_B || !Potentiometer_Val) // 摇杆按钮按下或电位器为0
  {
    Slider_Val = THROTTLE_MIN; // 关闭电机
  }
  SetThrottle(Slider_Val); // 设置油门
  // SetServoAngle(map(Joystick_Y, ANALOG_MIN, ANALOG_MAX, 0, 180), map(Joystick_X, ANALOG_MIN, ANALOG_MAX, 0, 180)); // 设置舵机位置
  if (Joystick_Y >= JOYSTICK_Y_MID)
  {
    Angle_Y = map(Joystick_Y, JOYSTICK_Y_MID, ANALOG_MAX, 90, 180 - PITCH_MAX);
  }
  else
  {
    Angle_Y = map(Joystick_Y, ANALOG_MIN, JOYSTICK_Y_MID, PITCH_MAX, 90);
  }
  if (Joystick_X >= JOYSTICK_X_MID)
  {
    Angle_X = map(Joystick_X, JOYSTICK_X_MID, ANALOG_MAX, 90, ROLL_MAX);
  }
  else
  {
    Angle_X = map(Joystick_X, ANALOG_MIN, JOYSTICK_X_MID, 180 - ROLL_MAX, 90);
  }
  SetServoAngle(Angle_Y, Angle_X); // 设置舵机位置
  delay(10); // 延时ms
}


/*
 * @brief 带倒计时提示的延时函数
 * @param s 等待时间，单位s
 */
void Wait(int s)
{
  for (int i = 0; i < s; i++)
  {
    Serial.print("Waiting: ");
    Serial.println(s - i);
    delay(1000); // 等待1s
  }
}


/*
 * @brief 读取天线数据
 */
void GetValue()
{
  if (Radio.available())
  {
    Radio.read(&Packet, sizeof(Packet));
    Joystick_X = Packet & 0x3FF;
    Joystick_Y = (Packet >> 10) & 0x3FF;
    Joystick_B = (Packet >> 20) & 0x1;
    Slider_Val = (Packet >> 21) & 0xFF;
    Potentiometer_Val = (Packet >> 29) & 0x7;
  }
}


/*
 * @brief 设置油门
 * @param val 油门数值, THROTTLE_MIN ~ THROTTLE_MAX
 */
void SetThrottle(int val)
{
  Motor.write(constrain(val, THROTTLE_MIN, THROTTLE_MAX)); // 将油门比例转换为对应的计数值，ratio的范围0到255，对应的数值范围8到199
  // Serial.print("Setting value: ");
  // Serial.println(constrain(val, THROTTLE_MIN, THROTTLE_MAX));
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
  Serial.println(Joystick_B);
  Serial.print(" | Potentiometer: ");
  Serial.println(Potentiometer_Val);
}


/*
 * @brief 设置舵机位置
 * @param p 俯仰舵机位置, [0, 180]
 * @param r 滚转舵机位置, [0, 180]
 */
void SetServoAngle(int p, int r)
{
  Servo_Pitch.write(p);
  Servo_Roll.write(r);
  // Serial.print("Pitch: ");
  // Serial.print(p);
  // Serial.print(" | Roll: ");
  // Serial.println(r);
}


/*
 * @brief 启动主翼电调
 * @note 操作顺序: 单片机RESET -> 看到串口信息 -> 立刻连接电池
 */
void ActivateESC()
{
  Motor.attach(PIN_MOTOR);
  delay(1);
  SetThrottle(THROTTLE_MAX); // 设置油门最大
  Serial.println("Please connect the battery to the ESC in 5 seconds!");
  // Wait(5);
  delay(500);
  SetThrottle(THROTTLE_MIN); // 设置油门最小
  // Wait(5);
  delay(500);
  Serial.println("ESC is ready");
}