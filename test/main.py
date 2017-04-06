import socket
s= socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('127.0.0.1',8888))
s.sendall('123\nabc\n456\n')
while True:
    buf = s.recv(1024)
    print buf