#include <Ticker.h>

Ticker counter_timer;  // 用计时器来中断来实时更新电机位置
float dt = 0.001;       // 1ms计算一次电机位置
String req = "";

StepMotor motor_0(33, 32, 0);
StepMotor motor_1(26, 25, 1);
StepMotor motor_2(14, 27, 2);


void IRAM_ATTR distanceCounter(){
    // 计时器触发的回调函数，计算一个时间微分内的电机位置变化量
    motor_0.updateDistance(dt);
    motor_1.updateDistance(dt);
}

void size64Writer(int64_t msg64){
    uint8_t buffer = 0;
    for(uint8_t i=0; i<8; i++){
        buffer = msg64 >> i * 8;
        Serial.write(buffer);
    }
}


void setup(){
    Serial.begin(115200);
    Serial.setTimeout(5);
    Serial2.begin(115200);
    Serial2.setTimeout(5);

    motor_0.strokeInit(18, 5, LOW);
    motor_1.strokeInit(19, 23, LOW);

    counter_timer.attach(dt, distanceCounter);
    Serial.println("等待指令中");
    Serial2.println("等待指令中");
}

void req_deal(String req){
    // 01234567 8 9
    // :0 -1600\r\n
    if (req.substring(0, 1) == ":"){
        uint8_t motorId = req.substring(1).toInt();
        double Spd = req.substring(3).toFloat();
        if     (motorId == 0){
            motor_0.motorSpeed(Spd);
        }
        else if(motorId == 1){
            motor_1.motorSpeed(Spd);
        }
        else if(motorId == 2){
            motor_2.motorSpeed(Spd);
        }
        
    }
    else if (req.substring(0, 1) == "?"){
        Serial.print("::");
        size64Writer(motor_0.nowDistance);
        size64Writer(motor_1.nowDistance);
        Serial.print("\n");
    }
    else if (req.substring(0, 1) == "!"){
        motor_0.motorSpeed(0);
        motor_1.motorSpeed(0);
        motor_2.motorSpeed(0);
    }
    else if (req.substring(0, 1) == "r"){
        motor_0.nowDistance = 0;
        motor_1.nowDistance = 0;
    }
}


void loop(){
    motor_0.loop_strokeChecker();
    motor_1.loop_strokeChecker();

    if (Serial.available()) {
        req = Serial.readStringUntil('\n');
        Serial.println(req);    
    }
    else if (Serial2.available()) {
        req = Serial2.readStringUntil('\n');
        Serial2.println(req);    
    }

    if (req != ""){
        req_deal(req);
    }
    req = "";
}
