/*******************************************************
 * Copyright (C) 2023
 *
 *  Created on: Oct 14, 2023
 *      Author: p.sydow
 *
 *******************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <assert.h>

#include "httpServer.h"
#include "stringUtils.h"
#define  MAX_TCP_PACKAGE_SIZE  1024

static void parsePropertyLine(struct HttpRequest* This, char* line) {
    char* key = line;
    char* value = line;
    char* begin = line;
    /*************************************************************/
    /* Parsing Key                                               */
    /*************************************************************/
    char* end = strstr(begin, ":");
    if (end == NULL) {
        return;
    }
    end[0] = 0;
    begin = end + 1;
    while (*begin == ' ') {
        begin++;
    }

    /*************************************************************/
    /* Parsing Value - rest of message                           */
    /*************************************************************/
    value = begin;

    /*************************************************************/
    /* Dispatch keys                                             */
    /*************************************************************/

    if ((value == NULL) || (key == NULL)) {
        return;
    }
    if (strcmp(key, "Content-Type") == 0) {
        This->contentType = strdup(value);
    } else if (strcmp(key, "Host") == 0) {
        This->host = strdup(value);
    } else if (strcmp(key, "Content-Length") == 0) {
        chomp(value);
        This->contentLength = atoi(value);
    }
}

static void parseFirstLine(struct HttpRequest* This, char* line) {
    char* begin = line;
    char* end = NULL;
    char* methodStr = NULL;
    char* urlStr = NULL;

    /*************************************************************/
    /* Parsing Method Name                                       */
    /*************************************************************/
    end = strstr(begin, " ");
    if (end == NULL) {
        return;
    }
    end[0] = 0;
    methodStr = begin;
    begin = end + 1;

    /*************************************************************/
    /* Parsing Method URL                                        */
    /*************************************************************/
    end = strstr(begin, " ");
    if (end == NULL) {
        return;
    }
    end[0] = 0;
    urlStr = begin;
    begin = end + 1;

    /*************************************************************/
    /* Parsing HTTP Version                                      */
    /*************************************************************/
    // printf("HTTP: {%s}\n", begin);  don't need HTTP version


    This->url = strdup(urlStr);
    if ((This->url == 0) && (This->url[0] == 0)) {
        // no url address
        return;
    }
    if (strcmp(methodStr, "POST") == 0) {
        This->method = REQ_POST;
    } else if (strcmp(methodStr, "GET") == 0) {
        This->method = REQ_GET;
    } else if (strcmp(methodStr, "DELETE") == 0) {
        This->method = REQ_DELETE;
    }
}

static void destroyHttpRequest(struct HttpRequest* This) {
    free(This->content);
    free(This->contentType);
    free(This->host);
    free(This->url);
}

static struct HttpRequest createHttpRequest(char *p_buff, size_t p_size) {
    struct HttpRequest This;
    This.method = REQ_UNKNOWN;
    This.url = NULL;
    This.content = NULL;
    This.contentType = NULL;
    This.host = NULL;
    This.contentLength = 0;


    char *end = NULL;
    char *begin = p_buff;
    int lineNo = 0;
    int endPatter = 0;
    do {
        end = strstr(begin, "\x0D\x0A");
        if (end != NULL) {
            end[0] = 0;
            end[1] = 0;
            if (begin[0] == 0) {
                begin = end + 2;
                endPatter = 1;
                break;
            }
            if (lineNo == 0) {
                parseFirstLine(&This, begin);
            } else {
                parsePropertyLine(&This, begin);
            }
            begin = end + 2;
        }
        lineNo++;
    } while (end != NULL);

    /*************************************************************/
    /* Check if end pattern "\x0D\x0A\x0D\x0A" occur             */
    /*************************************************************/
    if (endPatter == 0) {
        This.method = REQ_UNKNOWN;  // end pattern has not been found
        return This;
    }

    /*************************************************************/
    /* Get content if POST method. Make sure that size match     */
    /*************************************************************/
    if (This.method == REQ_POST) {
        int realSize = (int)(&p_buff[p_size] - begin);
        if (realSize != This.contentLength) {
            This.method = REQ_UNKNOWN;  // size does not match
        } else {
            This.content = strdup(begin);
        }
    }
    return This;
}

static int isServerRunning(struct HttpServer *This) {
    volatile int serverRunning;
    pthread_mutex_lock(&This->Private.lock);
    serverRunning = This->Private.serverRunning;
    pthread_mutex_unlock(&This->Private.lock);
    return serverRunning;
};

static void setServerRunning(struct HttpServer *This, int p_value) {
    pthread_mutex_lock(&This->Private.lock);
    This->Private.serverRunning = p_value;
    pthread_mutex_unlock(&This->Private.lock);
};

static int isServerGonigToEnd(struct HttpServer *This) {
    volatile int finishMe;
    pthread_mutex_lock(&This->Private.lock);
    finishMe = This->Private.finishMe;
    pthread_mutex_unlock(&This->Private.lock);
    return finishMe;
};

