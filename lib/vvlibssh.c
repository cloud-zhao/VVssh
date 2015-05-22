#include "./vvlibssh.h"

static unsigned long hostaddr;
static int sock;
static struct sockaddr_in sin;
static LIBSSH2_SESSION *session;
static LIBSSH2_CHANNEL *channel;
static struct stat fileinfo;
static FILE *local_file;

static int _ssh_connect(const char *ip,int block);
static int _ssh_auth(const char *user,const char *password,const char *key,int block);
static int _ssh_dischannel(int block);
static int _ssh_disconnect(void);
static int _ssh_cmd(const char *cmd,char *result);
static int _waitsocket(void);

static int _ssh_connect(const char *ip,int block){
	int rc;

	rc=libssh2_init(0);
	if(rc != 0){
		fprintf(stderr,"libssh2 initialization failed (%d)\n",rc);
		return 1;
	}

	hostaddr=inet_addr(ip);

	sock=socket(AF_INET,SOCK_STREAM,0);
	sin.sin_family=AF_INET;
	sin.sin_port=htons(22);
	sin.sin_addr.s_addr=hostaddr;
	if(connect(sock,(struct sockaddr*)(&sin),sizeof(struct sockaddr_in)) != 0){
		fprintf(stderr,"failed to connect\n");
		return -1;
	}

	session=libssh2_session_init();
	if(!session){
		close(sock);
		return -1;
	}
	
	if(block){
		libssh2_session_set_blocking(session,0);
		while((rc=libssh2_session_handshake(session,sock))==LIBSSH2_ERROR_EAGAIN);
	}else
		rc=libssh2_session_handshake(session,sock);
	if(rc){
		fprintf(stderr,"Failure establishing SSH session:%d\n",rc);
		close(sock);
		return -1;
	}
	return 0;
}


static int _ssh_auth(const char *user,const char *password,const char *key,int block){
	int rc;
	if((password==NULL)&&(key==NULL)){
		_ssh_disconnect();
		return 1;
	}
	
	if(key!=NULL){
		if(block){
			while((rc=libssh2_userauth_publickey_fromfile(session,user,NULL,key,NULL))
					==LIBSSH2_ERROR_EAGAIN);
		}else 
			rc=libssh2_userauth_publickey_fromfile(session,user,NULL,key,NULL);
		if(rc){
			fprintf(stderr,"\tAuthentication by public key failed\n");
			if(password==NULL){
				_ssh_disconnect();
				return 1;
			}
		}else
			return 0;
	}

	if(block){
		while((rc=libssh2_userauth_password(session,user,password))
				==LIBSSH2_ERROR_EAGAIN);
	}else
		rc=libssh2_userauth_password(session,user,password);
	if(rc){
		fprintf(stderr,"Authentication by password failed\n");
		_ssh_disconnect();
		return 1;
	}
	return 0;
}

int ssh_pull(	const char *ip,
		const char *user,
		const char *password,
		const char *key,
		const char *local,
		const char *remote){

	off_t got=0;
	int rc;

	if(_ssh_connect(ip,0))
		return 1;
	if(_ssh_auth(user,password,key,0))
		return 1;

	channel=libssh2_scp_recv(session,remote,&fileinfo);
	if(!channel){
		fprintf(stderr,"Unable to open a session:%d\n",libssh2_session_last_errno(session));
		_ssh_disconnect();
		return 1;
	}
	local_file=fopen(local,"wb");
	if(!local_file){
		fprintf(stderr,"Open file %s failed\n",local);
		_ssh_dischannel(0);
		_ssh_disconnect();
		return 1;
	}

	while(got < fileinfo.st_size){
		char mem[1024];
		int amout=sizeof(mem);

		if((fileinfo.st_size - got) < amout){
			amout=fileinfo.st_size - got;
		}

		rc=libssh2_channel_read(channel,mem,amout);
		if(rc > 0){
			/*write(2,mem,rc);*/
			fwrite(mem,rc,1,local_file);
		}else if(rc < 0){
		fprintf(stderr,"libssh2_channel_read() failed:%d\n",rc);
			break;
		}
		got += rc;
	}

	_ssh_dischannel(0);
	_ssh_disconnect();
	return 0;
}

