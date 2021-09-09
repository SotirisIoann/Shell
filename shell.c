#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>  
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <netinet/in.h>

#define BUFSIZE 10
#define TOK_DELIM "|>><"

void shell_loop();
char *read_Line();
char **Parse_line(char *command,int *siz);
void execute_command(char **args, char *str, int siz);
int pipe_Comm(char **args,int *st);
void cat_Command(char **args,int *st);

int main(){

    shell_loop();

    return 0;
}

void shell_loop(){
    
    char *command;
    char **args;
    char *str = malloc(BUFSIZE * sizeof(char*));
    int siz = 0;

    do{
        printf("my_shell$ ");
        command = read_Line();
        strcpy(str,command);
        if( strstr(command,"exit") != NULL ){
            exit(1);
        }
        args = Parse_line(command,&siz);
        execute_command(args,str,siz);

        free(args);
        free(command);
    }while( 1 );

}

char **Parse_line(char *command,int *siz){
    int pst = 0;
    char *token;    
    char **tokens = (char**) malloc(BUFSIZE * sizeof(char*));

    token = strtok(command,TOK_DELIM);
    while( token != NULL ){
        tokens[pst] = token;
        strcat(tokens[pst],"\0");
        pst++;
        token = strtok(NULL,TOK_DELIM);
    }
    tokens[pst] = NULL;
    *siz = pst;
    return tokens;
}

char *read_Line(){
    char *com = NULL;
    size_t bufsize = 0; 
    
    if( getline(&com, &bufsize, stdin) == -1 ){
        printf("getline FAILED...\nThe Program has been terminated\n");
        exit(-1);
    }

    return com;
}

void execute_command(char **args,char *str, int siz){

    int status;
    pid_t pid;
    int st = 0;

    // SIMPLE COMMAND TO RUN
    if( strstr(str,"|") == NULL && strstr(str,">>") == NULL && strstr(str,">") == NULL && strstr(str,"<") == NULL && strstr(str,"cat") == NULL ){
        pid = fork();
	    if(pid == -1){   
            printf("fork FAILED...\nThe Program has been terminated\n");
		    exit(-1);
	    }else if(pid == 0){
		    char *arg[4];
		        arg[0] = "sh";
		        arg[1] = "-c";
		        arg[2] = str;
		        arg[3] = NULL;
		    execv ("/bin/sh", arg);
            printf("Execv FAILED...\nThe Program has been terminated\n");
		    exit(-1);
	    }
	    if(waitpid(pid,&status,0) == -1){
		    exit(-1);
	    }

        return;
    }

    // COMMANDS WITH PIPE
    if( strstr(str,"|") != NULL && strstr(str,"<") == NULL && strstr(str,"cat") == NULL ){ 

        if( strstr(str,">") != NULL ){
            st = 1;
        }

        if( strstr(str,">>") != NULL ){
            st = 2;
        }
        
        pid = fork();
        if(pid == -1){
            printf("fork FAILED...\nThe Program has been terminated\n");
		    exit(-1);
	    }else if(pid == 0){
		    pipe_Comm(args,&st);
	    }
	    if(waitpid(pid,&status,0) == -1){
		    exit(-1);
	    }   
        
        return;
    }

    if( strstr(str,">>") != NULL && strstr(str,"|") == NULL && strstr(str,"<") == NULL && strstr(str,"cat") == NULL ){ 
        
        pid = fork();
        if( pid == -1 ){
            printf("fork FAILED...\nThe Program has been terminated\n");
		    exit(-1);
	    }else if( pid == 0 ){
            int fd;
            
            fd = open(args[1],O_RDWR|O_APPEND|O_CREAT,S_IRWXU);
            lseek(fd,0,SEEK_END);
            dup2(fd,STDOUT_FILENO);
            char *arg[4];
		        arg[0] = "sh";
		        arg[1] = "-c";
		        arg[2] = args[0];
		        arg[3] = NULL;
		    execvp ("/bin/sh", arg);
            close(fd);
        }

        if(waitpid(pid,&status,0) == -1){
		    exit(-1);
	    }

    }else if( strstr(str,">") != NULL && strstr(str,"|") == NULL  && strstr(str,"<") == NULL && strstr(str,"cat") == NULL){ 
        
        pid = fork();
        if( pid == -1 ){
            printf("fork FAILED...\nThe Program has been terminated\n");
		    exit(-1);
	    }else if( pid == 0 ){
            int fd;
            
            fd = open(args[1],O_RDWR|O_CREAT,S_IRWXU);
            ftruncate(fd,0);
            lseek(fd,0,SEEK_END);
            dup2(fd,STDOUT_FILENO);
            char *arg[4];
		        arg[0] = "sh";
		        arg[1] = "-c";
		        arg[2] = args[0];
		        arg[3] = NULL;
		    execvp ("/bin/sh", arg);
            close(fd);

        }

        if(waitpid(pid,&status,0) == -1){
		    exit(-1);
	    }
    
    }
    
    // SOME CAT COMMANDS
    if( strstr(str,"cat") != NULL && strstr(str,"|") == NULL  && strstr(str,"<") != NULL ){ 

        if( strstr(str,">") != NULL ){
            st = 1;
        }

        if( strstr(str,">>") != NULL ){
            st = 2;
        }

        pid = fork();
        if( pid == -1 ){
            printf("fork FAILED...\nThe Program has been terminated\n");
		    exit(-1);
	    }else if( pid == 0 ){
            cat_Command(args,&st);
            exit(1);
        }

        if(waitpid(pid,&status,0) == -1){
		    exit(-1);
	    }
    
    }
    
}       

