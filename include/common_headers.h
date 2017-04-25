#ifndef __COMMON_HEADERS
#define __COMMON_HEADERS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>

#define TEMPORARY_TABLE 0
#define LONGTERM_TABLE 1
#define NAME_LEN 21
#define PASSWD_LEN 21
#define IP_LEN 41
#define SQL_LEN 250
#define DUID_LEN 129

#endif
