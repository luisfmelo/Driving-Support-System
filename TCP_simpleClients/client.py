import socket, time

port = 4004
host = 'localhost'

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect((host, port)) 
count = 0
while 1:
   msg = raw_input("Enter stuff: ")
   print client.send(msg)
   if msg == 'exit':
      break
   time.sleep(0.1)

print "Good bye!"
