#include <iostream>
#include <cstring>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <string>
#include <sys/poll.h>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include "lib/config.hpp"
#include "lib/utils.hpp"


int GetMessage(int sock)
{
	char buffer[1024];
	int recvSize;

	while(1)
	{
		recvSize = recv(sock, buffer, 1024, MSG_NOSIGNAL);
		if (recvSize < 0)
		{
      shutdown(sock, SHUT_RDWR);
      close(sock);

      throw std::runtime_error("Server connection refused");
		}

		if (recvSize > 0){      
      std::cout << buffer << std::endl;
    }
	}

	return 0;
}

int SendInput(int sock, const std::string & userName)
{
	std::string buffer;  
  Config *config  = new Config();

	while(1)
	{
		buffer = "";

		std::cin >> buffer;


    if (buffer[0] == '3') {
      buffer.erase(buffer.begin(),buffer.begin()+2);
      buffer.insert(0, 1, config->STX);
    }    
    else if (buffer[0] == '1') {
      std::cout << "one!!" << "\n";
      buffer = config->ENQ + '\0';
    }
		send(sock, buffer.c_str(), 1024, MSG_NOSIGNAL);
	}
	return 0;
}


int main(int argc, char *argv[]) {
  Config *config  = new Config();
  const std::size_t user_max_size = 82;

  char *ip_addr = argv[1];
  char *port = argv[2];
  char server_response[MAX_LEN];
  char client_message[MAX_LEN];
  char user[user_max_size];
  std::string input_string;

  // config options found at http://man7.org/linux/man-pages/man2/socket.2.html
  int my_socket = create_socket(port, true, ip_addr);
  if (my_socket == config->error_state) {
    print_error_and_exit("connected socket creation failed");
  }

  std::cout << "Digite o nome de usuário" << "\n";
  getline (std::cin, input_string);

  std::size_t user_size = 1 + input_string.size() + 1;

  user[0] = config->ACK;
  std::copy(
    input_string.begin(),
    input_string.end(),
    user + 1
    );
  user[user_size - 1] = '\0';

  std::cout << "sending" << "\n";
  int sending_state = send(my_socket, user, strlen(user), config->send_flags);
  if (sending_state == config->error_state) {
    print_error_and_exit("Send failed");
  }

  std::cout << "Opções:" << "\n";
  std::cout << "1 (listar usuários)" << "\n";
  std::cout << "2;usuarioX;msg (msg privada)" << "\n";
  std::cout << "3;msg (broadcast)" << "\n";

  std::thread reading(GetMessage, std::ref(my_socket));
  std::thread sending(SendInput, std::ref(my_socket), std::ref(input_string));

  reading.join();
  sending.join();

  shutdown(my_socket, SHUT_RDWR);
  close(my_socket);
  return 0;
}
