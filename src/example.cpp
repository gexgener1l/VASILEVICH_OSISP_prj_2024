#include "RESTserver/RESTserver.hpp"
RESTserver server;
int main() {
    server.addHandler("GET", "/hello",
                      [](struct mg_connection *connection, int ev, mg_http_message *ev_data, void *fn_data) {
                          mg_http_reply(connection, 200, NULL, "Hello world!");
                      }
    );
    server.startServer("localhost:8000", 1000, NULL);
    return 0;
}