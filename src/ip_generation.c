#include "common_headers.h"
#include "portal_interface.h"
#include "nid_interface.h"
#include "configuration.h"

char dbUser[NAME_LEN], dbPasswd[PASSWD_LEN], dbName[NAME_LEN], dbTableName[2][NAME_LEN]; 

void read_mysql_configuration() {
    FILE *fin = fopen("/home/sccsatmtn/database/IPdb.conf", "r");
    fgets(dbUser, NAME_LEN, fin);
    fgets(dbPasswd, PASSWD_LEN, fin);
    fgets(dbName, NAME_LEN, fin);
    fgets(dbTableName[0], NAME_LEN, fin);
    fgets(dbTableName[1], NAME_LEN, fin);
    fclose(fin);

    dbUser[strlen(dbUser)-1] = dbPasswd[strlen(dbPasswd)-1] = dbName[strlen(dbName)-1] = '\0';
    dbTableName[0][strlen(dbTableName[0])-1] = dbTableName[1][strlen(dbTableName[1])-1] = '\0'; 
}

void generate_lip(struct in6_addr *ipPtr, char *nid) {
    int i, len;
    uint8_t nidByte[10];
    memset(ipPtr->s6_addr, 0, sizeof(ipPtr->s6_addr));
    ipPtr->s6_addr[1] = 1;
    memset(nidByte, 0, sizeof(nidByte));
    len = strlen(nid);
    for (i=0; i<len; ++i) {
        if (nid[i]>='0' && nid[i]<='9')
            nidByte[i] = nid[i]-'0';
        else 
            nidByte[i] = nid[i]-'a'+10;    
    }
    for (i=8; i<13; ++i)
        ipPtr->s6_addr[i] = nidByte[(i-8)*2]*16+nidByte[(i-8)*2+1];
}

struct NIDResponse ask_nid_management(struct NIDRequest nidRequest) {
    struct sockaddr_in6 nidAddress;
    int gipSockfd;
    struct NIDResponse nidResponse;

    gipSockfd = socket(AF_INET6, SOCK_STREAM, 0);
    
    nidAddress.sin6_family = AF_INET6;
    inet_pton(AF_INET6, nidManageAddr, &nidAddress.sin6_addr.s6_addr);
    nidAddress.sin6_port = htons(nidManagePort);

    while (connect(gipSockfd, (struct sockaddr *)&nidAddress, sizeof(nidAddress))==-1) {}
    
    write(gipSockfd, &nidRequest, sizeof(nidRequest));
    read(gipSockfd, &nidResponse, sizeof(nidResponse));
    return nidResponse;
}

void convert_to_hex(unsigned char duid[], unsigned char duidByte[]) {
    for (int i=0; i<strlen(duidByte); ++i) {
        duid[i*2] = (duidByte[i] & 240) >> 4;
        duid[i*2+1] = (duidByte[i] & 15);
    }
    duid[strlen(duidByte)*2] = '\0';
    for (int i=0; i<strlen(duid); ++i)
        if (duid[i]<10) 
            duid[i] = duid[i]+'0';
        else
            duid[i] = duid[i]+'a'-10;
}

int delete_duid(unsigned char duid[], struct in6_addr ip, int table) {
    char sql[SQL_LEN];
    char ipStr[IP_LEN];
    inet_ntop(AF_INET6, ip.s6_addr, ipStr, sizeof(ipStr));
    printf("delete_duid ip: %s\n", ipStr);
    sprintf(sql, "SELECT duid, ip FROM %s WHERE ip=\'%s\'", dbTableName[table], ipStr);

    MYSQL myConn;
    mysql_init(&myConn);
    mysql_real_connect(&myConn, "localhost", dbUser, dbPasswd, dbName, 0, NULL, 0);
    printf("start select\n");
    int res = mysql_query(&myConn, sql);
    MYSQL_RES *resPtr = mysql_store_result(&myConn);
    printf("find %d\n", mysql_num_rows(resPtr));
    MYSQL_ROW sqlRow = mysql_fetch_row(resPtr);
    mysql_free_result(resPtr);
    strcpy(duid, sqlRow[0]);
    printf("find %s\n", sqlRow[0]);
    /*convert_to_hex(duid, duidByte);*/
    sprintf(sql, "DELETE FROM %s WHERE ip=\'%s\'", dbTableName[table], ipStr);
    printf("delete ip %s\n", ipStr);
    res = mysql_query(&myConn, sql);
    mysql_close(&myConn);
    printf("delete complete\n");
    return 1;
}  

