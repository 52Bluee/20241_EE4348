import socket
import time
import json
import collections

LAST_TIME = time.time()
UPPER = 100; LOWER = 50

def connected_to_internet():
    """
    Check the Internet connection

    Returns:
        True if the connection is successful else False
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        # Attempt to connect to a reliable server (Google's DNS server)
        sock.settimeout(1)  # Set timeout for the connection attempt
        sock.connect(("8.8.8.8", 53))  # Google's DNS server on port 53 (DNS port)
        return True
    except (socket.timeout, socket.error):
        return False
    finally:
        # Close the socket whether the connection was successful or not
        sock.close()

def CoAP_Online():
    """
    Client mode.
    In this mode, the Controller sends stored data to ThingsBoard and get user's defined bound values.
    """
    global LAST_TIME, UPPER, LOWER
    # Prepare the telemetry data in JSON format

    # Define ThingsBoard CoAP server details
    THINGSBOARD_HOST =  "104.196.24.70"   # Replace with your ThingsBoard server address
    THINGSBOARD_PORT = 5683  # Default CoAP port
    payload = {
        "method": "getHumidityBoundValue",
        "params": {}
    }
    body = json.dumps(payload)

    # Prepare the CoAP request headers
    rpc_headers = b'\x50\x02\xf8\xbf=\x00104.196.24.70\x83api\x02v1\r\x07AyYM5ClBZVYZxwXuB6Sa\x03rpc\xff'
    telemetry_headers = b'\x50\x02\xf8\xbf=\x00104.196.24.70\x83api\x02v1\r\x07AyYM5ClBZVYZxwXuB6Sa\ttelemetry\xff'

    # Combine the headers and body to form the full CoAP request
    rpc_request = rpc_headers + body.encode('utf-8')

    # Create a socket and connect to the ThingsBoard server
    # sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDPsocket
    #sock.settimeout(20)
    # Update data when the Internet was lost
    if timeseries.get('humidity', None) is not None:
        for value, ts in zip(timeseries['humidity'], timeseries['ts']):
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDPsocke
            body = json.dumps({'ts': int(ts*1000), 'values': {'humidity': value}})
            tememetry_request = telemetry_headers + body.encode('utf-8')
            sock.sendto(tememetry_request, (THINGSBOARD_HOST, THINGSBOARD_PORT))
            response, _ = sock.recvfrom(4096)
            print('1')
    while True:
        if time.time() - LAST_TIME > 5:
            if not connected_to_internet():
                print('Lost Internet connection T.T')
                CoAP_Offline()
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDPsocke
            sock.sendto(rpc_request, (THINGSBOARD_HOST, THINGSBOARD_PORT))

            # Receive the response (waiting for an ACK or response)
            response, _ = sock.recvfrom(4096)  # Buffer size can be adjusted

            # Print the response (decode it to make it readable)
            response = response.decode('utf-8', errors = 'ignore')
            if '{' in response:
                body = response.split("{")[1]
                try:
                    body = json.loads("{" + body)
                except:
                    continue
                print("Response from ThingsBoard CoAP server:", body)  # Decoding the response to make it readable
                UPPER = float(body['upper'])
                LOWER = float(body['lower'])
            LAST_TIME = time.time()

def coap_message_type_switch(headers, body):
    """
    Filter Node, which detemines message based on its headers.
    Params:
        headers (str) - Headers of CoAP message in utf-8.
        body (dict) - Payload of CoAP message in json.
    Returns:
        Response body
    """
    if 'telemetry' in headers:
        coap_save_timeseries(body)
        return ""
    if 'rpc' in headers:
        rpc_call_reply = coap_rpc_request_from_device(body)
        return rpc_call_reply
    
def coap_save_timeseries(body):
    """
    Caching data for sending latter.
    Params:
        body (dict) - Payload of CoAP message in json.
    """
    for k, v in body.items():
        timeseries[k].append(v)
        timeseries['ts'].append(time.time())

def coap_rpc_request_from_device(body):
    """
    Handling RPC requests.
    Params:
        body (dict) - Payload of CoAP message in json.
    """
    if body.get('method', None) != 'getHumidityControlSignal':
        return None
    last_entity_data = timeseries.get('humidity', None)
    if last_entity_data is not None:
        last_entity_data = float(last_entity_data[-1])
        if last_entity_data < LOWER:
            controlsignal = 1
        elif last_entity_data > UPPER:
            controlsignal = -1
        else:
            controlsignal = 0
        return json.dumps({'humidityControlSignal': controlsignal})
    else:
        return None

def CoAP_Offline():
    """
    Server mode.
    Running localhost listening for devices' requests.
    """
    global timeseries
    print("Creating server...")
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDPsocket
    s.bind(('0.0.0.0', 10000))
    s.settimeout(2)
    timeseries = collections.defaultdict(list)
    #timeseries = {'humidity': [41, 266, 41, 89],
    #              'ts': [1733800499.7772536+1000, 1733800509.6265185+1000, 1733800516.1844318+1000, 1733800519.7815855+1000]}
    while True:
        print("CoAP cache:", timeseries)
        if connected_to_internet():
            print('The Internet is back ^-^')
            s.close()
            CoAP_Online()
        try:
            message, address = s.recvfrom(2048)
        except (socket.timeout, socket.error):
            continue
        message = message.decode('utf-8', errors = 'ignore')
        if '{' in message:
            headers = message.split('{')[0]
            body = message[len(headers):]
            print(body)
            body = json.loads(body)
            response_body = coap_message_type_switch(headers, body)
            if response_body is not None and len(response_body) != 0:
                reply_headers = b'\x50E4\xb0\xc12\xff'
                request = reply_headers + response_body.encode('utf-8')
                s.sendto(request, address)

if __name__ == '__main__':
    CoAP_Offline()