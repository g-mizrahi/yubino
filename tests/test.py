import serial
import time
ser = serial.Serial(port='/dev/ttyACM0', timeout=1)  # open serial port
# wait for device to wake up
time.sleep(2)

request = 0x0.to_bytes(1, 'big')
print(f"sending {request}")
ser.write(request)
ser.flush()

print(ser.read(2))

while True:
    time.sleep(0.1)
    print(ser.read(36))

ser.close()