/*
 *   CS631 Advanced Programming in the UNIX Environment
 *                    Assignment 2
 *                  Author: Kun Yao
 *                  Date:Sep,14,2013
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define MAXPATHLEN 500
int 
main(int argc,char** argv){

void *file_buf_from,*file_buf_to;
int fd_from,fd_to;
size_t file_from_len;
struct stat file_from_stat;

if(argc != 3){
	(void)fprintf(stderr,
		"Usage: %s [file1] [file2]\n\
		or: %s [file1] [dir]\n\
		or: %s [file1] [...dir/file2]\n",
		    argv[0],argv[0],argv[0]);
	exit(EXIT_FAILURE);
}

/*open the source file*/
if((fd_from = open(argv[1],O_RDWR)) == -1){
	(void)fprintf(stderr,"Open %s Error: %s\n",argv[1],strerror(errno));
	exit(EXIT_FAILURE);
}
if (fstat (fd_from, &file_from_stat) < 0){ 
	(void)fprintf(stderr,"Open %s Error: %s\n",argv[1],strerror(errno));
	exit(EXIT_FAILURE);
}
file_from_len = file_from_stat.st_size;

/*create the target file*/
if((fd_to = open(argv[2],O_RDWR|O_CREAT,S_IRUSR|S_IWUSR)) == -1){

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

			/*if directory: create file with same name as source file*/
			if((fd_to = open(p_path,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR)) == -1){
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

/*Set the new file length same as source file*/
if(ftruncate(fd_to,file_from_len)<0){
		(void)fprintf(stderr,"Open %s Error: %s\n",argv[2],strerror(errno));
		exit(EXIT_FAILURE);
}

/*Map source file into memory & close descriptor*/
file_buf_from=mmap(NULL,file_from_len,PROT_READ,MAP_SHARED,fd_from,SEEK_SET);
close(fd_from);
if((int *)file_buf_from == MAP_FAILED){
	(void)fprintf(stderr,"map %s Error: %s\n",argv[1],strerror(errno));
	exit(EXIT_FAILURE);
}

/*Map the new file to the same place as source file & close the file discriptor*/
file_buf_to =mmap(NULL,file_from_len,PROT_WRITE,MAP_SHARED,fd_to,SEEK_SET);
close(fd_to);
if((int *)file_buf_to == MAP_FAILED){
	(void)fprintf(stderr,"map %s Error: %s\n",argv[2],strerror(errno));
	munmap(file_buf_from,file_from_len);
	exit(EXIT_FAILURE);
}

/*Copy the memory into the new file map place*/
if(memcpy(file_buf_to,file_buf_from,file_from_len)==NULL){
	(void)fprintf(stderr,"map %s Error: %s\n",argv[2],strerror(errno));
	munmap(file_buf_from,file_from_len);
	munmap(file_buf_to,file_from_len);
	exit(EXIT_FAILURE);
}

munmap(file_buf_from,file_from_len);
munmap(file_buf_to,file_from_len);

exit(EXIT_SUCCESS);
}