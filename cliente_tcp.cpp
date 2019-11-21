#include <iostream>
#include <cstring>
#include <cstddef>
#include <cstring>
#include <string>
#include <sys/poll.h>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include "lib/config.hpp"
#include "lib/utils.hpp"


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

  while(true) {
    // poll(pfds, 2, -1);
    getline (std::cin, input_string);
    const char *option = input_string.c_str();

    if (input_string[0] == '1') {
      client_message[0] = config->ENQ;
      client_message[1] = '\0';
    }

    else if (input_string[0] == '3') {
      size_t pos = input_string.find(":");
      input_string.erase(0,pos+1);
      input_string.insert(0, 1, config->STX);
      strcpy(client_message, input_string.c_str());
    }

    std::cout << "sending" << "\n";
    int sending_state = send(my_socket, client_message, strlen(client_message), config->send_flags);
    if (sending_state == config->error_state) {
      print_error_and_exit("Send failed");
    }
    int received = recv(my_socket, server_response, sizeof(server_response), config->rec_flags);
    if (received <= 0) {
      print_error_and_exit("received with errors");
    }
    server_response[received] = '\0'; // TCP/IP Sockets in C: Practical Guide for Programmers.
    std::cout << server_response << "\n";

  }
  return 0;
}
