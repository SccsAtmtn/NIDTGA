#ifndef __PORTAL_INTERFACE
#define __PORTAL_INTERFACE

#define LOG_IN '0'
#define LOG_OUT '1'

struct PortalLogInRequest {
    char nid[11];
    char passwd[21];
    struct in6_addr ip;
};

struct PortalLogInResponse {
    int succeed;
    char nid[11];
    struct in6_addr lip;
};

struct PortalLogOutRequest {
    char nid[11];
    struct in6_addr ip;
};

struct PortalLogOutResponse {
    int succeed;
    char nid[11];
};

#endif
