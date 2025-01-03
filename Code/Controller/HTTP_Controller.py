import socket
import datetime
import re
import collections
import json
import time

LAST_TIME = time.time()
UPPER = 100
LOWER = 50

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

def HTTP_Online():
    """
    Client mode.
    In this mode, the Controller sends stored data to ThingsBoard and get user's defined bound values.
    """
    global LAST_TIME, UPPER, LOWER
    # ThingsBoard details
    HOST = "104.196.24.70"  # ThingsBoard server URL
    PORT = 80  # HTTP port
    ACCESS_TOKEN = "AyYM5ClBZVYZxwXuB6Sa"  # Your device access token
    RPC_ENDPOINT = f"/api/v1/{ACCESS_TOKEN}/rpc"  # ThingsBoard API endpoint
    TELEMETRY_ENDPOINT = f"/api/v1/{ACCESS_TOKEN}/telemetry"
    payload = {
        "method": "getLightBoundValue",
        "params": {}
    }
    body = json.dumps(payload)

    # Prepare the HTTP request headers
    rpc_headers = (
        f'POST {RPC_ENDPOINT} HTTP/1.1\r\n'
        f'Host: {HOST}\r\n'
        'Content-Type: application/json\r\n'
        f'Content-Length: {len(body)}\r\n'
        '\r\n')
    telemetry_headers = (
        'POST {} HTTP/1.1\r\n'
        'Host: {}\r\n'
        'Content-Type: application/json\r\n'
        'Content-Length: {}\r\n'
        '\r\n')

    # Combine the headers and body to form the full HTTP request
    rpc_request = rpc_headers.encode('utf-8') + body.encode('utf-8')

    # Create a socket and connect to the ThingsBoard server
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(10)  # Setting a longer timeout to wait for the response
    sock.connect((HOST, PORT))

    # Update data when the Internet was lost
    if timeseries.get('light', None) is not None:
        for value, ts in zip(timeseries['light'], timeseries['ts']):
            body = json.dumps({'ts': int(ts*1000), 'values': {'light': value}})
            tememetry_request = telemetry_headers.format(TELEMETRY_ENDPOINT, HOST, len(body)).encode('utf-8') + body.encode('utf-8')
            sock.send(tememetry_request)
            response = sock.recv(4096)
    try:
        while True:
            # Send the HTTP POST request if enough time has passed (5 seconds)
            if time.time() - LAST_TIME > 10:
                if not connected_to_internet():
                    sock.close()
                    print('Lost Internet connection T.T')
                    HTTP_Offline()
                sock.send(rpc_request)

                # Receive the response
                response = sock.recv(4096).decode('utf-8')

                # Check if there is a valid response body
                header_end_index = response.find("\r\n\r\n")
                if header_end_index != -1:
                    body = response[header_end_index + 4:]  # Get the body of the response

                    try:
                        # Parse the JSON body
                        response_json = json.loads(body)

                        # Extract 'upper' and 'lower' values
                        UPPER = float(response_json['upper'])
                        LOWER = float(response_json['lower'])
                        LAST_TIME = time.time()

                        print(f"Upper: {UPPER}, Lower: {LOWER}")

                    except json.JSONDecodeError:
                        print("Error decoding JSON response")

    except socket.timeout:
        print("Socket timeout. Could not receive a response.")
    except socket.error as e:
        print(f"Socket error: {e}")
    finally:
        sock.close()  # Always close the socket to free up resources

def get_content_length(headers):
    """
    Get the content length

    Params:
        headers (str) - Header of message in utf-8
    Returns:
        The length of content
    """
    match = re.search(r'Content-Length: (\d+)', headers, re.IGNORECASE)
    if match:
        return int(match.group(1))
    return 0

def message_type_switch(headers, body):
    """
    Filter Node, which detemines message based on its headers.
    Params:
        headers (str) - Headers of HTTP message in utf-8.
        body (dict) - Payload of HTTP message in json.
    Returns:
        Response body
    """
    uri = headers.split(' ')[1]
    message_type = uri.split('/')[-1]
    if message_type == 'telemetry':
        save_timeseries(body)
        return ""
    if message_type == 'rpc':
        rpc_call_reply = rpc_request_from_device(body)
        return rpc_call_reply
    
def save_timeseries(body):
    """
    Caching data for sending latter.
    Params:
        body (dict) - Payload of HTTP message in json.
    """
    for k, v in body.items():
        timeseries[k].append(v)
        timeseries['ts'].append(time.time())

def rpc_request_from_device(body):
    """
    Handling RPC requests.
    Params:
        body (dict) - Payload of HTTP message in json.
    """
    if body.get('method', None) != 'getLightControlSignal':
        return None
    last_entity_data = timeseries.get('light', None)
    if last_entity_data is not None:
        last_entity_data = float(last_entity_data[-1])
        if last_entity_data < LOWER:
            controlsignal = 1
        elif last_entity_data > UPPER:
            controlsignal = -1
        else:
            controlsignal = 0
        return json.dumps({'lightControlSignal': controlsignal})
    else:
        return None
    
def HTTP_Offline():
    """
    Server mode.
    Running localhost listening for devices' requests.
    """
    global timeseries
    print("Creating server...")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # TCP socket
    s.bind(('0.0.0.0', 10000))
    s.listen(5)  # Allow 5 incoming connections in the backlog queue
    s.settimeout(2)
    timeseries = collections.defaultdict(list)
    while True:
        print("HTTP cache:", timeseries)
        if connected_to_internet():
            print('The Internet is back ^-^')
            s.close()
            HTTP_Online()
        try:
            client, address = s.accept()
            print(f"Connected to {address}")
            headers = b''
        except (socket.timeout, socket.error):
            continue
        
        while True:
            content = client.recv(1024)
            if not content:
                break
            headers += content
            # Check if the full HTTP message (including headers) is received
            if b'\r\n\r\n' in headers:
                break
    
        headers = headers.decode('utf-8', errors='ignore')
        body_length = get_content_length(headers)  # Get content length if available
        if body_length > 0:
            # Read the body if Content-Length is specified
            body = b''  # Extract the body from the message
            crtime = 0
            while len(body) < body_length:
                content = client.recv(1024)
                body += content
                if time.time() - crtime > 1:
                    break
                else:
                    crtime = time.time()
        try:
            body = json.loads(body.decode('utf-8', errors='ignore'))
            print(f"Received body: {body}")
            response_body = message_type_switch(headers, body)
        except:
            continue
        if response_body is not None:
            response_body = response_body.encode('utf-8')
            response_headers = (
                'HTTP/1.1 200 OK\r\n'
                'vary: Origin\r\n'
                'vary: Access-Control-Request-Method\r\n'
                'vary: Access-Control-Request-Headers\r\n'
                'x-content-type-options: nosniff\r\n'
                'x-xss-protection: 0\r\n'
                'cache-control: no-cache, no-store, max-age=0, must-revalidate\r\n'
                'pragma: no-cache\r\n'
                'expires: 0\r\n'
                'x-frame-options: DENY\r\n'
                'content-length: {}\r\n'
                'date: {}\r\n\r\n'
            ).format(len(response_body), str(datetime.datetime.now(datetime.UTC)))
            client.sendall(response_headers.encode('utf-8') + response_body)

if __name__ == '__main__':
    #if connected_to_internet():
    #    HTTP_Online()
    #else:
    HTTP_Offline()