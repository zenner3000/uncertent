import zmq
import sys

context = zmq.Context()
print("Connecting to server...")
socket = context.socket(zmq.SUB)
socket.connect ("ipc:///tmp/myipct")

for request in range(1,10):
    print("sending request ",request,"...")
#    socket.send_string("Hello")
    socket.send(b"Hello")
    message = socket.recv()
    print("Received reply %s [%s]"%(request,message) )
