import time
from locust import Locust, events, task, Taskset
from socket import *


class TcpClient():
  def __init__(self):
    self.HOST = 'localhost'
    self.PORT = 8080
    self.BUFSIZE = 1024
    self.ADDR = (HOST, PORT)

  def wrapper(*args, **kwargs):
    
    start_time = time.time()
    c_sock = socket(AF_INET, SOCK_STREAM)
    
    try:
      c_sock.connect(self.ADDR)
      data = c_sock.recv(self.BUFSIZE);
    except Exception as e:
      total_time = int((time.time() - start_time) * 1000)
      events.request_failure.fire(response_time = total_time)
    else:
      total_time = int((time.time() - start_time) * 1000)
      events.request_success.fire(response_time = total_time)
    return wrapper 


class TcpLocust(Locust):
  def __init__(*args, **kwargs):
    super(TcpLocust, self).__init__(*args, **kwargs)
    self.client = TcpClient(self.host)


class ApiUser():
  host = "http://localhost:8080"
  min_wait = 1000
  max_wait = 2000

