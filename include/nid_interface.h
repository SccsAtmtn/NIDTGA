struct UserRequest {
    char nid[11];
    char passwd[21];
};

struct UserResponse {
    int succeed;
    char nid[11];
};
