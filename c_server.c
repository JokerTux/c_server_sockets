#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#define PORT 1337
#define BUFFER 15120
#define MAX_FILE_NAME_SIZE 120
#define handle_error(msg) \
	do {perror(msg); exit(EXIT_FAILURE); } while (0)


int main(){
	char* f[BUFFER];
	char answ_i[2];
	char cli_msg[50];
	int answ_int = 0;
	FILE* f_out; 
	ssize_t msg_len = 0;
	ssize_t opt = 0;
	size_t len = 0;
	char file_name[50];
	ssize_t f_n = 0;
	char path[10] = "./uploads/"; 


	//LINUX OS only. 
	struct stat st = {0};	
	if(stat(path, &st) == -1){
		mkdir(path, 0700);
	}

	struct sockaddr_in localaddr, remoteaddr;
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	localaddr.sin_port = htons(PORT);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1){
		handle_error("socket");
	}

	int bind1 = bind(sock, (struct sockaddr *) &localaddr, sizeof(localaddr));
	if(bind1 != 0){
		handle_error("bind");
	}

	int listen2 = listen(sock, 10);
	if(listen2 != 0){
		handle_error("listen");
	}
	else{
		printf("Server listening...\n");
	}

	int socklen_t = sizeof(remoteaddr);
	while(1){ 
		memset(f,'\0' ,BUFFER);
		memset(answ_i, '\0', 2);
		memset(cli_msg, '\0', 50);	
		memset(file_name, '\0', 50);
	
		int sock_r = accept(sock, (struct sockaddr *) &remoteaddr, &socklen_t);
		if(sock_r == -1){
			handle_error("accept");
		}
		
		opt = recv(sock_r, answ_i, 2, 0);
	 	//scanf("%1s", answ_i);
		//strncat(P_answ_i, answ_i, 1);
		answ_int = answ_i[0] - '0';
		printf("%d\n", answ_int);
		//printf("file size = %zu", sizeof(f));
		if(opt == -1){
			close(sock_r);
			shutdown(sock_r, SHUT_RDWR);
			//handle_error("recv");
		}
		switch(answ_int){
			case 1:{
				//printf("%s\n", answ_i); 
				msg_len = recv(sock_r, cli_msg, 50, 0);
	        		//printf("%ld\n", msg_len);
				if(msg_len == -1){
					break;
				}
				else{	
					printf("MSG: %s\n", cli_msg);
					//printf("MSG len: %ld\n", strlen(cli_msg));
					memset(cli_msg, '\0', 50);
				}
				break;
			}
			case 2:{
				f_n = recv(sock_r, file_name, 50, 0);
				ssize_t l_file_path = (strlen(path) + strlen(file_name));
				char file_path[l_file_path];
				//printf("file_name = %ld, path = %ld, l_file_path = %ld\n", strlen(file_name), strlen(path), l_file_path);
				memset(file_path, '\0', l_file_path);
				strcpy(file_path, path);
				strcat(file_path, file_name);
				if(strlen(file_name) == 0){
					memset(file_path, '\0', l_file_path);
					break;
				}
				if(l_file_path >= MAX_FILE_NAME_SIZE){
					break;
				}

				printf("%s\n", file_path);

				printf("file name : %s\n", file_name);
				f_out = fopen(file_path, "wb");
				if(f_out == 0){
					break;
				}
				
				printf("file incomming !!!\n");

				int b = 0, tot = 0;
				while((b = recv(sock_r, f, BUFFER, 0)) > 0){
					if(b == -1){
						break;
					}
				//	printf("%d[B]\n", b);
					tot += b;

					ssize_t f_wri =	fwrite(f, 1, b, f_out);	
					if(f_wri == 0){
						break;
					}
				}
				printf("%d[B]\n", tot);			
				fclose(f_out);
				break;
			}

			case 3:{				
				shutdown(sock, SHUT_RDWR);	
				close(sock);
				printf("Good bye ! :)\n");
			 	return 0;	
			}
			default:{
				answ_int = 0;	
			}
		}

	}		
	shutdown(sock, SHUT_RDWR);	
	close(sock);
	return 0;
}
// Verify the input file name.
