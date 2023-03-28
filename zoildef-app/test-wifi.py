import socket
import time

my_ip = "168.5.90.169"
port = 50000
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(20.0)
s.bind((my_ip, port))
s.listen(1)
conn, addr = s.accept()
x = 2.6
y = 1.3
r = 0.6
h = 2.8
b = 93
send_str = "x:{:.2f}y:{:.2f}r:{:.2f}h:{:.2f}b:{}\n".format(x, y, r, h, b)
while True:
    conn.sendall(bytes(send_str, 'utf-8'))
    time.sleep(1)