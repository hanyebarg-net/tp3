#include <string>
#include <netdb.h>
#define MAX_LEN 80
#define MAX_CLIENT 3 // 255

typedef struct addr_info {
  struct sockaddr_storage Source;
  socklen_t source_len = sizeof(Source);
}Addr_storage;

typedef struct addrinfo Addr;
void print_error_and_exit(std::string message);
Addr create_and_initialize_address();
Addr create_and_initialize_address_UDP(bool is_client);
char* invert_case(char *str, int num_bytes);
int create_socket(char *port, bool is_client, char *host);
