/*******************************************************
 * Copyright (C) 2023
 *
 *  Created on: Oct 14, 2023
 *      Author: p.sydow
 *
 *******************************************************/
#pragma once
#include <stdio.h>
#include <pthread.h>

struct HttpResponse{
    char* msg;
    char* contentType;
    char* content;
    int contentLength;
    int code;
};

enum HTTP_REQUEST_METHOD {REQ_UNKNOWN, REQ_GET, REQ_POST, REQ_DELETE};
struct HttpRequest {
    enum HTTP_REQUEST_METHOD method;
    char* url;
    size_t contentLength;
    char* content;
    char* contentType;
    char* host;
};


struct HttpServer {
    void (*start)(struct HttpServer *This, const char p_ip[], int p_port);
    void (*stop)(struct HttpServer *This);
    /*******************************************************************/
    /* Register call back by setting the onHttpRequest pointer         */
    /* Make sure that you register this before you call the start()    */
    /* otherwise you got into trouble because of thread hazard issue   */
    /*******************************************************************/
    struct HttpResponse  (*onHttpRequest)(struct HttpRequest* p_request);
    struct {
        pthread_mutex_t lock;
        volatile int finishMe;
        volatile int serverRunning;
        int listen_sd;
        pthread_t listenThread;
    }Private;
};



struct HttpServer createHttpServer();
struct HttpResponse createSimpleHttpResponse(const char* msg, int code);
