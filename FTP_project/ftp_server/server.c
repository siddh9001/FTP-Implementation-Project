#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
/*for getting file size using stat()*/
#include<sys/stat.h>
 
/*for sendfile()*/
#include<sys/sendfile.h>
 
/*for O_RDONLY*/
#include<fcntl.h>
 
int main(int argc,char *argv[])
{
	struct sockaddr_in server, client;
	struct stat obj;
	int sock1, sock2;
	char ipaddr[15], buf[100], command[5], filename[20];
	int k, i, size, len, c, port;
	int filehandle;

	sock1 = socket(AF_INET, SOCK_STREAM, 0);

	if(sock1 == -1)
	{
		printf("Socket creation failed\n");
		exit(1);
	}

	bzero(&server, sizeof(server));
	printf("Enter the port number: ");
	scanf("%d",&port);
	printf("\nEnter the ip address in which server will listen request: ");
	scanf("%s",ipaddr);

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(ipaddr);

	k = bind(sock1,(struct sockaddr*)&server,sizeof(server));
	if(k == -1)
	{
		printf("\nBinding error\n");
		exit(1);
	}
	k = listen(sock1,1);
	if(k == -1)
	{
		printf("\nListen failed\n");
		exit(1);
	}

	len = sizeof(client);
	sock2 = accept(sock1,(struct sockaddr*)&client, &len);
	if(sock2 < 0)
	{
		printf("\nAccept Failed!..\n");
		exit(1);
	}
	else
	{
		printf("\nRequest Accepted..\n");
	}

	i = 1;
	while(1)
	{
		recv(sock2, buf, 100, 0);
		sscanf(buf, "%s", command);//scans the mentioned format specifier value from buff(startting from 0) and store it in command
		if(!strcmp(command, "ls"))
		{
			system("ls >temps.txt");
			i = 0;
			stat("temps.txt",&obj);//stat function takes all the properties of given file in obj(instance to a structure).
			size = obj.st_size;
			send(sock2, &size, sizeof(int),0);
			filehandle = open("temps.txt", O_RDONLY);
			sendfile(sock2,filehandle,NULL,size);
		}
		else if(!strcmp(command,"get"))
		{
			sscanf(buf, "%s%s", filename, filename);
			stat(filename, &obj);
			filehandle = open(filename, O_RDONLY);
			size = obj.st_size;
			if(filehandle == -1)
				size = 0;
			send(sock2, &size, sizeof(int), 0);
			if(size)
				sendfile(sock2, filehandle, NULL, size);
			      
		}
		else if(!strcmp(command, "put"))
	        {
			 int c = 0, len;
			 char *f;
			 sscanf(buf+strlen(command), "%s", filename);
			 recv(sock2, &size, sizeof(int), 0);
			 i = 1;
			 while(1)
			 {
				//usage of O_CREAT | O_EXCL will make sure that if file already exist then open fails.
				//if file does not exist then it will create a new regular file
				//0666 is an octal number, i.e. every one of the 6's corresponds to three permission bits
				// 6 = rw
				// 7 = rwx
				// first three bits for owner permission, next three bits for group permission and next is for the world the first digit - 
				//represents that is file or directory. (0 - file, d - directory) here we used 0 means file
				filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666); 
				
				if(filehandle == -1)
				{
					//sprintf function copy the mentioned format specifier type value to given file
					sprintf(filename + strlen(filename), "%d", i);
				}
				else
					break;
			}

			f = malloc(size);
			recv(sock2, f, size, 0);
			c = write(filehandle, f, size);
			close(filehandle);
			send(sock2, &c, sizeof(int), 0);
	        }
		else if(!strcmp(command, "pwd"))
		{
			system("pwd >temp.txt");
			i = 0;
			FILE*f = fopen("temp.txt","r");
			while(!feof(f))
				buf[i++] = fgetc(f);
			buf[i-1] = '\0';
			fclose(f);
			send(sock2, buf, 100, 0);
		}
		else if(!strcmp(command, "cd"))
		{
			if(chdir(buf+3) == 0)
				c = 1;
			else
				c = 0;
			send(sock2, &c, sizeof(int), 0);
		}
		 
		 
		else if(!strcmp(command, "bye") || !strcmp(command, "quit"))
		{
			printf("FTP server quitting..\n");
			i = 1;
			send(sock2, &i, sizeof(int), 0);
			exit(0);
		}
	}
	return 0;
}
