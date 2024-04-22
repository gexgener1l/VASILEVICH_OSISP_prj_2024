
#include "RESTserver.hpp"

static void builtInHandler(mg_connection *connection, int ev, mg_http_message *ev_data, void *fn_data);
static void httpRequestDispatch(struct mg_connection *connection, int ev, void *ev_data, void *fn_data);

std::string ucase(std::string& target) {
    for (size_t i = 0; i < target.length(); i++) {
        target[i] &= ~32;
    }
    return target;
}

handler_identifier RESTserver::addHandler(std::string method, std::string path, handler eventHandler) {
    handlerInfo info;
    info.eventHandler = eventHandler;
    info.method = ucase(method);
    return this->router.insert(std::pair<std::string, handlerInfo>(path, info));
}

void RESTserver::removeHandler(handler_identifier identifier) {
    this->router.erase(identifier.first);
}

void RESTserver::setDefaultHandler(handler eventHandler) {
    this->defaultHandler = { "", eventHandler };
}

void RESTserver::removeDefaultHandler() {
    this->defaultHandler = { "", (handler)NULL };
}

void RESTserver::setPollHandler(handler pollHandler) {
    this->pollHandler = { "", pollHandler };
}

void RESTserver::removePollHandler() {
    this->pollHandler = { "", (handler)NULL };
}

handler RESTserver::getPollHandler() {
    if (this->pollHandler.eventHandler) {
        return this->pollHandler.eventHandler;
    }
    else {
        return [](struct mg_connection *connection, int ev, mg_http_message *ev_data, void *fn_data) {};
    }
}

void RESTserver::setWrongMethodHandler(handler eventHandler) {
    this->wrongMethodHandler = { "", eventHandler };
}


void RESTserver::removeWrongMethodHandler() {
    this->wrongMethodHandler = { "", (handler)NULL };
}

void RESTserver::startServer(std::string connectionString, int pollFrequency, void *userdata) {
    struct mg_mgr mgr;
    dispatcherInfo info;

    info.ptrToClass = this;
    info.userdata = userdata;

    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, connectionString.c_str(), httpRequestDispatch, &info);

    for (;;) {
        if (this->stopping) {
            break;
        }
        mg_mgr_poll(&mgr, pollFrequency);
    }
    mg_mgr_free(&mgr);
}

void RESTserver::stopServer() {
    this->stopping = true;
}

handler RESTserver::matchHandler(std::string method, std::string path) {
    auto info = this->router.find(path);   

    if (info != this->router.end()) {
        if (info->second.method.at(0) == '\0' || ucase(method) == info->second.method) {
            return info->second.eventHandler;
        }
        else {
            if (this->wrongMethodHandler.eventHandler) {
                return this->wrongMethodHandler.eventHandler;
            }
            else {
                return builtInHandler;
            }
        }
    }
    else {
        if (this->defaultHandler.eventHandler) {
            return this->defaultHandler.eventHandler;
        }
        else {
            return builtInHandler;
        }
    }
}

static void builtInHandler(mg_connection *connection, int ev, mg_http_message *ev_data, void *fn_data) {
    mg_printf(connection,
        "HTTP/1.1 404\r\n"
        "Content-Length: 9\r\n"
        "\r\n"
        "Not found"
    );
}
static void httpRequestDispatch(struct mg_connection *connection, int ev, void *ev_data, void *fn_data) {
    RESTserver *ptrToClass = ((dispatcherInfo *)fn_data)->ptrToClass; 

    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *httpMsg = (struct mg_http_message *)ev_data;

        auto handler = ptrToClass->matchHandler(
            std::string(httpMsg->method.ptr, httpMsg->method.len),
            std::string(httpMsg->uri.ptr, httpMsg->uri.len)
        );
        handler(connection, ev, (mg_http_message *)ev_data, fn_data);
    }
    else if (ev == MG_EV_POLL) {
        auto handler = ptrToClass->getPollHandler();
        handler(connection, ev, (mg_http_message *)ev_data, fn_data);
    }
}
