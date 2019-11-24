all:
	 g++ -pthread -g cliente_tcp.cpp lib/*.cpp -o cliente
	 g++ --std=c++17 -g servidor_tcp.cpp lib/*.cpp -o servidor
