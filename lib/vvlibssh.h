#ifndef _LIBSSH2_H
#include <libssh2.h>
#endif
#ifndef _SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifndef _NETINET_IN_H
#include <netinet/in.h>
#endif
#ifndef _UNISTD_H
#include <unistd.h>
#endif
#ifndef _ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifndef _SYS_TIME_H
#include <sys/time.h>
#endif
#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif
#ifndef _FCNTL_H
#include <fcntl.h>
#endif
#ifndef _ERRNO_H
#include <errno.h>
#endif
#ifndef _STDIO_H
#include <stdio.h>
#endif
#ifndef _CTYPE_H
#include <ctype.h>
#endif
#ifndef _GETOPT_H
#include <getopt.h>
#endif

#ifndef PUSH
#define PUSH 1
#endif
#ifndef PULL
#define PULL 2
#endif

/*
static unsigned long hostaddr;
static int sock;
static struct sockaddr_in sin;
static LIBSSH2_SESSION *session;
static LIBSSH2_CHANNEL *channel;
static struct stat fileinfo;
static FILE *local_file;
*/

//static int _ssh_connect(const char *ip);

//static int _ssh_auth(const char *user,const char *password,const char *key);

//static int _ssh_dischannel(void);

//static int _ssh_disconnect(void);

int ssh_push(	const char *ip,
		const char *user,
		const char *password,
		const char *key,
		const char *local,
		const char *remote
		);

int ssh_pull(	const char *ip,
		const char *user,
		const char *password,
		const char *key,
		const char *local,
		const char *remote
		);

//static int _ssh_cmd(const char *cmd,char *result);

int ssh_cmd(	const char *ip,
		const char *user,
		const char *password,
		const char *key,
		const char *cmd,
		char *result
		);

