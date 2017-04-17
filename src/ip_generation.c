#include "common_headers.h"
#include "portal_interface.h"
#include "nid_interface.h"
#include "configuration.h"

void generateLIP(struct in6_addr *ipPtr, char *nid) {
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

int main() {
    int serverSockfd, clientSockfd;
    int serverLen, clientLen;
    struct sockaddr_in6 serverAddress;
    struct sockaddr_in6 clientAddress;
    struct PortalRequest portalRequest;
    struct PortalResponse portalResponse;
    struct NIDRequest nidRequest;
    struct NIDResponse nidResponse;
    char clientIP[100];

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

        read(clientSockfd, &portalRequest, sizeof(portalRequest));
     
        strcpy(nidRequest.nid, portalRequest.nid);
        strcpy(nidRequest.passwd, portalRequest.passwd); 

        nidResponse = ask_nid_management(nidRequest);
        
        strcpy(portalResponse.nid, nidResponse.nid);
        portalResponse.succeed = nidResponse.succeed; 
        if (portalResponse.succeed) { 
            generateLIP(&portalResponse.lip, portalResponse.nid);
            /*
            TODO: add database operations to record the login user
            inet_ntop(AF_INET6, &portalRequest.ip, clientIP, sizeof(clientIP));
            */
        }
        else 
            memset(portalResponse.lip.s6_addr, 0, sizeof(portalResponse.lip.s6_addr)); 

        write(clientSockfd, &portalResponse, sizeof(portalResponse));
        printf("response one user.");
        close(clientSockfd);      
    }
}
