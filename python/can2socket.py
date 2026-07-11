# import pyserial
import cantools
import os
import zmq
import json
import serial

start_requested = 0

class gvret_frame:
    timestamp: int
    id: int
    extended: bool
    dir: str
    bus: int
    len: int
    data: list[int]
    
    def __str__(self):
        return f"GVRET Frame: timestamp={self.timestamp}, id={self.id}, extended={self.extended}, dir={self.dir}, bus={self.bus}, len={self.len}, data={self.data}"

def loadDBCFilesInFolder(folder_path):
    dbc_files = [f for f in os.listdir(folder_path) if f.endswith('.dbc')]
    db = []
    for d in dbc_files:
        db.append(cantools.database.load_file(os.path.join(folder_path, d)))
    return db

def setupZMQSocket():
    context = zmq.Context()
    socket = context.socket(zmq.PUB)
    socket.bind("tcp://*:5555")
    return context, socket

def initSerial():
    try:
        ser = serial.Serial(port="COM4", baudrate=921600, timeout=1)
    except:
        ser = serial.Serial(port="/dev/ttyACM0", baudrate=921600, timeout=1)
    return ser

def readGVRETFromSerial(ser):
    # check if serial is connected
    if not ser.is_open:
        ser.open()

    # if no data back, send string "s" to start
    l = ser.readline().decode("utf-8").strip()
    print(l)
    if l != "":
        if l.count(",") == 14:
            ls = l.split(',')
            frame = gvret_frame()
            frame.timestamp = int(ls[0])
            frame.id = int(ls[1], 16)
            frame.extended = ls[2] == "true"
            frame.dir = ls[3]
            frame.bus = int(ls[4])
            frame.len = int(ls[5])
            frame.data = [int(x, 16) for x in ls[6:6+frame.len]]
            return frame
        if "Logging stopped" in l:
            return "Stopped"
    else:
        if start_requested == 0:
            ser.write(b"s\n")
        start_requested = 1
        return "Start requested"
    
def main():
    db = loadDBCFilesInFolder('../dbc')
    ser = initSerial()
    context, socket = setupZMQSocket()
    while (True):
        result = readGVRETFromSerial(ser)
        if result == "Stopped":
            break
        
        # decode message
        if (isinstance(result, gvret_frame)):
            # find dbc for this message
            found = 0
            for d in db:
                try:
                    msg = d.get_message_by_frame_id(result.id)
                    decoded = msg.decode(bytes(result.data))
                    found = 1
                    break
                except Exception as e:
                    pass

            # push to socket
            if found == 1:
                value = {
                    "timestamp": result.timestamp / 1e6,
                }
                for key in decoded:
                    if isinstance(decoded[key], (int, float, str)):
                        value[key] = decoded[key]
                
                # socket.send_string("test", zmq.SNDMORE)
                try:
                    socket.send_string(json.dumps(value))
                except Exception as e:
                    print(f"Error sending message: {e}")

if __name__ == "__main__":
    main()
