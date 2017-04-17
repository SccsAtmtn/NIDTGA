#ifndef __NID_INTERFACE
#define __NID_INTERFACE

struct NIDRequest {
    char nid[11];
    char passwd[21];
};

struct NIDResponse {
    int succeed;
    char nid[11];
};

#endif
