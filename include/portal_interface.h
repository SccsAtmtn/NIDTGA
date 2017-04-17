#ifndef PORTAL_INTERFACE
#define PORTAL_INTERFACE

struct PortalRequest {
    char nid[11];
    char passwd[21];
    struct in6_addr ip;
};

struct PortalResponse {
    int succeed;
    char nid[11];
    struct in6_addr lip;
};

#endif
