#include <arpa/inet.h>
#include <sys/socket.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT      8080
#define FR_BUF_SZ 65495
#define BUF_SZ    65495

typedef struct _message
{
	char* res_ver;
	uint status;
	char* res_pharse;
	char** options;
	uint options_len;
}message_data;


ulong message_builder(message_data message, char* return_message) {	
	sprintf(return_message, "%s %d %s\r\n %s\r\n", \
	message.res_ver, message.status, message.res_pharse, message.options[message.options_len-1]);		
	
	ulong message_len = strlen(return_message);
	return message_len;
}

void router(int client_fd, const char* path)
{
	message_data m;
	m.res_ver = "HTTP/1.1";
	m.options = (char**)malloc(sizeof(char*)*5);
	char* message = (char*)malloc(sizeof(char)*60);	
	char res[FR_BUF_SZ] = {0, };
	int fd = 0;

	if (strcmp(path, "/main") == 0) {
		m.status = 200;
		m.res_pharse = "OK";
		m.options[0] = "Content-Type: text/html; charset=utf-8\r\n";
		m.options_len = 1;
		
		ulong len = message_builder(m, message);
		memcpy(res, message, len);		
		
		fd = open("main.html", O_RDONLY);		
		read(fd, res + strlen(message), FR_BUF_SZ);
	} else if (strcmp(path, "/favicon.ico") == 0) {
		m.status = 200;
		m.res_pharse = "OK";
		m.options[0] = "Content-Type: image/x-icon\r\n";
		m.options_len = 1;
		
		ulong len = message_builder(m, message);
		memcpy(res, message, len);
		
		fd = open("favicon.ico", O_RDONLY);
		off_t fsize = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);
		read(fd, res + strlen(message), fsize);
	} else {
		m.status = 404;
		m.res_pharse = "NOT FOUND";
		m.options[0] = "Content-Type: text/html; charset=utf-8\r\n";
		m.options_len = 1;

		ulong len = message_builder(m, message);
		memcpy(res, message, strlen(message));
	
		fd = open("notFound.html", O_RDONLY);
		read(fd, res + strlen(message), FR_BUF_SZ);
	}
	write(client_fd, res, sizeof(res));

	free(m.options);
	free(message);
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
