#include <algorithm>
#include <iostream>
#include <cstring>
#include <map>
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

  std::cout << config->ACK << "\n";
  std::cout << user[0] << "\n";

  int sending_state = send(my_socket, client_message, strlen(client_message), config->send_flags);
  if (sending_state == config->error_state) {
    print_error_and_exit("Send failed");
  }

  std::cout << "Opções:" << "\n";
  std::cout << "1 (listar usuários)" << "\n";
  std::cout << "2 destino->texto (msg privada)" << "\n";
  std::cout << "3 texto (broadcast)" << "\n";

  while(true) {
    getline (std::cin, input_string);
    const char *option = input_string.c_str();

    if (option[0] == '1') {
      client_message[0] = config->ENQ;
      std::copy(
        input_string.begin(),
        input_string.end(),
        client_message + 1
        );
      client_message[user_size - 1] = '\0';
    }

    int sending_state = send(my_socket, client_message, strlen(client_message), config->send_flags);
    if (sending_state == config->error_state) {
      print_error_and_exit("Send failed");
    }

    int total_bytes = 0;
    while (total_bytes < sizeof(client_message)) { // TCP/IP Sockets in C: Practical Guide for Programmers.

      int received = recv(my_socket, server_response, sizeof(server_response), config->rec_flags);
      if (received <= 0) {
        print_error_and_exit("received with errors");
      }
      total_bytes += received;
      server_response[total_bytes] = '\0'; // TCP/IP Sockets in C: Practical Guide for Programmers.
      std::cout << server_response << "\n";
      break;
    }
  }
  return 0;
}
