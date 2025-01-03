from tb_device_mqtt import TBDeviceMqttClient, TBPublishInfo

import socket
import json
import time
import collections

def connected_to_internet():
    """
    Check the Internet connection

    Returns:
        True if the connection is successful else False
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        # Attempt to connect to a reliable server (Google's DNS server)
        sock.settimeout(0.5)  # Set timeout for the connection attempt
        sock.connect(("8.8.8.8", 53))  # Google's DNS server on port 53 (DNS port)
        return True
    except (socket.timeout, socket.error):
        return False
    finally:
        # Close the socket whether the connection was successful or not
        sock.close()

def MQTT_Online():
    """
    Client mode.
    In this mode, the Controller sends stored data to ThingsBoard.
    """
    client = TBDeviceMqttClient( "104.196.24.70", username="AyYM5ClBZVYZxwXuB6Sa")
    client.connect()
    # Update data stored when the Internet was lost
    if timeseries.get('temperature', None) is not None:
        for value, ts in zip(timeseries['temperature'], timeseries['ts']):
            body = {'ts': int(ts*1000), 'values': {'temperature': value}}
            print(body)
            client.send_telemetry(body)
            result = client.send_telemetry(body)
            success = result.get() == TBPublishInfo.TB_ERR_SUCCESS
    client.disconnect()
    while True:
        if not connected_to_internet():
            print('Lost Internet connection (╥﹏╥)')
            MQTT_Offline()

def mqtt_message_type_switch(headers, body):
    """
    Filter Node, which detemines message based on its headers.
    Params:
        headers (str) - Headers of MQTT message in utf-8.
        body (dict) - Payload of MQTT message in json.
    Returns:
        Response body
    """
    if 'telemetry' in headers:
        mqtt_save_timeseries(body)
        return ""
    
def mqtt_save_timeseries(body):
    """
    Caching data for sending latter.
    Params:
        body (dict) - Payload of MQTT message in json.
    """
    for k, v in body.items():
        timeseries[k].append(v)
        timeseries['ts'].append(time.time())

def MQTT_Offline():
    """
    Server mode.
    Listening for Publishers' messages.
    """
    global timeseries
    print("Creating server")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # TCP socket
    s.bind(('0.0.0.0', 9999))
    s.listen(5)  # Allow 5 incoming connections in the backlog queue
    client, address = s.accept()
    print(f"Connected to {address}")
    timeseries = collections.defaultdict(list)
    while True:
        print("MQTT cache:", timeseries)
        if connected_to_internet():
            print('The Internet is back (ꈍᴗꈍ)♡')
            s.close()
            client.close()
            MQTT_Online()
        try:
            content = client.recv(1024)  # Increase buffer size to handle more data at once
        except (socket.timeout, socket.error):
            continue

        if len(content) == 0:
            break
        
        message = content.decode('utf-8', errors = 'ignore')
        if '{' in message:
            headers = message.split('{')[0]
            body = message[len(headers):]
            try:
                body = json.loads(body)
            except:
                continue
            response_body = mqtt_message_type_switch(headers, body)
        client.sendall(b'\x20\x02\x00\x00')

if __name__ == '__main__':
    MQTT_Offline()
