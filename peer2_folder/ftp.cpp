#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <ctype.h>
#include <fstream>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>

using namespace std;

#define STR_VALUE(val) #val
#define STR(name) STR_VALUE(name)

#define PATH_LEN 256
#define MD5_LEN 32


typedef struct file{
	char name[1024];
	char time[1024];
	int size;
	char type[200];
	char filemd5[MD5_LEN+1];
}fs;

//Global Variables for client side
char complete_command[1000];
char command[100][100];
int command_count=0;

//Global Variable for server side
char server_send_data [1024],server_recv_data[1024];
char recv_complete_command[1024];
char recv_command[32][32];
int recv_command_count=0;
fs Server_FileStructure[1000];
int Server_file_count =0;
char global_time[100];


void parse_time(char time1[]){
	int i=0;
	char ans[100];
	char month[4];
	for(int i=4;i<7;i++)month[i-4]=time1[i];
	month[3]='\0';
	ans[4]='-';
	//printf("DEBUG month %s\n",month);
	if(strcmp(month,"Jan")==0){ans[5]='0';ans[6]='1';}
	if(strcmp(month,"Feb")==0){ans[5]='0';ans[6]='2';}
	if(strcmp(month,"Mar")==0){ans[5]='0';ans[6]='3';}
	if(strcmp(month,"Apr")==0){ans[5]='0';ans[6]='4';}
	if(strcmp(month,"May")==0){ans[5]='0';ans[6]='5';}
	if(strcmp(month,"Jun")==0){ans[5]='0';ans[6]='6';}
	if(strcmp(month,"Jul")==0){ans[5]='0';ans[6]='7';}
	if(strcmp(month,"Aug")==0){ans[5]='0';ans[6]='8';}
	if(strcmp(month,"Sep")==0){ans[5]='0';ans[6]='9';}
	if(strcmp(month,"Oct")==0){ans[5]='1';ans[6]='0';}
	if(strcmp(month,"Nov")==0){ans[5]='1';ans[6]='1';}
	if(strcmp(month,"Dec")==0){ans[5]='1';ans[6]='2';}

	if(time1[8]==' '){ans[8]='0';ans[9]=time1[9];}
	else{ans[8]=time1[8];ans[9]=time1[9];}
	ans[7]='-';
	ans[10]=' ';

	for(i=20;i<24;i++){ans[i-20]=time1[i];}

	for(i=11;i<19;i++){ans[i]=time1[i];}

	ans[19]='\0';
	//printf("DEBUG --");
	for(i=0;i<20;i++){
		//printf("%c",ans[i]);
		global_time[i]=ans[i];
	}
	//printf("\n");
	//printf("DEBUG %s -- %s\n",global_time,ans);
}

char *concat(char c1[],char c2[]){
	char *ans = (char *)malloc(sizeof(char)*100);
	for(int i=0;i<strlen(c1);i++)ans[i]=c1[i];
	ans[strlen(c1)]=' ';
	for(int i=strlen(c1)+1;i<strlen(c1)+strlen(c2)+1;i++)ans[i]=c2[i-strlen(c1)-1];
	ans[strlen(c1)+strlen(c2)+1]='\0';
	return ans;
}

int timecmp(char time1[],char time2[]){
	return strcmp(time1,time2);
}

		




int CalcFileMD5(char *file_name, char *md5_sum)
{
	#define MD5SUM_CMD_FMT "md5sum %." STR(PATH_LEN) "s 2>/dev/null"
	char cmd[PATH_LEN + sizeof (MD5SUM_CMD_FMT)];
	sprintf(cmd, MD5SUM_CMD_FMT, file_name);
	#undef MD5SUM_CMD_FMT

	FILE *p = popen(cmd, "r");
	if (p == NULL) return 0;

	int i, ch;
	for (i = 0; i < MD5_LEN && isxdigit(ch = fgetc(p)); i++) {
		*md5_sum++ = ch;
	}

	*md5_sum = '\0';
	pclose(p);
	return i == MD5_LEN;
}

