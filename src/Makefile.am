INCLUDES=-I../lib
bin_PROGRAMS=vvssh vvinput vvinit
vvssh_SOURCES=vvssh.c
vvinput_SOURCES=vvinput.c
vvinit_SOURCES=vvinit.c
vvssh_LDADD=../lib/libvvsqlite.a -lssh2 -lsqlite3
vvinput_LDADD=../lib/libvvsqlite.a -lsqlite3
vvinit_LDADD=../lib/libvvsqlite.a -lsqlite3
