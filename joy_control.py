import sys
from pygame import joystick
import math
# 导入依赖
import time
import serial
import pygame



def spd_map(map_0, map_1):
    """将速度从区间a线性映射到区间b，返回函数的斜率k和偏置b"""
    k = (map_1[1] - map_1[0]) / (map_0[1] - map_0[0])
    b = (map_1[0] - k*map_0[0])
    return k, b


class joy_and_uart():
    def __init__(self, uart, joy_num) -> None:
        self.uart = uart
        pygame.init()
        pygame.joystick.init()
        self.joystick = pygame.joystick.Joystick(joy_num)
        self.joystick.init()
        self.CLOCK = pygame.time.Clock()
        self.FPS = 120
        self.spd_list = [0 for i in range(self.joystick.get_numaxes())]
    
    def main_loop(self):
        k0, b0 = spd_map((-1, 1), (3600, -3600))
        k1, b1 = spd_map((-1, 1), (-3600, 3600))
        k3, b3 = spd_map((-1, 1), (-6400, 6400))
        while True:
            for event in pygame.event.get():
                pass
            self.speed_ctrl(0, 0, k0, b0)
            self.speed_ctrl(1, 1, k1, b1)
            self.speed_ctrl(2, 2, k3, b3)
            self.CLOCK.tick(self.FPS)
    
    def speed_ctrl(self, axis_id, moto_id, k, b):
        axis_value = self.joystick.get_axis(axis_id)
        if 0.05 > axis_value  > -0.05:
            axis_value  = 0
        if -1. < axis_value  < -0.995:
            axis_value  = -1
        if 1. > axis_value  > 0.995:
            axis_value  = 1
        speed = k*axis_value + b
        speed_old = self.spd_list[axis_id]
        if abs(speed-speed_old) > 5 or (axis_value==0 and speed_old!=0):
            
            self.set_speed(moto_id, speed)
            self.spd_list[axis_id] = speed
    
    def set_speed(self, moto_id, spd_v):
        msg = f":{moto_id} {int(spd_v)}\r\n"
        print(msg)
        self.uart.write(msg.encode())

if __name__ == "__main__":
    # 参数配置
    # 角度定义
    SERVO_PORT_NAME =  'COM3' # 舵机串口号
    SERVO_BAUDRATE = 115200 # 舵机的波特率

    # 初始化串口
    uart = serial.Serial(port=SERVO_PORT_NAME, baudrate=SERVO_BAUDRATE,\
					 parity=serial.PARITY_NONE, stopbits=1,\
					 bytesize=8,timeout=0)
    # 初始化舵机管理器
    a = joy_and_uart(uart, 0)
    a.main_loop()

        
