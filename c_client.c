#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <resolv.h>
#include <linux/in.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <libgen.h>
#include <time.h>
#define PORT 1337
#define BUFFER 15120
#define MAX_FILE_SIZE 2147483640
#define MAX_FILE_NAME_LEN 120

#define handle_error(msg) \
	do {perror(msg); exit(EXIT_FAILURE); } while (0)


bool check(char answ[]){
	for(int i = 0; i < strlen(answ); i++){
		if(isdigit(answ[i]) == 0){
			printf("failure, answer needs to be an integer \n");
			return false;
		}
		return true;
		}
}


int file_sz(FILE *f){
	int pos = ftell(f);
	int size = 0;

	printf("Warning text files may break the program.\n");
	int f_chk = fseek(f, 0L, SEEK_END);
	if(f_chk == -1){
		handle_error("reading file");
	}
	size = ftell(f);
	//Return to 0 offset
	fseek(f, 0, SEEK_SET);
	if(size >= MAX_FILE_SIZE){
		printf("File is to large : %d[B], max : %d[B]\n", size, MAX_FILE_SIZE); 
		exit(1);
	}	
	return size;
}


int main(int argc, char **argv){
	char cli_msg[50];
	char *P_cli_msg;
	char answ[2];
	char *P_answ_s;
	char answ_s[2];
	int answ_m = 0, sz = 0, connected = 0;
	FILE *f;
	char c_sz[11];
	char s_buffer[BUFFER];
	char f_f[MAX_FILE_NAME_LEN];
	char *f_n_s;

	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serveraddr.sin_port = htons(PORT);
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1){
		handle_error("socket");
	}
	//printf("%d \n", sock);
	
	int serveraddr_l = sizeof(serveraddr);
	if (connected = connect(sock, (struct sockaddr *) &serveraddr, serveraddr_l) != 0){
		handle_error("connect");
	}
	else{
		printf("connected !!!!\n");
		printf("1. Send message\n2. Send file\n3. Shutdown server\nSelect :");
		scanf("%2s", answ);
		if(check(answ) == 1){
			answ_m = atoi(answ);
		}
			
		printf("%d\n", answ_m);
		switch(answ_m){
			case 1:
				answ_s[0] = '1';
				P_answ_s = answ_s;
				size_t msg_s = send(sock, P_answ_s, 1, 0);
				if(msg_s == -1){
					shutdown(sock, SHUT_RDWR);
					close(sock);
					handle_error("send msg");
				}		
					
				printf("Write your msg (max 48 char):");
				getchar();
					
				if(fgets(cli_msg, 48, stdin)== NULL){
					if(feof(stdin)){
						exit(1);
					}
					else{
						shutdown(sock, SHUT_RDWR);
						close(sock);
						handle_error("Error while reading from stdin");
					}
				}
				else if(strchr(cli_msg, '\n') == NULL){
 					int c;
					while((c = getc(stdin)) != '\n' && c != EOF){
						shutdown(sock, SHUT_RDWR);
						close(sock);
	 					handle_error("stderr, Input too long");
						//exit(1);
					}
				}	
				else{
					printf("Msg size :%ld\n", strlen(cli_msg));
					P_cli_msg = cli_msg;
					printf("cli_msg :%s, P_cli_msg :%s", cli_msg, P_cli_msg);
					size_t send_msg = send(sock, P_cli_msg, strlen(P_cli_msg), 0);
					if(send_msg == -1){
						shutdown(sock, SHUT_RDWR);
						close(sock);
						handle_error("send msg");
					}		
				}
					
				break;

			case 2:
				answ_s[0] = '2';
				P_answ_s = answ_s;
				ssize_t f_send = send(sock, P_answ_s, 1, 0);
				if(f_send == -1){
					shutdown(sock, SHUT_RDWR);
					close(sock);
					handle_error("send file buffer");
				}
				//int w8 = 1;		
				//wait(&w8);	
				printf("File path to file :");
				scanf("%s", f_f);					
				if(strlen(f_f) >= MAX_FILE_NAME_LEN){
					shutdown(sock, SHUT_RDWR);
					close(sock);
					handle_error("File path is to long max. 120 chars.");
				}
			       	
				f = fopen(f_f, "rb");
				if(f == NULL){
					shutdown(sock, SHUT_RDWR);
					close(sock);
					handle_error("fopen rb");
				}
				
				//basename works only on linux systems.
				char *P_file_name = basename(f_f);
				ssize_t f_n_b = send(sock, P_file_name, strlen(P_file_name), 0);
				printf("%ld\n", strlen(P_file_name));
				//printf("%s\n", P_file_name);
				sleep(0.2);// otherwise filename buffer will append some of the first bytes from the file buffer.
				sz = file_sz(f);
				if(sz > 1){
					int b = 0, tot = 0;
					while((b = fread(s_buffer, 1, BUFFER, f)) > 0){
						if(b == 0){
							handle_error("fread");
						}
						size_t b_send = send(sock, s_buffer, b, 0);
						if(b_send == -1){
							shutdown(sock, SHUT_RDWR);
							close(sock);
							handle_error("send file buffer");
						}		
						tot += b;
					}
						//printf("%ld\n", b_send);
				fclose(f);
				}
				printf("File has been uploaded\n");
				break;
			
			case 3:
				answ_s[0] = '3';
				P_answ_s = answ_s;
				size_t close_sv = send(sock, P_answ_s, 1, 0);
				if(close_sv == -1){
					shutdown(sock, SHUT_RDWR);
					close(sock);
					handle_error("send close server");
				}
				break;
		}
					
	
        }	
	shutdown(sock, SHUT_RDWR);
	close(sock);
	return 0;
}
