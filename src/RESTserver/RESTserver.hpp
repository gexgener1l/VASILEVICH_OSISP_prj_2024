/*
File:   RESTserver.hpp
Author: Hanson
Desc:   Define types and function prototypes of the REST server
*/

#define MG_ENABLE_SOCKETPAIR 1

#if !defined(_MSC_VER)
#include "mongoose.h"
#else
extern "C" {
#include "mongoose.h"
}
#endif

#include <map>
#include <string>

// handler type is for the server event handlers
typedef void (*handler)(mg_connection *connection, int ev, mg_http_message *ev_data, void *fn_data);

// For internal use only. Stores router info, including method and event handler
typedef struct _handlerInfo {
    std::string method;         // Empty string ("") means method will be ignored
    handler     eventHandler;   // Remember to check for NULL function pointers
} handlerInfo;

// handler_identifier can be used to remove router rules
typedef std::pair<std::map<std::string, handlerInfo>::iterator, bool> handler_identifier;

class RESTserver {
public:

    handler_identifier addHandler(std::string method, std::string path, handler eventHandler);

    void removeHandler(handler_identifier identifier);

    void setDefaultHandler(handler eventHandler);

    void removeDefaultHandler();

    void setWrongMethodHandler(handler eventHandler);

    void removeWrongMethodHandler();

    handler matchHandler(std::string method, std::string path);

    void setPollHandler(handler pollHandler);

    void removePollHandler();

    handler getPollHandler();

    void startServer(std::string connectionString, int pollFrequency, void *userdata);

    void stopServer();

private:
    std::map<std::string, handlerInfo> router;
    handlerInfo defaultHandler = { "", (handler)NULL };
    handlerInfo wrongMethodHandler = { "", (handler)NULL };
    handlerInfo pollHandler = { "", (handler)NULL };

    // If the server is stopping
    bool stopping = false;
};

typedef struct _dispatcherInfo {
    RESTserver  *ptrToClass;
    void        *userdata;
} dispatcherInfo;