static void  setServerEnd(struct HttpServer *This) {
    pthread_mutex_lock(&This->Private.lock);
    This->Private.finishMe = 1;
    pthread_mutex_unlock(&This->Private.lock);
};



#define SERVER_PORT  8000

#define TRUE             1
#define FALSE            0

static int createSocket(const char p_ip[], int p_port) {
    int    len, rc, on = 1;
    int    listen_sd = -1;
    int    desc_ready, end_server = FALSE, compress_array = FALSE;
    int    close_conn;
    char   buffer[80];
    struct sockaddr_in6   addr;


    /*************************************************************/
    /* Create an AF_INET6 stream socket to receive incoming      */
    /* connections on                                            */
    /*************************************************************/
    listen_sd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_sd < 0)
    {
      perror("socket() failed");
      exit(-1);
    }

    /*************************************************************/
    /* Allow socket descriptor to be reuseable                   */
    /*************************************************************/
    rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
                    (char *)&on, sizeof(on));
    if (rc < 0)
    {
      perror("setsockopt() failed");
      close(listen_sd);
      exit(-1);
    }

    /*************************************************************/
    /* Set socket to be nonblocking. All of the sockets for      */
    /* the incoming connections will also be nonblocking since   */
    /* they will inherit that state from the listening socket.   */
    /*************************************************************/
    rc = ioctl(listen_sd, FIONBIO, (char *)&on);
    if (rc < 0)
    {
      perror("ioctl() failed");
      close(listen_sd);
      exit(-1);
    }

    /*************************************************************/
    /* Bind the socket                                           */
    /*************************************************************/
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family      = AF_INET6;
    memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    addr.sin6_port        = htons(SERVER_PORT);
    rc = bind(listen_sd,
              (struct sockaddr *)&addr, sizeof(addr));
    if (rc < 0)
    {
      perror("bind() failed");
      close(listen_sd);
      exit(-1);
    }

    /*************************************************************/
    /* Set the listen back log                                   */
    /*************************************************************/
    rc = listen(listen_sd, 32);
    if (rc < 0)
    {
      perror("listen() failed");
      close(listen_sd);
      exit(-1);
    }
    return listen_sd;
}

void clearHttpResponse(struct HttpResponse* This) {
    This->contentLength=-1;
    This->contentType=NULL;
    This->content=NULL;
    This->msg=NULL;
    This->code=0;
}

static void destroyHttpResponse(struct HttpResponse* This) {
    This->contentLength=-1;
    free(This->contentType);
    free(This->msg);
    free(This->content);
}

void sendHttpResponse(struct HttpResponse* This, int p_socket) {
    char buff[MAX_TCP_PACKAGE_SIZE]={0};
    char contentLengthStr[32]={0};

    assert(This->msg);  // expect that the message will be provided

    snprintf(buff, MAX_TCP_PACKAGE_SIZE, "HTTP/1.1 %d %s\x0D\x0A", This->code, This->msg);
    if (This->contentType != NULL) {
        strncat(buff, "Content-Type: ", MAX_TCP_PACKAGE_SIZE-1);
        strncat(buff, This->contentType, MAX_TCP_PACKAGE_SIZE-1);
        strncat(buff, "\x0D\x0A", MAX_TCP_PACKAGE_SIZE-1);
    }

    if (This->contentLength >= 0) {
        snprintf(contentLengthStr, sizeof(contentLengthStr), "%d", This->code, This->contentLength);
        strncat(buff, "Content-Length: ", MAX_TCP_PACKAGE_SIZE-1);
        strncat(buff, contentLengthStr, MAX_TCP_PACKAGE_SIZE-1);
        strncat(buff, "\x0D\x0A", MAX_TCP_PACKAGE_SIZE-1);
    }

    /*************************************************************/
    /* Mark end of header ODOA                                   */
    /*************************************************************/
    strncat(buff, "\x0D\x0A", MAX_TCP_PACKAGE_SIZE-1);

    /*************************************************************/
    /* Add content if exist.                                     */
    /* Assume that there is no NULL character inside             */
    /*************************************************************/
    if (This->content != NULL) {
        strncat(buff, This->content , MAX_TCP_PACKAGE_SIZE);
    }

    send(p_socket, buff, strlen(buff), 0);
}

struct HttpResponse createSimpleHttpResponse(const char* msg, int code) {
    struct HttpResponse response;
    clearHttpResponse(&response);
    response.msg = strdup(msg);
    response.code = code;
    return response;
}

// static const char* HTTP_404_ERROR = "HTTP/1.1 404 Not Found" "\x0D\x0A\x0D\x0A";

static void httpRequestDispacer(struct HttpServer* This, char* p_httpRequestToParse, size_t p_size, int p_new_sd) {

    struct HttpRequest reqest = createHttpRequest(p_httpRequestToParse, p_size);

    struct HttpResponse response = This->onHttpRequest(&reqest);


    sendHttpResponse(&response, p_new_sd);

    destroyHttpResponse(&response);
    destroyHttpRequest(&reqest);
}

/***********************************************************/
/* Default callback                                        */
/***********************************************************/
struct HttpResponse  onHttpRequest(struct HttpRequest* p_request) {
    printf("This is default callback for http server. You have to implement you callback.\n");
    struct HttpResponse response = createSimpleHttpResponse("Please Implement onHttpRequest method", 404);
    return response;
}

