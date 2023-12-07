import serial
import time
ser = serial.Serial(port='/dev/ttyACM0', timeout=0.2)  # open serial port
# wait for device to wake up
time.sleep(2)

request = 0x0.to_bytes(1, 'big')
print(f"sending {request}")
ser.write(request)
ser.flush()

time.sleep(1)
print(ser.readall())

time.sleep(1)

request = 0x3.to_bytes(1, 'big')
print(f"sending {request}")
ser.write(request)
ser.flush()

time.sleep(10)
print(ser.readall())


request = 0x0.to_bytes(1, 'big')
print(f"sending {request}")
ser.write(request)
ser.flush()

time.sleep(1)
while True:
    print(ser.readall())


ser.close()