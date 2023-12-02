const int MAX_NAME = 1000;
const int MAX_DATA = 1000;

enum msgType{
    LOGIN,
    LO_ACK,
    LO_NAK,
    EXIT,
    JOIN,
    JN_ACK,
    JN_NAK,
    LEAVE_SESS,
    NEW_SESS,
    NS_ACK,
    MESSAGE,
    QUERY,
    QU_ACK
};

struct message{
    unsigned int type;
    unsigned int size;
    unsigned char source[1000];
    unsigned char data[1000];
};

struct userInfo{
    char *clientID;
    char *pwd;
    char *sessionID;

    char *IP;
    int PORT;
};