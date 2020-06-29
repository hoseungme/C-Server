#include <arpa/inet.h>
#include <sys/socket.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT      8080
#define FR_BUF_SZ 4096
#define BUF_SZ    65495

char not_found_message[] =
	"HTTP/1.1 404 NOT FOUND\r\n"
	"Content-Type: text/html\r\n"
	"\r\n"
	"not found";

char ok_message[] =
	"HTTP/1.1 200 OK\r\n"
	"Content-Type: text/html; charset=utf-8\r\n"
	"\r\n";

void router(int client_fd, const char* path)
{
	char res[FR_BUF_SZ] = {0, };
	if (strcmp(path, "/main") == 0) {
		memcpy(res, ok_message, sizeof(ok_message));
		int fd = open("main.html", O_RDONLY);
		/* overwrite last '\0' byte of string to concatnate strings */
		read(fd, res + sizeof(ok_message) - 1, FR_BUF_SZ);
	} else {
		memcpy(res, not_found_message, sizeof(not_found_message));
	}

	write(client_fd, res, strlen(res));
}

int main(int argc, char const *argv[])
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	int option = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	if (-1 == bind(server_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr))) {
		perror("bind error");
		exit(1);
	}

	if (-1 == listen(server_fd, SOMAXCONN)) {
		perror("listen error");
		exit(1);
	}

	struct sockaddr_in client_addr;
	uint client_addr_size;
	uint client_fd;

	char req[BUF_SZ];
	while (1) {
		client_addr_size = sizeof(client_addr);
		client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_size);

		read(client_fd, req, BUF_SZ);

		char *start_line = strtok(req, "\n");
		printf("%s\n", start_line);

		char *method = strtok(start_line, " ");
		char *path = strtok(NULL, " ");
		printf("%s\n", path);

		router(client_fd, path);
		close(client_fd);
	}

	return 0;
}
