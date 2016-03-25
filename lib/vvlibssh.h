#ifndef _VVLIBSSH_H
#define _VVLIBSSH_H

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

#endif