void update_file_structure(){
	DIR *dp;
	struct dirent *ep;

	dp = opendir ("./");
	Server_file_count = 0;
	if (dp != NULL)
	{
		while (ep = readdir (dp)){
			strcpy(Server_FileStructure[Server_file_count].name,ep->d_name);
			
			struct stat st;
			stat(ep->d_name, &st);

			int size = st.st_size;
			char terminal_command[100];
			strcpy(terminal_command,concat("file ",Server_FileStructure[Server_file_count].name));
			system(concat(terminal_command,">fileType"));
			ifstream input;
			string line;
			input.open("fileType");
			getline(input,line);
			input.close();
			strcpy(Server_FileStructure[Server_file_count].type,line.c_str());
			Server_FileStructure[Server_file_count].size= size;
			strcpy(Server_FileStructure[Server_file_count].time,ctime(&st.st_mtime));
			CalcFileMD5(Server_FileStructure[Server_file_count].name,Server_FileStructure[Server_file_count].filemd5);
			Server_file_count++;
		}
		(void) closedir (dp);
	}
	else
		puts ("Couldn't open the directory.");
}

void parse(){
	char c;
	int count=0;

	for(int i=0;i<strlen(server_recv_data);i++)
		recv_complete_command[count++]=server_recv_data[i];

	recv_complete_command[count++]='\0';
	recv_command_count=0;

	int count2=0;
	for(int i=0;i<strlen(recv_complete_command);i++){
		if(recv_complete_command[i]==' '){
			recv_command[recv_command_count][count2++]='\0';
			recv_command_count++;
			count2=0;
			continue;
		}   
		recv_command[recv_command_count][count2]=recv_complete_command[i];
		count2++;
	}  
	recv_command[recv_command_count][count2++]='\0';
	recv_command_count++;
}


void scan_input(){
	char c;
	int count=0;
	printf(" $ ");
	scanf("%c",&c);
	while(c!='\n'){
		complete_command[count]=c;
		count++;
		scanf("%c",&c);
	}
	complete_command[count++]='\0';
	command_count=0;
	int count2=0;
	for(int i=0;i<strlen(complete_command);i++){
		if(complete_command[i]==' '){
			command[command_count][count2++]='\0';
			command_count++;
			count2=0;
			continue;
		}
		command[command_count][count2]=complete_command[i];
		count2++;
	}
	command[command_count][count2++]='\0';
	command_count++;
}

