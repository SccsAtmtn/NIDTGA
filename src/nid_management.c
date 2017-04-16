#include "common_headers.h"
#include "nid_interface.h"

int main() {
    int serverSockfd, clientSockfd;
    int serverLen, clientLen, requestLen, responseLen;
    struct sockaddr_in6 serverAddress;
    struct sockaddr_in6 clientAddress;
    struct UserRequest request;
    struct UserResponse response;

    serverSockfd = socket(AF_INET6, SOCK_STREAM, 0);
    serverAddress.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "1::2", serverAddress.sin6_addr.s6_addr);
    serverAddress.sin6_port = htons(9734);
    serverLen = sizeof(serverAddress);
    bind(serverSockfd, (struct sockaddr *)&serverAddress, serverLen);

    listen(serverSockfd, 5);
    
    requestLen = sizeof(request);
    responseLen = sizeof(response);

    while (1) {
        clientLen = sizeof(clientAddress);
        clientSockfd = accept(serverSockfd, (struct sockaddr *)&clientAddress, &clientLen);        
        read(clientSockfd, &request, requestLen);
        memcpy(response.nid, request.nid, sizeof(request.nid));
        response.succeed = 1;
        write(clientSockfd, &response, responseLen);
        close(clientSockfd);
    }

    return 0;
}
