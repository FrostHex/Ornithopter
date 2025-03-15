// 下位机

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

RF24 radio(7,8); // 创建RF24对象，CE对应7号引脚, CSN对应8号引脚
const byte address[6] = "00006"; // 创建地址，用于识别发送端和接收端

Servo servo_main; // 主翼电机
#define PIN_SERVO_MAIN 1 // 主翼电机对应的引脚
#define PIN_SERVO_ROLL 9 // 横滚舵机对应的引脚
#define PIN_SERVO_PITCH 10 // 俯仰舵机对应的引脚
#define ANALOG_MAX 1023 // 模拟信号最大值
#define ANALOG_MIN 0 // 模拟信号最小值
#define THROTTLE_MAX 199 // 主翼电机转速信号最大值，对应占空比11.57%
#define THROTTLE_MIN 8 // 主翼电机转速信号最小值，对应占空比3.15%
int Slider_Val = 0; // 电机转速信号
int Joystick_X = 0; // 摇杆X轴信号
int Joystick_Y = 0; // 摇杆Y轴信号
int Joystick_B = 0; // 摇杆按钮信号, 0: 未按下, 1: 按下
uint32_t Packet = 0; // 接收数据包


void setup() 
{
  // 初始化串口
  Serial.begin(9600);
  Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n==============================");

  // 初始化天线
  radio.begin();
  radio.openReadingPipe(1, address); // 设置地址，0-5指定打开的管道
  radio.setPALevel(RF24_PA_LOW); // 设置功率放大器级别，RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_1MBPS); // 设置发送速率，RF24_250KBPS, RF24_1MBPS, RF24_2MBPS
  radio.startListening(); // 设置为接收端
  
  // 启动电调
  ActivateESC();
}


void loop() 
{
  // 读取天线数据
  if (radio.available())
  {
    radio.read(&Packet, sizeof(Packet));
    //TODO: 解析数据包
  }

  // 设置油门和舵机
  SetThrottle(Slider_Val);
  // Serial.print("Joystick X: ");
  // Serial.println(Joystick_X);
  // Serial.print("Joystick Y: ");
  // Serial.println(Joystick_Y);

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
 * @brief 设置油门
 * @param val 油门数值, THROTTLE_MIN ~ THROTTLE_MAX
 */
void SetThrottle(int val)
{
  servo_main.write(constrain(val, THROTTLE_MIN, THROTTLE_MAX)); // 将油门比例转换为对应的计数值，ratio的范围0到255，对应的数值范围8到199
  Serial.print("Setting value: ");
  Serial.println(constrain(val, THROTTLE_MIN, THROTTLE_MAX));
}


/*
 * @brief 启动主翼电调
 * @note 操作顺序: 单片机RESET -> 看到串口信息 -> 立刻连接电池
 */
void ActivateESC()
{
  servo_main.attach(PIN_SERVO_MAIN);
  delay(1);
  SetThrottle(100); // 设置油门最大
  Serial.println("Please connect the battery to the ESC in 5 seconds!");
  Wait(5);
  SetThrottle(0); // 设置油门最小
  Wait(5);
  Serial.println("ESC is ready");
}