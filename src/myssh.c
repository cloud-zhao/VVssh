/*#include "libssh2_config.h"*/
#include <libssh2.h>
#include <libssh2_sftp.h>
 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
 
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>



const char *keyfile1="~/.ssh/id_rsa.pub";
const char *keyfile2="~/.ssh/id_rsa";
const char *username="root";
const char *password="kvm123!@#";
 
 
static void kbd_callback(const char *name, int name_len,
                         const char *instruction, int instruction_len,
                         int num_prompts,
                         const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
                         LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses,
                         void **abstract)
{
    (void)name;
    (void)name_len;
    (void)instruction;
    (void)instruction_len;
    if (num_prompts == 1) {
        responses[0].text = strdup(password);
        responses[0].length = strlen(password);
    }
    (void)prompts;
    (void)abstract;
}


int main(int argc, char *argv[])
{
    unsigned long hostaddr;
    int rc, sock, i, auth_pw = 0;
    struct sockaddr_in sin;
    const char *fingerprint;
    char *userauthlist;
    LIBSSH2_SESSION *session;
    LIBSSH2_CHANNEL *channel;
 
#ifdef WIN32
    WSADATA wsadata;
    int err;
 
    err = WSAStartup(MAKEWORD(2,0), &wsadata);
    if (err != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", err);
        return 1;
    }
#endif
 
    if (argc > 1) {
        hostaddr = inet_addr(argv[1]);
    } else {
        hostaddr = inet_addr("127.0.0.1");
    }
 
    if(argc > 2) {
        username = argv[2];
    }
    if(argc > 3) {
        password = argv[3];
    }
 
    rc = libssh2_init (0);

    if (rc != 0) {
        fprintf (stderr, "libssh2 initialization failed (%d)\n", rc);
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
 
    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    sin.sin_addr.s_addr = hostaddr;
    if (connect(sock, (struct sockaddr*)(&sin),
                sizeof(struct sockaddr_in)) != 0) {
        fprintf(stderr, "failed to connect!\n");
        return -1;
    }


    session = libssh2_session_init();

    if (libssh2_session_handshake(session, sock)) {

        fprintf(stderr, "Failure establishing SSH session\n");
        return -1;
    }


    fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);

    fprintf(stderr, "Fingerprint: ");
    for(i = 0; i < 20; i++) {
        fprintf(stderr, "%02X ", (unsigned char)fingerprint[i]);
    }
    fprintf(stderr, "\n");

    userauthlist = libssh2_userauth_list(session, username, strlen(username));

    fprintf(stderr, "Authentication methods: %s\n", userauthlist);
    if (strstr(userauthlist, "password") != NULL) {
        auth_pw |= 1;
    }
    if (strstr(userauthlist, "keyboard-interactive") != NULL) {
        auth_pw |= 2;
    }
    if (strstr(userauthlist, "publickey") != NULL) {
        auth_pw |= 4;
    }

    if(argc > 4) {
        if ((auth_pw & 1) && !strcasecmp(argv[4], "-p")) {
            auth_pw = 1;
        }
        if ((auth_pw & 2) && !strcasecmp(argv[4], "-i")) {
            auth_pw = 2;
        }
        if ((auth_pw & 4) && !strcasecmp(argv[4], "-k")) {
            auth_pw = 4;
        }
    }
 
    if (auth_pw & 1) {
        if (libssh2_userauth_password(session, username, password)) {

            fprintf(stderr, "\tAuthentication by password failed!\n");
            goto shutdown;
        } else {
            fprintf(stderr, "\tAuthentication by password succeeded.\n");
        }
    } else if (auth_pw & 2) {
        if (libssh2_userauth_keyboard_interactive(session, username,

                                                  &kbd_callback) ) {
            fprintf(stderr,
                "\tAuthentication by keyboard-interactive failed!\n");
            goto shutdown;
        } else {
            fprintf(stderr,
                "\tAuthentication by keyboard-interactive succeeded.\n");
        }
    } else if (auth_pw & 4) {
        if (libssh2_userauth_publickey_fromfile(session, username, keyfile1,

                                                keyfile2, password)) {
            fprintf(stderr, "\tAuthentication by public key failed!\n");
            goto shutdown;
        } else {
            fprintf(stderr, "\tAuthentication by public key succeeded.\n");
        }
    } else {
        fprintf(stderr, "No supported authentication methods found!\n");
        goto shutdown;
    }

    if (!(channel = libssh2_channel_open_session(session))) {

        fprintf(stderr, "Unable to open a session\n");
        goto shutdown;
    }

    printf("libssh2_channel_setenv\n"); //insert

    libssh2_channel_setenv(channel, "FOO", "bar");

    if (libssh2_channel_request_pty(channel, "vanilla")) {

        fprintf(stderr, "Failed requesting pty\n");
        goto skip_shell;
    }
 
    printf("Open a SHELL on that pty\n"); //insert

    /* Open a SHELL on that pty */ 
    if (libssh2_channel_shell(channel)) {

        fprintf(stderr, "Unable to request shell on allocated pty\n");
        goto shutdown;
    }

    printf("END!!!\n"); //insert
 
  skip_shell:
    if (channel) {
        libssh2_channel_free(channel);

        channel = NULL;
    }

  shutdown:
 
    libssh2_session_disconnect(session,

                               "Normal Shutdown, Thank you for playing");
    libssh2_session_free(session);

 

    close(sock);
    fprintf(stderr, "all done!\n");
 
    libssh2_exit();

 
    return 0;
}