static void newConnectionHasCome(struct HttpServer* This, int new_sd) {
    int rc;
    int timeout_ms = 1000;
    printf("new Connection!!!\n");
    const int    nfds = 1;
    struct pollfd fds[nfds];

    fds[0].fd = new_sd;
    fds[0].events = POLLIN;

    /***********************************************************/
    /* Wait for http data...                                   */
    /***********************************************************/
    rc = poll(fds, nfds, timeout_ms);

    /***********************************************************/
    /* Check to see if the poll call failed.                   */
    /***********************************************************/
    if (rc < 0) {
        perror("  client's poll() failed");
        return;
    }

    /***********************************************************/
    /* Check to see if the time out expired.                   */
    /***********************************************************/
    if (rc == 0) {
        printf("Error: dropping connection to client since it is too lazy to send me data.\n");
        return;
    }



    /***********************************************************/
    /* Receive data on This connection                         */
    /***********************************************************/
    char   buffer[MAX_TCP_PACKAGE_SIZE];
    rc = recv(new_sd, buffer, sizeof(buffer), 0);
    if (rc < 0) {
        perror("  recv() failed");
        return;
    }

    /***********************************************************/
    /* Check to see if the connection has been                 */
    /* closed by the client                                    */
    /***********************************************************/
    if (rc == 0)
    {
      printf("  Connection closed\n");
      return;
    }

    /***********************************************************/
    /* Check to see if we received some data                   */
    /***********************************************************/
    if (rc > 0) {
        httpRequestDispacer(This, buffer, rc, new_sd);

    }


}


static void* waitForNewConnectionThread(void *arg) {
    struct HttpServer* This = (struct HttpServer*)arg;
    int    on = 1;
    int    timeout_ms;
    int    new_sd = -1;
    int    rc;
    const int    nfds = 1;
    struct pollfd fds[nfds];

    /*************************************************************/
    /* Initialize the pollfd structure                           */
    /*************************************************************/
    memset(fds, 0 , sizeof(fds));

    /*************************************************************/
    /* Set up the initial listening socket                        */
    /*************************************************************/
    fds[0].fd = This->Private.listen_sd;
    fds[0].events = POLLIN;
    /*************************************************************/
    /* Initialize the timeout to 3 minutes. If no                */
    /* activity after 3 minutes This program will end.           */
    /* timeout value is based on milliseconds.                   */
    /*************************************************************/
    timeout_ms = 500;
    printf("Waiting on clients...\n");
    while (isServerGonigToEnd(This) == 0) {
        rc = poll(fds, nfds, timeout_ms);

        /***********************************************************/
        /* Check to see if the poll call failed.                   */
        /***********************************************************/
        if (rc < 0) {
            perror("  poll() failed"); fflush(stdout);
        }

        /***********************************************************/
        /* Check to see if the  time out expired.                  */
        /***********************************************************/
        if (rc == 0) {
            // printf("  poll() timed out.\n"); fflush(stdout);
        }

        /*****************************************************/
        /* Accept each incoming connection.                  */
        /*****************************************************/
        if (rc > 0) {
            printf("Got client!!!\n"); fflush(stdout);
            new_sd = accept(This->Private.listen_sd, NULL, NULL);
            printf("  accepted!!!\n"); fflush(stdout);
            if (new_sd < 0) {
                perror("  accept() failed"); fflush(stdout);
                continue;
            }
            /*************************************************************/
            /* Set socket to be nonblocking. All of the sockets for      */
            /* the incoming connections will also be nonblocking since   */
            /* they will inherit that state from the listening socket.   */
            /*************************************************************/
            rc = ioctl(new_sd, FIONBIO, (char *)&on);
            if (rc < 0)
            {
              perror("ioctl() failed");
            }

            newConnectionHasCome(This, new_sd);
            close(new_sd);
        }
    }

    return NULL;
}



static void start(struct HttpServer *This, const char p_ip[], int p_port) {
    pthread_t thid;
    This->Private.listen_sd = createSocket(p_ip, p_port);

    if (pthread_create(&This->Private.listenThread, NULL, waitForNewConnectionThread, This) != 0) {
        perror("pthread_create() error");
        exit(1);
    }

    setServerRunning(This, 1);

}

static void stop(struct HttpServer *This) {
    if (!isServerRunning(This)) {
        printf("Error: Nothing to stop. Server is not running\n");
        return;
    }
    setServerEnd(This);
    void *ret;
    if (pthread_join(This->Private.listenThread, &ret) != 0) {
        perror("pthread_create() error");
        exit(3);
    }

    close(This->Private.listen_sd);
    printf("Server thread has been closed\n");
    setServerRunning(This, 0);
}


struct HttpServer createHttpServer() {
    struct HttpServer hs;
    hs.start = &start;
    hs.stop = &stop;
    hs.onHttpRequest = &onHttpRequest;
    hs.Private.finishMe = 0;
    hs.Private.serverRunning = 0;
    return hs;
}

