#include <string.h>  
#include <sys/ioctl.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <libgen.h>  
#include <fcntl.h>  
#include <errno.h>  
#include <stdio.h>  
#include <ctype.h>  
#include <stdlib.h>  
#include <termios.h>  
#include <getopt.h>
#define _GNU_SOURCE
   
#include <libssh2.h>  
#include "../lib/vvsqlite.h"
  
#define COPYRIGHT "VVSSH Copyright (C) 2015, V.V wlsdhzy@163.com\n"  
   
struct termios _saved_tio;  
int tio_saved = 0;  
   
static int _raw_mode(void)  
{  
    int rc;  
    struct termios tio;  
  
    rc = tcgetattr(fileno(stdin), &tio);  
    if (rc != -1) {  
        _saved_tio = tio;  
        tio_saved = 1;  
        cfmakeraw(&tio);  
        rc = tcsetattr(fileno(stdin), TCSADRAIN, &tio);  
    }  
  
    return rc;  
}  
   
static int _normal_mode(void)  
{  
    if (tio_saved)  
        return tcsetattr(fileno(stdin), TCSADRAIN, &_saved_tio);  
  
    return 0;  
}

static int _help(void){
	printf("Options:\n\t");
	printf("-h,\t--hostname\t\t\tTarget host name.\n\t");
	printf("-i,\t--ipaddr\t\t\tTarget host ip.\n\t");
	printf("-u,\t--user\t\t\t\tTarget host user.\n\t");
	printf("-p,\t--password\t\t\tTarget host password.\n\t");
	printf("-k,\t--key\t\t\t\tTarget host private key.\n\t");
	printf("-r,\t--role=[description]\t\tTarget host description.\n\t");
	printf("-H,\t--help\t\t\t\tPrintf help info.\n");
	printf("\n\nUsage:\n\t");
	printf("First login: vvssh -h test1 -i 127.0.0.1 -u root -p root [--role=role]\n\t");
	printf("             vvssh -h test1 -i 127.0.0.1 -u root -k ./id_rsa [--role=role]\n\t");
	printf("             vvssh -h test1 -i 127.0.0.1 -u root -p root -k ./id_rsa [--role=role]\n\t");
	printf("Later login: vvssh -h test1\n\t");
	printf("             vvssh -i ipaddr\n\t");
	printf("	     vvssh -h test1 -i new_ip -u new_user update login info.\n\t");
	printf("	     vvssh -h new_hostname -i ip -u new_user -p new_pass update login info.\n\n");
	printf("For more see source code.\n");
	printf(COPYRIGHT);
	return 0;
}
   
