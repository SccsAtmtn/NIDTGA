#include "common_headers.h"
#include "nid_interface.h"
#include "configuration.h"

int main() {
    int serverSockfd, clientSockfd;
    int serverLen, clientLen, requestLen, responseLen;
    struct sockaddr_in6 serverAddress;
    struct sockaddr_in6 clientAddress;
    struct NIDRequest request;
    struct NIDResponse response;
    char dbUser[21], dbUserPasswd[21], dbName[21], dbTableName[21];

    FILE *fin = fopen("/home/sccsatmtn/database/NIDdb.conf", "r");
    fgets(dbUser, 20, fin);
    fgets(dbUserPasswd, 20, fin);
    fgets(dbName, 20, fin);
    fgets(dbTableName, 20, fin);
    fclose(fin);

    dbUser[strlen(dbUser)-1] = dbUserPasswd[strlen(dbUserPasswd)-1] = '\0';
    dbName[strlen(dbName)-1] = dbTableName[strlen(dbTableName)-1] = '\0';

    serverSockfd = socket(AF_INET6, SOCK_STREAM, 0);
    serverAddress.sin6_family = AF_INET6;
    inet_pton(AF_INET6, nidManageAddr, serverAddress.sin6_addr.s6_addr);
    serverAddress.sin6_port = htons(nidManagePort);
    serverLen = sizeof(serverAddress);
    bind(serverSockfd, (struct sockaddr *)&serverAddress, serverLen);

    listen(serverSockfd, 5);
    
    requestLen = sizeof(request);
    responseLen = sizeof(response);

    while (1) {
        clientLen = sizeof(clientAddress);
        clientSockfd = accept(serverSockfd, (struct sockaddr *)&clientAddress, &clientLen);        
        read(clientSockfd, &request, requestLen);
        
        MYSQL myConnection;
        mysql_init(&myConnection);
        while (!mysql_real_connect(&myConnection, "localhost", dbUser, dbUserPasswd, dbName, 0, NULL, 0)) {
            printf("Connection failed\n");
        }
        char sql[250];
        sprintf(sql, "SELECT nid FROM %s WHERE nid=\'%s\' and passwd=\'%s\'", dbTableName, request.nid, request.passwd);
        int res = mysql_query(&myConnection, sql); 
        if (res) {
            printf("SELECT error %d: %s\n", mysql_errno(&myConnection), mysql_error(&myConnection));
        }
        else {
            MYSQL_RES *resPtr = mysql_store_result(&myConnection); 
            if (mysql_num_rows(resPtr)) 
                response.succeed = 1;
            else
                response.succeed = 0;
            mysql_free_result(resPtr);
        }
        mysql_close(&myConnection);

        memcpy(response.nid, request.nid, sizeof(request.nid));
        write(clientSockfd, &response, responseLen);
        close(clientSockfd);
    }

    return 0;
}
