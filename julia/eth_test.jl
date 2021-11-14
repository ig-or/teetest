


using Sockets
s = Sockets.UDPSocket()
# Listen on all local IP addresses for UDP messages sent to port 12222
Sockets.bind(s, ip"0.0.0.0", 8888)

hostport, packet = Sockets.recvfrom(s)
Sockets.send(s, ip"192.168.0.177", 8888, "Hi")
hostport