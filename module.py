#!/usr/local/bin/python3
import socket

class modx51:
	def register(self, bits):
		str="0 register 0x{0:x}".format(bits)
		print(str)
		self.s.sendto(str.encode('ascii'), self.remote)
		buff=self.s.recvfrom(80)
		print(buff)


	def __init__(self, adress, port, local_port=32000):
		self.remote=(adress, port)
		self.s=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.s.bind(('', local_port))
		self.port=local_port



try:
	module=modx51("127.0.0.1", 8000, 32001)
	module.register(0x15)

except (KeyboardInterrupt, SystemExit):
	raise
except:
	print("error while registering module")


