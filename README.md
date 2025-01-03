# 20241_EE4348
1.Controller
Các file HTTP_Controller.py, CoAP_Controller.py và MQTT_Controller.py được dùng để tạo một controller tương ứng với mỗi giao thức.
Yêu cầu:
Môi trường: Python3
Thư viện: socket, tb_devices
File Controller.py được dùng để tạo một controller phân luồng thực hiện cả ba giao thức cùng một thời điểm.
Yêu cầu:
Môi trường: Python3
Thư viện, socket, tb_devices, threading
2.Thingsboard
File json là Rule chain được sử dụng trên server Thingsboard để thực hiện các luật điều khiển, lấy ngưỡng do người dùng đặt.
Vào Rule chain – Import – Set root rule chain.
Yêu cầu:
Tài khoản Thingsboard Community
Device ở Entities
Asset
3.Devices
Các folder MQTT, HTTP, CoAP, HTTPS được dùng để nạp chương trình vào vi xử lí
Yêu cầu
ESP32 WROOM 38-pin type C
Arduino IDE: coap-simple, ESPping

 