void insert_duid(unsigned char duid[], struct in6_addr ip) {
    char sql[SQL_LEN];
    char ipStr[IP_LEN];
    inet_ntop(AF_INET6, ip.s6_addr, ipStr, sizeof(ipStr));
    sprintf(sql, "INSERT INTO %s VALUES(\'%s\', \'%s\')", dbTableName[LONGTERM_TABLE], duid, ipStr);
    
    MYSQL myConn;
    mysql_init(&myConn);
    mysql_real_connect(&myConn, "localhost", dbUser, dbPasswd, dbName, 0, NULL, 0);
    int res = mysql_query(&myConn, sql);
    mysql_close(&myConn); 
}

void process_log_in_request(int clientSockfd) {
    printf("start process log in request\n");
    struct PortalLogInRequest portalRequest;
    struct PortalLogInResponse portalResponse;
    struct NIDRequest nidRequest;
    struct NIDResponse nidResponse;
    
    read(clientSockfd, &portalRequest, sizeof(portalRequest));
    
    strcpy(nidRequest.nid, portalRequest.nid);
    strcpy(nidRequest.passwd, portalRequest.passwd); 

    printf("nid: %s\n", portalRequest.nid);
    printf("passwd: %s\n", portalRequest.passwd);
    nidResponse = ask_nid_management(nidRequest);
        
    printf("ask nid management complete\n");

    strcpy(portalResponse.nid, nidResponse.nid);
    portalResponse.succeed = nidResponse.succeed; 
    if (portalResponse.succeed) { 
        generate_lip(&portalResponse.lip, portalResponse.nid);
        unsigned char duid[DUID_LEN];
        int res = delete_duid(duid, portalRequest.ip, TEMPORARY_TABLE);
        printf("duid: %s\n", duid);
        insert_duid(duid, portalResponse.lip);
    }
    else 
        memset(portalResponse.lip.s6_addr, 0, sizeof(portalResponse.lip.s6_addr)); 

    char tempIP[IP_LEN];
    inet_ntop(AF_INET6, portalResponse.lip.s6_addr, tempIP, sizeof(tempIP));
    printf("lip: %s\n", tempIP);
    write(clientSockfd, &portalResponse, sizeof(portalResponse));
}

void process_log_out_request(int clientSockfd) {
    printf("start process log_out_request\n");
    struct PortalLogOutRequest portalRequest;
    struct PortalLogOutResponse portalResponse;

    read(clientSockfd, &portalRequest, sizeof(portalRequest));
    for (int i=0; i<16; ++i)
        printf("%d ", portalRequest.ip.s6_addr[i]);
    printf("\n");
    
    printf("nid: %s\n", portalRequest.nid);
    
    unsigned char duid[DUID_LEN];
    int res = delete_duid(duid, portalRequest.ip, LONGTERM_TABLE);

    strcpy(portalResponse.nid, portalRequest.nid);
    portalResponse.succeed = res;

    write(clientSockfd, &portalResponse, sizeof(portalResponse));
}

int main() {
    struct PortalLogOutRequest portalRequest;
    int serverSockfd, clientSockfd;
    int serverLen, clientLen;
    struct sockaddr_in6 serverAddress;
    struct sockaddr_in6 clientAddress;
    char requestType;

    printf("%d\n", sizeof(portalRequest));
    read_mysql_configuration();

    serverSockfd = socket(AF_INET6, SOCK_STREAM, 0);
    
    serverAddress.sin6_family = AF_INET6;
    inet_pton(AF_INET6, ipGenerateAddr, serverAddress.sin6_addr.s6_addr);
    serverAddress.sin6_port = htons(ipGeneratePort);
    serverLen = sizeof(serverAddress);
    bind(serverSockfd, (struct sockaddr *)&serverAddress, serverLen);

    listen(serverSockfd, 5);

    clientLen = sizeof(clientAddress);

    while (1) {
        printf("server waiting.\n");

        clientSockfd = accept(serverSockfd, (struct sockaddr *)&clientAddress, &clientLen);

        printf("start handle user's request.\n");
        
        read(clientSockfd, &requestType, sizeof(requestType));

        printf("%c\n", requestType);

        if (requestType==LOG_IN) 
            process_log_in_request(clientSockfd);
        else
            process_log_out_request(clientSockfd);

        printf("response one user.\n");

        close(clientSockfd);      
    }
   
    return EXIT_SUCCESS;
}