int main (int argc, char *argv[])  
{  
    int sock = 0;  
    unsigned long hostaddr = 0;  
    short port = htons(22);  
    char *hostname = NULL;
    char *ip=NULL;
    char *user = NULL;  
    char *password = NULL;
    char *key = NULL;
    char *role = NULL;  
    struct sockaddr_in sin;  
    LIBSSH2_SESSION *session;  
    LIBSSH2_CHANNEL *channel;  
    int nfds = 1;  
    char buf;  
    LIBSSH2_POLLFD *fds = NULL;  
   
    /* Struct winsize for term size */   
    struct winsize w_size;  
    struct winsize w_size_bck;  
  
    /* For select on stdin */   
    fd_set set;  
    struct timeval timeval_out;  
    timeval_out.tv_sec = 0;  
    timeval_out.tv_usec = 10;  
   
	int opt;
	struct option longopts[]={
		{"hostname",1,NULL,'h'},
               	{"ipaddr",1,NULL,'i'},
                {"user",1,NULL,'u'},
                {"password",1,NULL,'p'},
                {"key",1,NULL,'k'},
                {"role",1,NULL,'r'},
                {"help",0,NULL,'H'},
                {0,0,0,0}
	};

	if(argc<2){
		_help();
		exit(1);
	}
	while((opt=getopt_long(argc,argv,":h: :i: :u: :p: :k: :r: H",longopts,NULL)) != -1){
                switch(opt){
                case 'h':
                        hostname=optarg;
                        break;
                case 'i':
			ip=optarg;
                        break;
                case 'u':
                        user=optarg;
                        break;
                case 'p':
                        password=optarg;
                        break;
                case 'k':
                        key=optarg;
                        break;
                case 'r':
                        role=optarg;
                        break;
                case 'H':
                        _help();
			exit(0);
                case ':':
                        printf("option needs a value\n");
                        exit(1);
                case '?':
                        printf("unknown option:%c\n",optopt);
                        exit(1);
                }
        }

	if((hostname==NULL)&&(ip==NULL)){
		fprintf(stderr,"Please enter hostname or ipaddr\n");
		return 1;
	}

	int sql_ret=0;
	char *whereid=NULL;
	char **sql_result=init_Res();
	int sql_count=0;
	int *psql_count=&sql_count;
	int flag_insert=0;
	int flag_update=-1;

	if((hostname!=NULL)&&(ip!=NULL)){
		sql_ret=sqlite3_checkinfo("hostname",hostname);
		if(sql_ret==2){
			sql_ret=sqlite3_checkinfo("ip",ip);
			if(sql_ret==2){
				if((user!=NULL)&&((key!=NULL)||(password!=NULL))){
					if(key==NULL)
						key="null";
					if(password==NULL)
						password="null";
					if(role==NULL)
						role="null";
					flag_insert=1;
				}else{
					fprintf(stderr,"Error:paramerter imperfect.\n");
					free_Res(sql_result);
					return 1;
				}
			}else if(sql_ret==0){
				whereid="ip";
				sqlite3_alltable(whereid,ip,sql_result,psql_count);
				flag_update=10;
			}
		}else if(sql_ret==0){
			whereid="hostname";
			sqlite3_alltable(whereid,hostname,sql_result,psql_count);
			flag_update=0;
		}
	}else{
		if(hostname!=NULL){
			sql_ret=sqlite3_checkinfo("hostname",hostname);
			if(sql_ret==2){
				fprintf(stderr,"Error:unknown host name %s.\n",hostname);
				free_Res(sql_result);
				return 1;
			}else if(sql_ret==0){
				whereid="hostname";
				sqlite3_alltable(whereid,hostname,sql_result,psql_count);
				ip=sql_result[1];
				flag_update=0;
			}
		}else if(ip!=NULL){
			sql_ret=sqlite3_checkinfo("ip",ip);
			if(sql_ret==2){
				fprintf(stderr,"Error:unknown ipadder %s.\n",ip);
				free_Res(sql_result);
				return 1;
			}else if(sql_ret==0){
				whereid="ip";
				sqlite3_alltable(whereid,ip,sql_result,psql_count);
				hostname=sql_result[0];
				flag_update=10;
			}
		}
	}

	if(sql_ret==1){
		fprintf(stderr,"Database error.\n");
		free_Res(sql_result);
		return 1;
	}

	if(role==NULL)
		role=sql_result[5];
	if(key==NULL)
		key=sql_result[4];
	if(password==NULL)
		password=sql_result[3];
	if(user==NULL);
		user=sql_result[2];
	if(	strcmp(user,sql_result[2]) || strcmp(key,sql_result[4]) || 
		strcmp(password,sql_result[3]) || strcmp(role,sql_result[5]) ||
		strcmp(hostname,sql_result[0]) || strcmp(ip,sql_result[1]))
		flag_update+=1;
	
	hostaddr=inet_addr(ip);

	printf("Login info:%s\t%s\t%s\t%s\n",hostname,ip,user,role);


    /* SSH INIT */
    if (libssh2_init (0) != 0) {  
        fprintf (stderr, "libssh2 initialization failed\n");  
        return -1;  
    }  
  
    sock = socket (AF_INET, SOCK_STREAM, 0);  
    sin.sin_family = AF_INET;  
    sin.sin_port = port;  
    sin.sin_addr.s_addr = hostaddr;  
    if (connect(sock, (struct sockaddr *) &sin, sizeof(struct sockaddr_in)) != 0) {  
        fprintf (stderr, "Failed to established connection!\n");  
        return -1;  
    }  
  
    /* Open a session */   
    session = libssh2_session_init();  
    if (libssh2_session_startup (session, sock) != 0) {  
        fprintf(stderr, "Failed Start the SSH session\n");  
        return -1;  
    }  
    

    /* Authenticate via public key */   
    if (libssh2_userauth_publickey_fromfile(session, user,NULL,key,NULL)){
        fprintf(stderr, "\tAuthentication by public key failed!\n");
    }else{
    	goto SSH_START;
    }

    /* Authenticate via password */   
    if (libssh2_userauth_password(session, user, password) != 0) {  
        fprintf(stderr, "\tAuthentication by password failed!\n");
        close(sock);  
        goto ERROR;  
    }  

SSH_START:
    /* Insert and update login data */
    if(flag_insert==1){
    	if(sqlite3_insert(hostname,ip,user,password,key,role))
		fprintf(stderr,"Error:insert data failed.\n");
    }
    if(flag_update==1){
    	if(sqlite3_delete(hostname,NULL))
		fprintf(stderr,"Error:update data failed.\n");
	else{
		if(sqlite3_insert(hostname,ip,user,password,key,role))
			fprintf(stderr,"Error:update data failed.\n");
		else
			fprintf(stderr,"Update data successful.\n");
	}
    }else if(flag_update==11){
    	if(sqlite3_delete(NULL,ip))
		fprintf(stderr,"Error:update data failed.\n");
	else{
		if(sqlite3_insert(hostname,ip,user,password,key,role))
			fprintf(stderr,"Error:update data failed.\n");
		else
			fprintf(stderr,"Update data successful.\n");
	}
    }

      
    /* Open a channel */   
    channel = libssh2_channel_open_session(session);  
      
    if ( channel == NULL ) {  
        fprintf(stderr, "Failed to open a new channel\n");  
        close(sock);  
        goto ERROR;  
    }  
      
    /* Request a PTY */   
    if (libssh2_channel_request_pty( channel, "xterm") != 0) {  
        fprintf(stderr, "Failed to request a pty\n");  
        close(sock);  
        goto ERROR;  
    }  
      
    /* Request a shell */   
    if (libssh2_channel_shell(channel) != 0) {  
        fprintf(stderr, "Failed to open a shell\n");  
        close(sock);  
        goto ERROR;  
    }  
      
    if (_raw_mode() != 0) {  
        fprintf(stderr, "Failed to entered in raw mode\n");  
        close(sock);  
        goto ERROR;  
    }  
      
    while (1) {  
        FD_ZERO(&set);  
        FD_SET(fileno(stdin),&set);  
      
        ioctl(fileno(stdin), TIOCGWINSZ, &w_size);  
        if ((w_size.ws_row != w_size_bck.ws_row) ||  
            (w_size.ws_col != w_size_bck.ws_col)) {  
            w_size_bck = w_size;  
            libssh2_channel_request_pty_size(channel, w_size.ws_col, w_size.ws_row);  
        }  
      
        if ((fds = malloc (sizeof (LIBSSH2_POLLFD))) == NULL)  
            break;  
      
        fds[0].type = LIBSSH2_POLLFD_CHANNEL;  
        fds[0].fd.channel = channel;  
        fds[0].events = LIBSSH2_POLLFD_POLLIN;  
        fds[0].revents = LIBSSH2_POLLFD_POLLIN;  
      
        if (libssh2_poll(fds, nfds, 0) >0) {  
            libssh2_channel_read(channel, &buf, 1);  
            fprintf(stdout, "%c", buf);  
            fflush(stdout);  
        }  
      
        if (select(fileno(stdin)+1,&set,NULL,NULL,&timeval_out) > 0)  
            if (read(fileno(stdin), &buf, 1) > 0)  
                libssh2_channel_write(channel, &buf, 1);  
      
        free (fds);  
      
        if (libssh2_channel_eof(channel) == 1)  
         break;  
    }  
      
    if (channel) {  
        libssh2_channel_free (channel);  
        channel = NULL;  
    }  
      
    _normal_mode();  
      
    libssh2_exit();  
    free_Res(sql_result);
  
    return 0;  
ERROR:  
    libssh2_session_disconnect(session, "Session Shutdown, Thank you for playing");  
    libssh2_session_free(session);  
    free_Res(sql_result);
    return -1;  
} 