int ssh_push(	const char *ip,
		const char *user,
		const char *password,
		const char *key,
		const char *local,
		const char *remote){

	size_t nread;
	char mem[1024];
	char *ptr;
	int rc;

	local_file=fopen(local,"rb");
	if(!local_file){
		fprintf(stderr,"Open file %s failed\n",local);
		return 1;
	}
	stat(local,&fileinfo);

	if(_ssh_connect(ip,0))
		return 1;
	if(_ssh_auth(user,password,key,0))
		return 1;

	channel=libssh2_scp_send(session,remote,fileinfo.st_mode & 0777,(unsigned long)fileinfo.st_size);
        if(!channel){
	        char *errmsg;
                int errlen;
                int err=libssh2_session_last_error(session,&errmsg,&errlen,0);
                fprintf(stderr,"Unable to open a session:(%d) %s\n",err,errmsg);
		_ssh_disconnect();
		return 1;
        }

        fprintf(stderr,"SCP session waiting to send file\n");
        do{
                nread=fread(mem,1,sizeof(mem),local_file);
                if(nread <= 0){
                        break;
                }
                ptr=mem;

                do{
                        rc=libssh2_channel_write(channel,ptr,nread);
                        if(rc < 0){
                                fprintf(stderr,"ERROR %d\n",rc);
                                break;
                        }else{
                                ptr += rc;
                                nread -= rc;
                        }
                }while(nread);
        }while(1);

        fprintf(stderr,"Sending EOF\n");
        libssh2_channel_send_eof(channel);
        fprintf(stderr,"Waiting for EOF\n");
        libssh2_channel_wait_eof(channel);
        fprintf(stderr,"Waiting for channel to close\n");
        libssh2_channel_wait_closed(channel);

	_ssh_dischannel(0);
	_ssh_disconnect();
	return 0;
}

static _ssh_cmd(const char *cmd,char *result){
	int rc;

	if(cmd==NULL)
		return 0;

	while((channel=libssh2_channel_open_session(session))==NULL &&
		libssh2_session_last_error(session,NULL,NULL,0)==LIBSSH2_ERROR_EAGAIN){
		_waitsocket();
	}
	if(channel==NULL){
		fprintf(stderr,"Error:command channel open failed.\n");
		_ssh_disconnect();
		return 1;
	}

	while((rc=libssh2_channel_exec(channel,cmd))==LIBSSH2_ERROR_EAGAIN){
		_waitsocket();
	}
	if(rc!=0){
		fprintf(stderr,"Error:command exec failed.\n");
		_ssh_dischannel(1);
		_ssh_disconnect();
		return 1;
	}

	for(;;){
		int rc;
		do{
			char buffer[0x4000];
			rc=libssh2_channel_read(channel,buffer,sizeof(buffer));
			if(rc>0){
				int i;
				if(result==NULL)
					for(i=0;i<rc;i++)
						fputc(buffer[i],stdout);
				else{
					for(i=0;i<rc;i++)
						fputc(buffer[i],stdout);
				}
				//fprintf(stdout,"\n");
			}
		}while(rc>0);

		if(rc==LIBSSH2_ERROR_EAGAIN){
			_waitsocket();
		}else
			break;
	}
	
	_ssh_dischannel(1);
	_ssh_disconnect();
	return 0;
}

int ssh_cmd(	const char *ip,
		const char *user,
		const char *password,
		const char *key,
		const char *cmd,
		char *result
		){
	if(cmd==NULL)
		return 0;
	
	if(_ssh_connect(ip,1))
		return 1;
	if(_ssh_auth(user,password,key,1))
		return 1;

	if(_ssh_cmd(cmd,result))
		return 1;

	return 0;
}

static int _ssh_dischannel(int block){
	if(block){
		while(libssh2_channel_close(channel)==LIBSSH2_ERROR_EAGAIN)
			_waitsocket();
	}
	libssh2_channel_free(channel);
	channel=NULL;
	return 0;
}

static int _ssh_disconnect(){
	libssh2_session_disconnect(session,"Normal shutdown");
	libssh2_session_free(session);
	close(sock);
	libssh2_exit();
	return 0;
}


static int _waitsocket(void){
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;
 
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
 
    FD_ZERO(&fd);
    FD_SET(sock, &fd);
 
    dir = libssh2_session_block_directions(session);

 
    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd; 
    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;
 
    rc = select(sock + 1, readfd, writefd, NULL, &timeout);
 
    return rc;
}
