import serial
import serial.tools.list_ports
import time

class Move_Robot():
    def __init__(self, COM_num):

        self.ser = serial.Serial()
        self.ser.baudrate = 115200
        self.ser.port = COM_num
        self.ser.open()
        self.position = None
        self.position_init()
        self.spd_now = 0

    def move(self, vec, spd):

        """用来输入相对坐标，单位为mm，速度单位为mm/s 提示1.875mm/s为100脉冲"""
        x,y = vec
        self.spd = spd
        if self.spd < 0:
            print("invaild spd")
        move_time = ((x **2 + y **2) **0.5) / self.spd
        print(move_time)
        # spd单位mm/s
        spdx = x/move_time
        spdy = y/move_time
        
        freqx = (spdx/3) * 160
        freqy = (spdy/3) * 160

        if self.ser.isOpen():
            try:
                moto_0_msg = f":0 {int(freqx)}\r\n".encode()
                moto_1_msg = f":1 {int(freqy)}\r\n".encode()
                self.ser.write(moto_0_msg)
                self.ser.write(moto_1_msg)
                time.sleep(move_time)
                self.ser.write(b":1 0\r\n")
                self.ser.write(b":0 0\r\n")
                if self.position is not None:
                    x0,y0 = self.position
                    x1 = 160 if x0+x > 160 else x0+x
                    y1 = 160 if y0+y > 160 else y0+y
                    self.position = (x1,y1)
            except BaseException as e:
                print("invalid msg" + str(e))
            print(self.position)

    def position_init(self):
        """复位函数，回到原点"""
        if self.ser.isOpen():
            
            self.move((-200,-200), 100)
            self.position = (0,0)
    
    def move_to(self,vec,spd):
        """绝对坐标,总的地图尺寸为160mm*160mm"""
        x1,y1 = vec
        x0,y0 = self.position
        dx = x1 - x0
        dy = y1 - y0
        
        if self.ser.isOpen():
            self.move((dx,dy), spd)

    def setRotate(self,rps):
        """输入电机转速，单位为 圈/秒"""
        spd_target = int(rps * 1600)
        while self.spd_now != spd_target:
            if self.spd_now < spd_target:
                self.spd_now += 1600
                self.spd_now = spd_target if self.spd_now > spd_target else self.spd_now
            if self.spd_now > spd_target:
                self.spd_now -= 1600
                self.spd_now = spd_target if self.spd_now < spd_target else self.spd_now
            self.ser.write(f":2 {self.spd_now}\r\n".encode())
            time.sleep(0.5)


if __name__ == "__main__":
    # 这里修改成自己需要的串口号
    Robot = Move_Robot("COM3")
    Robot.move((100,100),5)
    Robot.move_to((40,50),50)