int client_code(int connect_port_no,char * type){
	fs FileStructure[1000];
	char md5[MD5_LEN + 1];
	char recv_md5[MD5_LEN + 1];

	int sock, bytes_recieved;  
	char send_data[1024],recv_data[1024];
	int recv_data_int;
	struct hostent *host;
	struct sockaddr_in server_addr;

	host = gethostbyname("127.0.0.1");
	if(strcmp(type,"tcp")==0){
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror("Socket");
			return 1;
		}
	}
	if(strcmp(type,"udp")==0){
		if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
			perror("Socket");
			return 1;
		}
	}

	server_addr.sin_family = AF_INET;     
	server_addr.sin_port = htons(connect_port_no);   
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(server_addr.sin_zero),8); 
	int sin_size = sizeof(struct sockaddr_in);
	socklen_t * lol = (socklen_t *) &sin_size;
	if(strcmp(type,"tcp")==0){
		if (connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1) {
			perror("Connect");
			return 2;
		}
		printf("\n CLIENT : Connected to Port No %d\n",connect_port_no);
	}
	scan_input();

	while(strcmp(command[0],"Exit")!=0 && strcmp(command[0],"q")!=0 && strcmp(command[0],"Q")!=0 && strcmp(command[0],"exit")!=0 ){
		if(strcmp(command[0],"help")==0){
			printf("Download <filename> : To download file from the remote host\n");
			printf("Ex: Download abc.mp3\n");
			printf("--------------------------\n");
			printf("upload allow: This will allow for other peers to upload files on my server\n");
			printf("Ex: upload allow\n");
			printf("--------------------------\n");
			printf("upload deny: This will deny for other peers to upload files on my server\n");
			printf("Ex: upload deny\n");
			printf("--------------------------\n");
			printf("upload <filename> : To upload file to the remote host\n");
			printf("Ex:upload abc.mp3\n");
			printf("--------------------------\n");
			printf("IndexGet ShortList <Starting Time Stamp> <Ending Time Stamp>: To Get the information of the files that are modified in the given time stamps\n");
			printf("Format of the time stamps : yyyy.mm.dd hh.mm.ss yyyy.mm.dd hh.mm.ss\n");
			printf("Ex: IndexGet ShortList 2013.05.12 11.00.15 2013.05.14 12.15.00\n");
			printf("--------------------------\n");
			printf("IndexGet LongList : To get the information of all the files from the remote host\n");
			printf("Ex: IndexGet LongList\n");
			printf("--------------------------\n");
			printf("IndexGet regEx : To get all the files matching the specified regular expression\n");
			printf("Ex: IndexGet regEx *mp3\n");
			printf("--------------------------\n");
			printf("fileHash Verify <filename>: to check the md5hash of a particular file\n");
			printf("Ex: fileHash Verify abc.mp3\n");
			printf("--------------------------\n");
			printf("fileHash checkAll: to check the md5hash of all the files in the remote host\n");
			printf("Ex: fileHash checkAll\n");
			printf("--------------------------\n");
		}
		if(strcmp(command[0],"Download")==0){
			if(command_count < 2){
				printf("arguments missing\n");
				scan_input();
				continue;
			}

			else{
				/*************** Sending Download Command Header **************************/	
				if(strcmp(type,"tcp")==0)
					send(sock,complete_command,strlen(complete_command),0);
				else
					sendto(sock,complete_command,strlen(complete_command),0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
				/********* recieved file Existence Header ******************************/
				if(strcmp (type,"tcp")==0)
					bytes_recieved = recv(sock,recv_data,1024,0);
				else
					bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,lol);
				recv_data[bytes_recieved]='\0';

				if(strcmp(recv_data,"file doesn't exist")!=0){
					/********** recieved MD5 packet *******************/
					if(strcmp(type,"tcp")==0)
						recv(sock,recv_data,MD5_LEN+1,0);
					else
						recvfrom(sock,recv_data,MD5_LEN+1,0,(struct sockaddr *)&server_addr,lol);
					strcpy(recv_md5,recv_data);
					/******* recieved first Packet and it's size*******************/
					if(strcmp(type,"tcp")==0)
						recv(sock,&recv_data_int,sizeof(recv_data_int),0);
					else
						recvfrom(sock,&recv_data_int,sizeof(recv_data_int),0,(struct sockaddr *)&server_addr,lol);
					if(strcmp(type,"tcp")==0)
						bytes_recieved = recv(sock,recv_data,1024,0);
					else
						bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,lol);
					/**************************************************************/

					FILE *fp;
					fp = fopen(command[1],"w");
					printf("<");
					while(strcmp(recv_data,"End of File")!=0){
						printf(".");
						for(int i=0;i<recv_data_int;i++)
							fprintf(fp,"%c",recv_data[i]);
						/********* recieving packet size************/
						if(strcmp(type,"tcp")==0)
							recv(sock,&recv_data_int,sizeof(int),0);
						else
							recvfrom(sock,&recv_data_int,sizeof(int),0,(struct sockaddr *)&server_addr,lol);
						/********* recievin packet data**************/
						if(strcmp(type,"tcp")==0)
							bytes_recieved = recv(sock,recv_data,1024,0);
						else
							bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,lol);
						recv_data[bytes_recieved]='\0';
					}
					printf(">\n");
					printf("checking for the md5sum\n");
					printf("md5sum of the file to be downloaded = %s\n",recv_md5);
					fclose(fp);
					if (!CalcFileMD5(command[1], md5)) {
						puts("Error occured in md5sum! :(                          [Fail]");
					} 
					else {
						printf("md5sum of the file recieved: %s                    [OK]\n", md5);
					}
					if(strcmp(md5,recv_md5)==0){
						printf("md5sum for the file matched                        [OK]\n");
						printf("File Download completed                            [OK]\n");
					}
					else
						printf("md5 check sum error                                 [Fail]\n");
				}
				else
					printf("No such file or directory found on the remote host              [Fail]\n");
			}
		}
		else if(strcmp(command[0],"IndexGet")==0){
			/******** Sending IndexGet command header *********************/
			if(strcmp(type,"tcp")==0)
				send(sock,complete_command,strlen(complete_command),0);
			else
				sendto(sock,complete_command,strlen(complete_command),0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
			/******** recieving file count packet **********************/
			if(strcmp(type,"tcp")==0)
				recv(sock,&recv_data_int,sizeof(recv_data_int),0);
			else
				recvfrom(sock,&recv_data_int,sizeof(recv_data_int),0,(struct sockaddr *)&server_addr,lol);
			int file_count = recv_data_int;
			for(int i=0;i<file_count;i++){
				/********* recieving File name packet ************/
				if(strcmp(type,"tcp")==0)
					bytes_recieved = recv(sock,recv_data,1024,0);
				else
					bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,lol);
				recv_data[bytes_recieved]='\0';

				strcpy(FileStructure[i].name,recv_data);

				/********* recieving File type packet ************/
				if(strcmp(type,"tcp")==0)
					bytes_recieved = recv(sock,recv_data,1024,0);
				else
					bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,lol);
				recv_data[bytes_recieved]='\0';
				strcpy(FileStructure[i].type,recv_data);
				/********* recieving File size packet ************/
				if(strcmp(type,"tcp")==0)
					recv(sock,&recv_data_int,sizeof(int),0);
				else
					recvfrom(sock,&recv_data_int,sizeof(int),0,(struct sockaddr *)&server_addr,lol);

				FileStructure[i].size = recv_data_int;

				/********* recieving Last Modified time packet ************/
				if(strcmp(type,"tcp")==0)
					bytes_recieved = recv(sock,recv_data,1024,0);
				else
					bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,lol);
				recv_data[bytes_recieved]='\0';
				strcpy(FileStructure[i].time,recv_data);

			}
			if(strcmp(command[1],"ShortList")==0){
				char time1[100];
				char time2[100];
				if(strlen(command[2])>0 && strlen(command[3])>0 && strlen(command[4])>0 && strlen(command[5])>0){
					strcpy(time1,concat(command[2],command[3]));
					strcpy(time2,concat(command[4],command[5]));
					for(int i=0;i<file_count;i++){
						/**** compare the time stamps and select the proper files to show ***/
						parse_time(FileStructure[i].time);
						if(timecmp(global_time,time1)>=0 && timecmp(time2,global_time)>=0){
							printf("\n-----------------------\n");
							printf("Name : %s\n --|size = %d\n --|type = %s\n --|time=%s\n",FileStructure[i].name,FileStructure[i].size,FileStructure[i].type,FileStructure[i].time);
						}
					}
					printf("\n-----------------------\n");
				}
				else
					printf("Error in timeStamps\n");
			}
			else if(strcmp(command[1],"LongList")==0){
				/************* Long Listing of recieved files *********/
				for(int i=0;i<file_count;i++){
					printf("\n-----------------------\n");
					printf("Name : %s\n --|size = %d\n --|type = %s\n --|time=%s\n",FileStructure[i].name,FileStructure[i].size,FileStructure[i].type,FileStructure[i].time);
				}
				printf("\n-----------------------\n");
			}
			else if(strcmp(command[1],"regEx")==0){

				if(strlen(command[1])>0){
					/*********** recieved special regEx packets *****/
					if(strcmp(type,"tcp")==0)
						bytes_recieved = recv(sock,recv_data,1024,0);
					else
						bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,lol);
					recv_data[bytes_recieved]='\0';
					while(strcmp(recv_data,"End of File")!=0){
						for(int i=0;i<file_count;i++){
							if(strcmp(recv_data,FileStructure[i].name)==0){
								printf("\n-----------------------\n");
								printf("Name : %s\n --|size = %d\n --|type = %s\n --|time=%s\n",FileStructure[i].name,FileStructure[i].size,FileStructure[i].type,FileStructure[i].time);
							}
						}
						if(strcmp(type,"tcp")==0)
							bytes_recieved = recv(sock,recv_data,1024,0);
						else
							bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,lol);
						recv_data[bytes_recieved]='\0';
					}
					printf("\n-----------------------\n");
				}
				else
					printf("Argument for regEx is Missing \n");
			}
			else
				printf("Arguments Missing\n");
		}
		/*************************************** fileHash *************************************************************************************************/
		else if(strcmp(command[0],"fileHash")==0){
			/******** sending fileHash command Header ****************************/
			if(strcmp(type,"tcp")==0)	
				send(sock,complete_command,strlen(complete_command),0);
			else
				sendto(sock,complete_command,strlen(complete_command),0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
			/******* recieved fileCount Packet **********************************/
			if(strcmp(type,"tcp")==0)
				recv(sock,&recv_data_int,sizeof(recv_data_int),0);
			else
				recvfrom(sock,&recv_data_int,sizeof(recv_data_int),0,(struct sockaddr *)&server_addr,lol);
			int file_count = recv_data_int;
			/*******************************************************************/
			for(int i=0;i<file_count;i++){
				/*** File Name Packet*******/
				if(strcmp(type,"tcp")==0)
					bytes_recieved = recv(sock,recv_data,1024,0);
				else
					bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,lol);
				recv_data[bytes_recieved]='\0';

				strcpy(FileStructure[i].name,recv_data);
				/*** File type Packet*******/

				if(strcmp(type,"tcp")==0)
					bytes_recieved = recv(sock,recv_data,1024,0);
				else
					bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,lol);
				recv_data[bytes_recieved]='\0';
				strcpy(FileStructure[i].type,recv_data);
				/*** File Size Packet*******/
				if(strcmp(type,"tcp")==0)
					recv(sock,&recv_data_int,sizeof(int),0);
				else
					recvfrom(sock,&recv_data_int,sizeof(int),0,(struct sockaddr *)&server_addr,lol);
				FileStructure[i].size = recv_data_int;
				/*** File time Packet*******/

				if(strcmp(type,"tcp")==0)
					bytes_recieved = recv(sock,recv_data,1024,0);
				else
					bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,lol);
				recv_data[bytes_recieved]='\0';

				strcpy(FileStructure[i].time,recv_data);
				/*** File md5Hash Packet*******/
				if(strcmp(type,"tcp")==0)
					recv(sock,FileStructure[i].filemd5,MD5_LEN+1,0);
				else
					recvfrom(sock,FileStructure[i].filemd5,MD5_LEN+1,0,(struct sockaddr *)&server_addr,lol);

			}
			if(strcmp(command[1],"Verify")==0){
				int i;
				for(i=0;i<file_count;i++){
					// Filtering of a particular file to verify
					if(strcmp(command[2],FileStructure[i].name)==0){
						printf("\n-----------------------\n");
						printf("Name : %s\n --|size = %d\n --|type = %s\n --|time=%s --|md5=%s\n",FileStructure[i].name,FileStructure[i].size,FileStructure[i].type,FileStructure[i].time,FileStructure[i].filemd5);
						break;
					}
				}
				if(i==file_count)printf("No Such File on Remote Host\n");
				else printf("\n-----------------------\n");
			}
			else if(strcmp(command[1],"checkAll")==0){
				/*** Sending all files for checkall */
				for(int i=0;i<file_count;i++){					
					printf("\n-----------------------\n");
					printf("Name : %s\n --|size = %d\n --|type = %s\n --|time=%s --|md5=%s\n",FileStructure[i].name,FileStructure[i].size,FileStructure[i].type,FileStructure[i].time,FileStructure[i].filemd5);
				}
				printf("\n-----------------------\n");
			}
			else
				printf("Argument Missing\n");
		}
		else if(strcmp(command[0],"upload")==0){
			if(command_count<2)
				printf("Arguments missing");
			else {

				if(strcmp(command[1],"allow")==0 || strcmp(command[1],"deny")==0){
					FILE * upload_file = fopen("upload_command","w");
					fprintf(upload_file,"%s",command[1]);
					fclose(upload_file);
				}
				else{
					ifstream ifile(command[1]);
					if(ifile){
						int garbage_int;
						/********************** sending upload command header **************************/
						if(strcmp(type,"tcp")==0)
							send(sock,complete_command,1024,0);
						else
							sendto(sock,complete_command,1024,0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));

						/*********recieving permission header ********************************************/
						if(strcmp(type,"tcp")==0)
							bytes_recieved = recv(sock,recv_data,1024,0);
						else
							bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,lol);
						recv_data[bytes_recieved]='\0';

						if(strcmp(recv_data,"allowed")==0){
							printf("Allowed command recieved\n");
							if (!CalcFileMD5(command[1], md5)) {
								puts("Error occured in md5sum! :(");
							} 
							else {
								printf("\nmd5sum: %s\n", md5);
							}

							char ch;
							FILE *fp;
							fp = fopen(command[1],"r");
							int count;
							/** Sending FileMD5 packet ****/
							if(strcmp(type,"tcp")==0)
								send(sock,md5,MD5_LEN+1,0);
							else
								sendto(sock,md5,MD5_LEN+1,0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));

							while(fscanf(fp,"%c",&ch)!=EOF){
								count=0;
								server_send_data[count++]=ch;
								while(count<1024 && fscanf(fp,"%c",&ch)!=EOF){
									server_send_data[count++]=ch;
								}
								/*** sending packet size header *****/
								if(strcmp(type,"tcp")==0)
									send(sock,&count,sizeof(int),0);
								else
									sendto(sock,&count,sizeof(int),0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
								/*** sending packet data *************/
								if(strcmp(type,"tcp")==0)
									send(sock,server_send_data,1024,0);
								else
									sendto(sock,server_send_data,1024,0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
							}
							/******** sending End Of File header ********************************************************/
							if(strcmp(type,"tcp")==0)
								send(sock,&garbage_int,sizeof(int),0);
							else
								sendto(sock,&garbage_int,sizeof(int),0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
							if(strcmp(type,"tcp")==0)
								send(sock,"End of File",1024,0);
							else
								sendto(sock,"End of File",1024,0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
							/**********************************************************************************************/
						}
						else
							printf("Upload denied\n");
					}
					else
						printf("No such file or directory");
				}
			}
		}
		else{
			if(strcmp(complete_command,"")!=0)
				printf("INVALID COMMAND\n",complete_command);
		}
		scan_input();
	}

	return 0;
}

