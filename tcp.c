/*
 *   CS631 Advanced Programming in the UNIX Environment
 *                    Assignment 2
 *                  Author: Kun Yao
 *                  Date:Sep,14,2013
 */
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define MAXPATHLEN 500
int 
main(int argc,char** argv){

char buffer[BUFFER_SIZE];
char* ptr;
int fd_from,fd_to;
int bytes_read,bytes_write;

if(argc != 3){
        (void)fprintf(stderr,
                "Usage: %s [file1] [file2]\n\
                or: %s [file1] [dir]\n\
                or: %s [file1] [...dir/file2]\n",
                    argv[0],argv[0],argv[0]);
        exit(EXIT_FAILURE);
}

/*open the source file*/
if((fd_from = open(argv[1],O_RDONLY)) == -1){
        (void)fprintf(stderr,"Open %s Error: %s\n",argv[1],strerror(errno));
        exit(EXIT_FAILURE);
}

/*create the target file*/
if((fd_to = open(argv[2],O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR)) == -1){

        /*create failed check if directory*/
        if(errno == EISDIR){
                char to_path[MAXPATHLEN + 1]="";
                char *p_path;
                char *slash = "/";
                if(strlen(argv[2])+strlen(argv[1])<MAXPATHLEN){
                        p_path = strncpy(to_path, argv[2], strlen(argv[2]));
                        if(p_path[strlen(to_path)-1] != '/') 
                                p_path = strncat(to_path, slash, (int)strlen(slash));
                p_path = strncat(to_path, argv[1], strlen(argv[1]));

                        /*create file with same name as source file*/
                        if((fd_to = open(p_path,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR)) == -1){
                                (void)fprintf(stderr,"Open %s Error: %s\n",argv[2],strerror(errno));
                                exit(EXIT_FAILURE);
                        }

                }else{
                        (void)fprintf(stderr,"%s: name/path too long",argv[2]);
                        exit(EXIT_FAILURE);
                }
        }else{
                (void)fprintf(stderr,"Open %s Error: %s\n",argv[2],strerror(errno));
                exit(EXIT_FAILURE);
        }
}

/*Begin to copy file*/
bytes_read = read(fd_from,buffer,BUFFER_SIZE);
while(bytes_read){

        if(bytes_read == -1) {
                (void)fprintf(stderr,"Read %s Error.\n",argv[1]);
                exit(EXIT_FAILURE);
        }

        /*Read not complete*/
        else if(bytes_read > 0){
                ptr = buffer;
                bytes_write = write(fd_to,ptr,bytes_read);

                /*write from current buffer not complete*/
                while(bytes_write){
                        if(bytes_write == -1) {
                                (void)fprintf(stderr,"Write %s Error.\n",argv[2]);
                                exit(EXIT_FAILURE);
                        }
                        else if(bytes_write == bytes_read) break;
                        else if(bytes_write > 0){
                                ptr += bytes_write;
                                bytes_read -= bytes_write;
                        }
                        bytes_write = write(fd_to,ptr,bytes_read);
                }
        }
        bytes_read = read(fd_from,buffer,BUFFER_SIZE);
}

close(fd_from);
close(fd_to);
exit(EXIT_SUCCESS);
}