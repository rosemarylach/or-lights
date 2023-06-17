import socket
import time

my_ip = socket.gethostbyname(socket.gethostname())
print("ip: ", my_ip)
port = 50000
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(20.0)
s.bind((my_ip, port))
s.listen(1)
conn, addr = s.accept()
# x = 2.6
# y = 1.3
# r = 0.6
# h = 2.8
# b = 93
# send_str = "x:{:.2f}y:{:.2f}r:{:.2f}h:{:.2f}b:{}\n".format(x, y, r, h, b)
# pitch = [48, -48, -48, 48]
# yaw = [45, -45, -45, 45]
# focus = [-0.1, -0.1, -0.1, -0.1]
pitch = [0, 0, 0, 0]
yaw = [0, 0, 0, 0]
focus = [0, 0, 0, 0]
b = 0

send_str = "f1:{:.2f}p1:{:.2f}y1:{:.2f}" \
            "f2:{:.2f}p2:{:.2f}y2:{:.2f}" \
            "f3:{:.2f}p3:{:.2f}y3:{:.2f}" \
            "f4:{:.2f}p4:{:.2f}y4:{:.2f}" \
            "b:{}\n".format(
                    focus[0], pitch[0], yaw[0],
                    focus[1], pitch[1], yaw[1],
                    focus[2], pitch[2], yaw[2],
                    focus[3], pitch[3], yaw[3],
                    b
                   )
while True:
    conn.sendall(bytes(send_str, 'utf-8'))
    time.sleep(3)