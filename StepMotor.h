#ifndef StepMotor_H //预编译指令，防止重复定义类
#define StepMotor_H

class StepMotor{
    public:
        StepMotor(uint8_t, uint8_t, uint8_t); //类的构造函数，与类名相同
        void motorSpeed(double);
        void strokeInit(uint8_t, uint8_t, uint8_t);
        void updateDistance(float);
        void loop_strokeChecker();
        uint8_t dirState;
        double  nowFreq;
        int64_t nowDistance=0;
    private:
        //-------------电机与PWM通道所使用的变量-------------------
        uint8_t motorChannel;
        uint8_t dirPin;
        uint8_t pulPin;
        //--------------为限位开关功能使用的变量--------------------
        uint8_t positivePin;
        uint8_t negativePin;
        uint8_t strokeOnPinMode  = HIGH;  // 限位开关为“触发”时这个管脚的状态
        uint8_t positivePinValue = 0;
        uint8_t negativePinValue = 0;
        //---------------为计时器更新位置使用的变量-----------------
        float dir_ = -1;              // 方向标识符
        int   dx_  =  0;              // 时间微分
};

StepMotor :: StepMotor(uint8_t Pul, uint8_t Dir, uint8_t motorChannel){
    // 初始化一个电机，提供其关联管脚与电机号
    this->motorChannel = 2*motorChannel;
    this->dirPin = Dir;
    this->pulPin = Pul;
    pinMode(dirPin, OUTPUT);
    digitalWrite(dirPin, LOW);
    ledcSetup(this->motorChannel, 0, 10);
    ledcAttachPin(Pul, this->motorChannel);
}

void StepMotor :: motorSpeed(double freq_Speed){
    // 设置电机的速度
    // 只有没有触发限位时才有效
    if(freq_Speed>0 && positivePin!=strokeOnPinMode){
        digitalWrite(dirPin, HIGH);
        this->dirState = HIGH;
    }
    else if (freq_Speed<0 && negativePin!=strokeOnPinMode){
        digitalWrite(dirPin, LOW);
        this->dirState = LOW;
    }
    this->nowFreq = abs(freq_Speed);
    ledcWriteTone(motorChannel, nowFreq);
}

void StepMotor :: strokeInit(uint8_t positivePin, uint8_t negativePin, uint8_t strokeOnPinMode=HIGH){
    // 初始化限位开关
    pinMode(positivePin, INPUT_PULLUP);
    pinMode(negativePin, INPUT_PULLUP);
    this->positivePinValue = digitalRead(positivePin);
    this->negativePinValue = digitalRead(negativePin);
    this->positivePin = positivePin;
    this->negativePin = negativePin;
    this->strokeOnPinMode = strokeOnPinMode;
}

void StepMotor :: loop_strokeChecker(){
    // 把这个放在主循环里来做限位检查
    uint8_t now_positive = !strokeOnPinMode;
    uint8_t now_negative = !strokeOnPinMode;
    // 信号滤波
    if(digitalRead(positivePin) == strokeOnPinMode){
        delay(1);
        if(digitalRead(positivePin)==strokeOnPinMode){
            now_positive = strokeOnPinMode;
        }
    }
    if(digitalRead(negativePin) == strokeOnPinMode){
        delay(1);
        if(digitalRead(negativePin)==strokeOnPinMode){
            now_negative = strokeOnPinMode;
        }
    }

    if(positivePinValue!=strokeOnPinMode && now_positive==strokeOnPinMode){
        // 开启沿触发
        if(nowFreq!=0.0 && dirState){
            ledcWriteTone(motorChannel, 0.0);
            this->nowFreq = 0.0;
        }
    }

    if(negativePinValue!=strokeOnPinMode && now_negative==strokeOnPinMode){
        // 开启沿触发
        if(nowFreq!=0.0 && !dirState){
            ledcWriteTone(motorChannel, 0.0);
            this->nowFreq = 0.0;
        }
    }
    // 更新限位状态
    this->positivePinValue = now_positive;
    this->negativePinValue = now_negative;
    // Serial2.print(positivePin);
    // Serial2.print(':');
    // Serial2.print(positivePinValue);
    // Serial2.print('\t');
    // Serial2.print(negativePin);
    // Serial2.print(':');
    // Serial2.println(negativePinValue);
}

void StepMotor :: updateDistance(float dt){
    dir_ = -1;
    if(dirState){
        dir_ = 1;
    }
    // 结果为整形，但是为了提升精度：100,005代表100.005，表示的脉冲数附带了小数点后三位的结果。
    dx_ = (int)(nowFreq * dt * 1000 * dir_);
    nowDistance = nowDistance + dx_;
}
#endif