int pipe_Comm(char **args,int *st){

    pid_t pid;
    int pp;
    int fd[2];
    int ffd;
    
    // OVERWRITE
    if( *st == 1 ){
        ffd = open(args[2],O_RDWR|O_CREAT,S_IRWXU);
        if( ffd < 0 ){
            printf("open FAILED\n");
            exit(1); //from child process
        }
        ftruncate(ffd,0);
        lseek(ffd,0,SEEK_END);
        dup2(ffd,STDOUT_FILENO);
    }
    
    // APPEND
    if( *st == 2 ){
        ffd = open(args[2],O_RDWR|O_APPEND|O_CREAT,S_IRWXU);
        if( ffd < 0 ){
            printf("open FAILED\n");
            exit(1); //from child process
        }
        lseek(ffd,0,SEEK_END);
        dup2(ffd,STDOUT_FILENO);
    }
    
    pp = pipe(fd);
    if(pp == -1){
        printf("pipe FAILED\n");
        exit(1); //from child process
    }

    pid = fork(); 
    if(pid < 0){
        printf("fork FAILED\n");
        exit(1); //from child process
    }

    if(pid == 0){
        close(fd[0]); 
        dup2(fd[1], 1);
        close(fd[1]);
        char *arg1[4];
		    arg1[0] = "sh";
		    arg1[1] = "-c";
		    arg1[2] = args[0];
		    arg1[3] = NULL;
		    execv ("/bin/sh", arg1);
    }else{
        close(fd[1]);
        dup2(fd[0], 0);
        close(fd[0]);
        char *arg[4];
		    arg[0] = "sh";
		    arg[1] = "-c";
		    arg[2] = args[1];
		    arg[3] = NULL;
		    execv ("/bin/sh", arg);
    }
    
}

void cat_Command(char **args,int *st){
    pid_t pid;
    int pp;
    int fdin;
    int fdout;
    int n;  
    char c;
    char *fname;
    
    fname = strtok(args[1]," ");

    fdin = open(fname,O_RDWR,S_IRWXU);
    if( fdin < 0 ){
        printf("%s: No such file or directory\n",fname);
        return;
    }

    // OVERWRITE
    if( *st == 1 ){
        fdout = open(args[2],O_RDWR|O_CREAT,S_IRWXU);
        if( fdout < 0 ){
            printf("open FAILED\n");
            return;
        }
        ftruncate(fdout,0);
        lseek(fdout,0,SEEK_END);
        dup2(fdout,STDOUT_FILENO);
    }

    // APPEND
    if( *st == 2 ){
        fdout = open(args[2],O_RDWR|O_APPEND|O_CREAT,S_IRWXU);
        if( fdout < 0 ){
            printf("open FAILED\n");
            return;
        }
        lseek(fdout,0,SEEK_END);
        dup2(fdout,STDOUT_FILENO);
    }

    while (n = read(fdin,&c,sizeof(char)) > 0) {
        putchar(c);
    }

}