int server_code(int server_port_no,char *type){
	int garbage_int=0;
	char md5[MD5_LEN + 1];
	char recv_md5[MD5_LEN + 1];

	int sock, connected, bytes_recieved ;  

	struct sockaddr_in server_addr,client_addr;    
	int sin_size;

	if(strcmp(type,"tcp")==0){

		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror("Socket");
			return 1;
		}
	}
	if(strcmp(type,"udp")==0){

		if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
			perror("Socket");
			return 1;
		}
	}


	server_addr.sin_family = AF_INET;         
	server_addr.sin_port = htons(server_port_no);     
	server_addr.sin_addr.s_addr = INADDR_ANY; 
	bzero(&(server_addr.sin_zero),8); 

	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))
			== -1) {
		perror("Unable to bind");
		return 2;
	}

	if(strcmp(type,"tcp")==0){
		if (listen(sock, 5) == -1) {
			perror("Listen");
			return 3;
		}
	}
	if(strcmp(type,"tcp")==0)
		printf("\nTCPServer Waiting for client on port %d\n $ ",server_port_no);
	if(strcmp(type,"udp")==0)
		printf("\nUDPServer Waiting for client on port %d\n $ ",server_port_no);

	fflush(stdout);


	while(1){  

		sin_size = sizeof(struct sockaddr_in);
		socklen_t * lol = (socklen_t *) &sin_size;
		if(strcmp(type,"tcp")==0){
			connected = accept(sock, (struct sockaddr *)&client_addr,lol);
			printf("\n I got a connection from (%s , %d)\n $ ", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		}


		while (1){
			/************************************ Packet header recieving ********************************************/
			if(strcmp(type,"tcp")==0)
				bytes_recieved = recv(connected,server_recv_data,1024,0);
			if(strcmp(type,"udp")==0)
				bytes_recieved = recvfrom(sock,server_recv_data,1024,0,(struct sockaddr *)&client_addr, lol);
			server_recv_data[bytes_recieved] = '\0';

			/*** packet parsing **/
			parse();
			/********************/

			if(bytes_recieved==0){
				printf("\nConnection closed by remote host\n $ ");
				close(connected);
				break;
			}
			if(strcmp(server_recv_data , "Exit") == 0 || strcmp(server_recv_data , "exit") == 0){
				printf("Connection closed by remote host\n $ ");

				close(connected);
				break;
			}
			else{
				printf("\n RECIEVED DATA = %s \n $ " , recv_complete_command);

				if(strcmp(recv_command[0],"Download")==0){
					ifstream ifile(recv_command[1]);
					if(ifile){
						/***************** file existence header *******************/
						if(strcmp(type,"tcp")==0)
							send(connected,"file exists",1024,0);
						else
							sendto(sock,"file exists",1024,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));

						if (!CalcFileMD5(recv_command[1], md5)) {
							puts("Error occured in md5sum! :(");
						} 
						else {
							printf("\nmd5sum: %s\n", md5);
						}

						char ch;
						FILE *fp;
						fp = fopen(recv_command[1],"r");
						int count;
						if(strcmp(type,"tcp")==0)
							send(connected,md5,MD5_LEN+1,0);
						else
							sendto(sock,md5,MD5_LEN+1,0,(struct sockaddr *)&client_addr,sizeof(struct sockaddr));
						while(fscanf(fp,"%c",&ch)!=EOF){
							count=0;
							server_send_data[count++]=ch;
							while(count<1024 && fscanf(fp,"%c",&ch)!=EOF){
								server_send_data[count++]=ch;
							}
							/**** Packet containing header Size ********/
							if(strcmp(type,"tcp")==0)
								send(connected,&count,sizeof(int),0);
							else
								sendto(sock,&count,sizeof(int),0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));

							/********************************************/

							/***** Packet data (1024B chunks)************/
							if(strcmp(type,"tcp")==0)
								send(connected,server_send_data,1024,0);
							else
								sendto(sock,server_send_data,1024,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
							/********************************************/

						}
						/******* Sending header for End Of File **********/
						if(strcmp(type,"tcp")==0)
							send(connected,&garbage_int,sizeof(int),0);
						else
							sendto(sock,&garbage_int,sizeof(int),0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));

						if(strcmp(type,"tcp")==0)
							send(connected,"End of File",1024,0);
						else
							sendto(sock,"End of File",1024,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						/*************************************************/
					}
					else{
						/********** File Existence Header ****************/
						/*if(strcmp(type,"tcp")==0)
							send(connected,&garbage_int,sizeof(int),0);
						else
							sendto(sock,&garbage_int,sizeof(int),0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));*/
						if(strcmp(type,"tcp")==0)
							send(connected,"file doesn't exist",1024,0);
						else
							sendto(sock,"file doesn't exist",1024,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						/************************************************/
					}
				}
				if(strcmp(recv_command[0],"IndexGet")==0){
					update_file_structure();
					/*********** Sending File Count ************/
					if(strcmp(type,"tcp")==0)
						send(connected,&Server_file_count,sizeof(int),0);
					else
						sendto(sock,&Server_file_count,sizeof(int),0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
					/*******************************************/

					for(int i=0;i<Server_file_count;i++){
						/*************** Sending File Structure *********************/

						/***** File Name Packet *****/
						if(strcmp(type,"tcp")==0)
							send(connected,Server_FileStructure[i].name,1024,0);
						else
							sendto(sock,Server_FileStructure[i].name,1024,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						/*****File Type Packet *****/
						if(strcmp(type,"tcp")==0)
							send(connected,Server_FileStructure[i].type,1024,0);
						else
							sendto(sock,Server_FileStructure[i].type,1024,0,(struct sockaddr *)&client_addr,sizeof(struct sockaddr));
						/****File Size Packet *****/
						if(strcmp(type,"tcp")==0)
							send(connected,&Server_FileStructure[i].size,sizeof(int),0);
						else
							sendto(sock,&Server_FileStructure[i].size,sizeof(int),0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						/***Last Modified Time Packet ***/
						if(strcmp(type,"tcp")==0)
							send(connected,Server_FileStructure[i].time,1024,0);
						else
							sendto(sock,Server_FileStructure[i].time,1024,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						
					}
					if(recv_command_count>1){
						if(strcmp(recv_command[1],"regEx")==0){
							/**** Special regEx Packets ****/
							char terminal_command[100];
							strcpy (terminal_command,concat("ls ",recv_command[2]));
							system(concat(terminal_command,">out"));
							FILE * out_file = fopen("out","r");
							while(fscanf(out_file,"%s",&server_send_data)!=EOF){
								if(strcmp(type,"tcp")==0)
									send(connected,server_send_data,1024,0);
								else
									sendto(sock,server_send_data,1024,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
							}
							if(strcmp(type,"tcp")==0)
								send(connected,"End of File\0",1024,0);
							else
								sendto(sock,"End of File\0",1024,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}
					}
				}

				/*************************** File Hash *************************************************************************************/


				if(strcmp(recv_command[0],"fileHash")==0){
					update_file_structure();
					/*********** Header For File Hash ***********/
					if(strcmp(type,"tcp")==0)
						send(connected,&Server_file_count,sizeof(int),0);
					else
						sendto(sock,&Server_file_count,sizeof(int),0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));

					for(int i=0;i<Server_file_count;i++){
						/***** File Name Packet ***/
						if(strcmp(type,"tcp")==0)
							send(connected,Server_FileStructure[i].name,1024,0);
						else
							sendto(sock,Server_FileStructure[i].name,1024,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));

						/***** File type Packet ***/
						if(strcmp(type,"tcp")==0)
							send(connected,Server_FileStructure[i].type,1024,0);
						else
							sendto(sock,Server_FileStructure[i].type,1024,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						/****** File Size Packet ***/
						if(strcmp(type,"tcp")==0)
							send(connected,&Server_FileStructure[i].size,sizeof(int),0);
						else
							sendto(sock,&Server_FileStructure[i].size,sizeof(int),0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						/***** Last Modified Time Packet*/
						if(strcmp(type,"tcp")==0)
							send(connected,Server_FileStructure[i].time,1024,0);
						else
							sendto(sock,Server_FileStructure[i].time,1024,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						/**** MD5sum Packet ***********/
						if(strcmp(type,"tcp")==0)
							send(connected,Server_FileStructure[i].filemd5,MD5_LEN+1,0);
						else
							sendto(sock,Server_FileStructure[i].filemd5,MD5_LEN+1,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
					}	
				}
				if(strcmp(recv_command[0],"upload")==0){
					/********* Checking For permission ****/
					FILE * upload_file = fopen("upload_command","r");
					char permission[100];
					fscanf(upload_file,"%s",permission);
					fclose(upload_file);
					if(strcmp(permission,"deny")==0){
						if(strcmp(type,"tcp")==0)
							send(connected,"denied",1024,0);
						else
							sendto(sock,"denied",1024,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
					}

					else{
						/******* Sending Permission Header ********/
						if(strcmp(type,"tcp")==0)
							send(connected,"allowed",1024,0);
						else
							sendto(sock,"allowed",1024,0,(struct sockaddr *)&client_addr,sizeof(struct sockaddr));

						/****** Recieving MD5 packet *************/
						if(strcmp(type,"tcp")==0)
							recv(connected,server_recv_data,MD5_LEN+1,0);
						if(strcmp(type,"udp")==0)
							recvfrom(sock,server_recv_data,MD5_LEN+1,0,(struct sockaddr *)&client_addr, lol);
						strcpy(recv_md5,server_recv_data);

						/********* receiving Packet Size Header ****/
						int server_recv_data_int;
						if(strcmp(type,"tcp")==0)
							recv(connected,&server_recv_data_int,sizeof(int),0);
						if(strcmp(type,"udp")==0)
							recvfrom(sock,&server_recv_data_int,sizeof(int),0,(struct sockaddr *)&client_addr,lol);

						/********** recieving first packet **********/
						if(strcmp(type,"tcp")==0)
							bytes_recieved = recv(connected,server_recv_data,1024,0);
						if(strcmp(type,"udp")==0)
							bytes_recieved = recvfrom(sock,server_recv_data,1024,0,(struct sockaddr *)&client_addr, lol);
						server_recv_data[bytes_recieved] = '\0';


						FILE *fp;
						fp = fopen(recv_command[1],"w");
						printf("<");
						while(strcmp(server_recv_data,"End of File")!=0){
							printf(".");//showing progress
							for(int i=0;i<server_recv_data_int;i++)
								fprintf(fp,"%c",server_recv_data[i]);
							/******* File Size Header ************/
							if(strcmp(type,"tcp")==0)
								recv(connected,&server_recv_data_int,sizeof(server_recv_data_int),0);
							if(strcmp(type,"udp")==0)
								recvfrom(sock,&server_recv_data_int,sizeof(server_recv_data_int),0,(struct sockaddr *)&client_addr, lol);
							
							/****** Data Packet ****************/
							if(strcmp(type,"tcp")==0)
								bytes_recieved = recv(connected,server_recv_data,1024,0);
							if(strcmp(type,"udp")==0)
								bytes_recieved = recvfrom(sock,server_recv_data,1024,0,(struct sockaddr *)&client_addr, lol);
							server_recv_data[bytes_recieved] = '\0';

						}
						printf(">\n");
						printf("checking for the md5sum\n");
						printf("md5sum of the file uploaded on me = %s\n",recv_md5);
						fclose(fp);
						if (!CalcFileMD5(recv_command[1], md5)) {
							puts("Error occured in md5sum! :(                          [Fail]");
						} 
						else {
							printf("md5sum of the file recieved: %s                    [OK]\n", md5);
						}
						if(strcmp(md5,recv_md5)==0){
							printf("md5sum for the file matched                        [OK]\n");
							printf("File upload completed                            [OK]\n");
						}
						else
							printf("md5 check sum error                                 [Fail]\n");

					}
				}
			}
		}
		fflush(stdout);
	}
	close(sock);
	return 0;
}

int main(){
	FILE *upload_file;
	upload_file = fopen("upload_command","w");
	fprintf(upload_file,"allow");
	fclose(upload_file);
	int server_port_no;
	int connect_port_no;
	char *type = (char *)malloc(sizeof(char) * 100);
	printf("Port number on which you want to listen(>1024): ");
	scanf("%d",&server_port_no);
	printf("Give port no to which you want to send the data (>1024): ");
	scanf("%d",&connect_port_no);
	printf("Type of transfer protocol(tcp/udp): ");
	scanf("%s",type);
	printf("Type 'help' for help\n");
	pid_t pid;
	pid=fork();
	if(pid==-1){
		printf("Error in creating Fork\n");
		exit(0);
	}
	if (pid==0) // Child process
	{
		server_code(server_port_no,type);
	}
	else // parent process 
	{
		while(client_code(connect_port_no,type)>0){
			sleep(1);
		}
	}
	kill(pid,SIGQUIT);
	return 0;
}
