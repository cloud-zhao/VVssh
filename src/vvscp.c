#include <libssh2.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <getopt.h>
//#include "../lib/vvsqlite.h"

#ifndef PUSH
#define PUSH 1
#endif
#ifndef PULL
#define PULL 2
#endif


static int ifstr(char *ps,char pt);
static int strpara(char *para,char *pd[],char pp[]);
static int ifip(const char *ip);
static int _help(void);

int main(int argc,char *argv[])
{
	unsigned long hostaddr;
	int sock,i,recv_send,auth_pw=1;
	struct sockaddr_in sin;
	//const char *fingerprint;
	LIBSSH2_SESSION *session;
	LIBSSH2_CHANNEL *channel;
	const char *host=NULL;
	const char *hostname=NULL;
	const char *user=NULL;
	const char *pass=NULL;
	const char *ip=NULL;
	const char *key=NULL;
	const char *role=NULL;
	const char *local_path=NULL;
	const char *remote_path=NULL;
	struct stat fileinfo;
	FILE *local_file;
	int rc=0;
	char all_para[1024];
	char *pa[3];

	//Parameter analysis
	if((argc!=3)&&(argc!=4)){
		_help();
		return 1;
	}

	if(argc==4)
		pass=argv[3];

	if(ifstr(argv[1],':')){
		recv_send=PULL;
		strpara(argv[1],pa,all_para);
		if(ifstr(argv[1],'@')){
			user=pa[0];
			host=pa[1];
			remote_path=pa[2];
		}else{
			host=pa[0];
			remote_path=pa[1];
		}
		local_path=argv[2];
	}else if(ifstr(argv[2],':')){
		recv_send=PUSH;
		strpara(argv[2],pa,all_para);
		if(ifstr(argv[2],'@')){
			user=pa[0];
			host=pa[1];
			remote_path=pa[2];
		}else{
			host=pa[0];
			remote_path=pa[1];
		}
		local_path=argv[1];
	}else{
		fprintf(stderr,"Parameter format error.\n");
		return 1;
	}

	//Host information analysis
	if(!ifip(host))
		ip=host;
	if(ifstr(host,'_')&&strcmp(host,"all"))


	printf("%s\t%s\t%s\t%s\n",user,pass,local_path,remote_path);
	return 0;


	rc=libssh2_init(0);
	if(rc != 0){
		fprintf(stderr,"libssh2 initialization failed (%d)\n",rc);
		return 1;
	}

	sock=socket(AF_INET,SOCK_STREAM,0);
	sin.sin_family=AF_INET;
	sin.sin_port=htons(22);
	sin.sin_addr.s_addr=hostaddr;
	if(connect(sock,(struct sockaddr*)(&sin),sizeof(struct sockaddr_in)) != 0){
		fprintf(stderr,"failed to connect\n");
		return -1;
	}

	session=libssh2_session_init();
	if(!session)
		return -1;

	rc=libssh2_session_handshake(session,sock);
	if(rc){
		fprintf(stderr,"Failure establishing SSH session:%d\n",rc);
		return -1;
	}

	/*
	fingerprint=libssh2_hostkey_hash(session,LIBSSH2_HOSTKEY_HASH_SHA1);
	fprintf(stderr,"Fingerprint: ");
	for(i=0;i<20;i++){
		fprintf(stderr,"%02X",(unsigned char)fingerprint[i]);
	}
	fprintf(stderr,"\n");
	*/

	if(auth_pw){
		if(libssh2_userauth_password(session,user,pass)){
			fprintf(stderr,"Authentication by password failed\n");
			goto shutdown;
		}
	}else{
		if(user == "root"){
			if(libssh2_userauth_publickey_fromfile(session,user,"/root/.ssh/id_rsa.pub","/root/.ssh/id_rsa",pass)){
				fprintf(stderr,"\tAuthentication by public key failed\n");
				goto shutdown;
			}
		}else{
			if(libssh2_userauth_publickey_fromfile(session,user,"/home/user/.ssh/id_rsa.pub","/home/user/.ssh/id_rsa",pass)){
				fprintf(stderr,"\tAuthentication by public key failed\n");
				goto shutdown;
			}
		}
	}

	if(recv_send){
		off_t got=0;

		channel=libssh2_scp_recv(session,remote_path,&fileinfo);
		if(!channel){
			fprintf(stderr,"Unable to open a session:%d\n",libssh2_session_last_errno(session));
			goto shutdown;
		}
		local_file=fopen(local_path,"wb");
		if(!local_file){
			fprintf(stderr,"Open file %s failed\n",local_path);
			goto clean_channel;
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
	}else{
		size_t nread;
		char mem[1024];
		char *ptr;

		local_file=fopen(local_path,"rb");
		if(!local_file){
			fprintf(stderr,"Open file %s failed\n",local_path);
			goto shutdown;
		}
		stat(local_path,&fileinfo);

		channel=libssh2_scp_send(session,remote_path,fileinfo.st_mode & 0777,(unsigned long)fileinfo.st_size);
	        if(!channel){
        	        char *errmsg;
	                int errlen;
	                int err=libssh2_session_last_error(session,&errmsg,&errlen,0);
	                fprintf(stderr,"Unable to open a session:(%d) %s\n",err,errmsg);
	                goto shutdown;
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
	}

	clean_channel:
		libssh2_channel_free(channel);
		channel=NULL;
	shutdown:
		libssh2_session_disconnect(session,"Normal shutdown");
		libssh2_session_free(session);

	close(sock);
	libssh2_exit();
	return 0;
}

static int ifstr(char *ps,char pt){
	while(*ps){
		if(*ps == pt)
			return 1;
		ps++;
	}
	return 0;
}

static int strpara(char *para,char *pd[],char pp[]){
        char *cp=pp;
        int i=1,j=0;
        pd[0]=&pp[0];

        while(*para){
                if((*para == '@') || (*para == ':')){
                        pp[j]='\0';
                        cp++;
                        j++;
                        pd[i]=&pp[j];
                        para++;
                        i++;
                        continue;
                }   
                *cp++=*para++;
                j++;
        }   
        pp[j]='\0';
        return 0;
}

static int ifip(const char *ip){
	unsigned long addr;
	if(ip==NULL)
		return 1;
	addr=inet_addr(ip);
	if(addr==INADDR_NONE)
		return 1;
	else
		return 0;
}

static int _help(void){
	printf("Usage:\n\t");
	printf("Push file to remote host: \n\t\tvvscp localfile user@[host|ip|role|all]:filepath [password]\n\t");
	printf("Pull file from remote host: \n\t\tvvscp user@[host|ip|role|all]:filepath localfile [password]\n");
	printf("Options:\n\t");
	printf("host\t\t\tTarget host name\n\t");
	printf("ip\t\t\tTarget ipadder\n\t");
	printf("role\t\t\tTarget host role.All will be from the same role as host push or pull files\n\t");
	printf("all\t\t\tAll target host.All will be from the host push or pull files\n\t");
	printf("user\t\t\tTarget host user.role|all use the same user.user can set null\n\t");
	printf("password\t\tTarget host password.role|all use the same password\n\t");
	printf("role|all pull file localfile must directory\n");
	printf("For more see source code.\n");
	return 0;
}

