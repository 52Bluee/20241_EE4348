import threading

from HTTP_Controller import HTTP_Offline
from CoAP_Controller import CoAP_Offline
from MQTT_Controller import MQTT_Offline

if __name__ == '__main__':
    t1 = threading.Thread(target = HTTP_Offline, args = ())
    t2 = threading.Thread(target = CoAP_Offline, args = ())
    t3 = threading.Thread(target = MQTT_Offline, args = ())
    print("Waiting for connections...")

    t1.start()
    t2.start()
    t3.start()

    t1.join()
    t2.join()
    t3.join()


    