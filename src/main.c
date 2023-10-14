/*******************************************************
 * Copyright (C) 2023 Ftd Aero
 *
 *      Author: p.sydow
 *
 *******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

// Test scenario:
//     1. socket already in use
//     2. link disappear

// Using poll() instead of select()
// https://www.ibm.com/docs/en/i/7.1?topic=designs-using-poll-instead-select

struct HttpServer {
    void (*start)(struct HttpServer *this, const char p_ip[], uint16_t p_port);
    void (*stop)(struct HttpServer *this);
    struct {
        pthread_mutex_t lock;
        volatile int finishMe;
        volatile int serverRunning;
        int listen_sd;
        pthread_t listenThread;
    }private;

};


static int isServerRunning(struct HttpServer *this) {
    volatile int serverRunning;
    pthread_mutex_lock(&this->private.lock);
    serverRunning = this->private.serverRunning;
    pthread_mutex_unlock(&this->private.lock);
    return serverRunning;
};

static void setServerRunning(struct HttpServer *this, int p_value) {
    pthread_mutex_lock(&this->private.lock);
    this->private.serverRunning = p_value;
    pthread_mutex_unlock(&this->private.lock);
};

static int isServerGonigToEnd(struct HttpServer *this) {
    volatile int finishMe;
    pthread_mutex_lock(&this->private.lock);
    finishMe = this->private.finishMe;
    pthread_mutex_unlock(&this->private.lock);
    return finishMe;
};

static void  setServerEnd(struct HttpServer *this) {
    pthread_mutex_lock(&this->private.lock);
    this->private.finishMe = 1;
    pthread_mutex_unlock(&this->private.lock);
};

void* thread(void *arg) {
    while (1) {
        sleep(2);
        printf("threed loop\n");
        fflush(stdout);
    }
}

#define SERVER_PORT  5000

#define TRUE             1
#define FALSE            0

static int createSocket(const char p_ip[], uint16_t p_port) {
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

void checkNewClients(int listen_sd) {
    int    timeout_ms;
    int    new_sd = -1;
    int    rc;
    struct pollfd fds[1];
    int    nfds = 1;
    /*************************************************************/
    /* Initialize the pollfd structure                           */
    /*************************************************************/
    memset(fds, 0 , sizeof(fds));

    /*************************************************************/
    /* Set up the initial listening socket                        */
    /*************************************************************/
    fds[0].fd = listen_sd;
    fds[0].events = POLLIN;
    /*************************************************************/
    /* Initialize the timeout to 3 minutes. If no                */
    /* activity after 3 minutes this program will end.           */
    /* timeout value is based on milliseconds.                   */
    /*************************************************************/
    timeout_ms = 2000;
    printf("Waiting on clients...\n");
    while(1) {
        rc = poll(fds, nfds, timeout_ms);

        /***********************************************************/
        /* Check to see if the poll call failed.                   */
        /***********************************************************/
        if (rc < 0) {
            perror("  poll() failed");
        }

        /***********************************************************/
        /* Check to see if the  time out expired.                  */
        /***********************************************************/
        if (rc == 0) {
            printf("  poll() timed out.\n");
        }

        /*****************************************************/
        /* Accept each incoming connection.                  */
        /*****************************************************/
        if (rc > 0) {
            printf("Got client!!!\n");
            new_sd = accept(listen_sd, NULL, NULL);
            printf("  accepted!!!\n");
            if (new_sd < 0) {
                perror("  accept() failed");
                continue;
            }
            close(new_sd);
        }
    }

    printf("Done.\n");

}


static void* waitForNewConnectionThread(void *arg) {
    struct HttpServer* this = (struct HttpServer*)arg;

    while (isServerGonigToEnd(this) == 0) {
        sleep(2);
        printf("thread loop\n");
        fflush(stdout);
    }
    return NULL;
}



static void start(struct HttpServer *this, const char p_ip[], uint16_t p_port) {
    pthread_t thid;
    this->private.listen_sd = createSocket(p_ip, p_port);

    if (pthread_create(&this->private.listenThread, NULL, waitForNewConnectionThread, this) != 0) {
        perror("pthread_create() error");
        exit(1);
    }

    setServerRunning(this, 1);

}

static void stop(struct HttpServer *this) {
    if (!isServerRunning(this)) {
        printf("Error: Nothing to stop. Server is not running\n");
        return;
    }
    setServerEnd(this);
    void *ret;
    if (pthread_join(this->private.listenThread, &ret) != 0) {
        perror("pthread_create() error");
        exit(3);
    }

    close(this->private.listen_sd);
    printf("Server thread has been closed\n");
    setServerRunning(this, 0);
}


struct HttpServer createHttpServer() {
    struct HttpServer hs;
    hs.start = &start;
    hs.stop = &stop;
    hs.private.finishMe = 0;
    hs.private.serverRunning = 0;
    return hs;
}


//void testMe() {
//    struct timeval tv;
//    fd_set readfds;
//
//    tv.tv_sec = 1;
//    tv.tv_usec = 0;
//
//    int s, s_remote;
//    struct sockaddr_in remote;
//    struct sockaddr_un local;
//
//    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
//      perror("socket");
//      exit(1);
//    }
//
//    FD_ZERO(&readfds);
//    FD_SET(s, &readfds);
//
//    if (select(s+1, &readfds, NULL, NULL, &tv) > 0) {
//
//      printf("Waiting for a connection...\n");
//
//      memset(&local, 0, sizeof(local));
//      local.sun_family = AF_UNIX;
//      strcpy(local.sun_path, SOCK_PATH);
//      unlink(local.sun_path);
//
//      if (bind(s, (struct sockaddr *)&local, sizeof(local)) == -1) {
//        perror("UnixSocketClient :: error bind.\n");
//        close(s);
//        return -1;
//      }
//
//      if (listen(s, 5) == -1) {
//        perror("UnixSocketClient :: error listen.\n");
//        close(s);
//        return -1;
//      }
//
//      socklen_t remote_len = sizeof(remote);
//      printf("Accept :\n\r");
//
//      if ((s_remote = accept(s, (struct sockaddr *)&remote, &remote_len)) == -1) {
//        perror("UnixSocket :: error accept.\n");
//        return -1;
//      }
//      printf("Client accepted\n\r");
//    }
//}
static void chomp(char* line) {
    int chars_read = strlen(line);
    if(chars_read > 0 && line[chars_read-1] == '\n') {
        line[chars_read-1] = '\0';
        // special care for windows line endings:
        if(chars_read > 1 && line[chars_read-2] == '\r') line[chars_read-2] = '\0';
    }
}

int main() {
    char *line = NULL;
    size_t len = 0;
    struct HttpServer server = createHttpServer();
    server.start(&server, "0.0.0.0", 5000);
    int finishMe = 0;
    while (finishMe == 0) {
        printf("[PS]> ");  // print prompt
        int lineSize = getline(&line, &len, stdin);
        chomp(line);
        //*********************************
        //      exit command
        //*********************************

        if ((strcmp(line, "exit") == 0) || (strcmp(line, "quit") == 0)) {
            finishMe = 1;
            server.stop(&server);
        } else if (strcmp(line, "") == 0){
            // nothing to do;
        }else {
            printf("Error: Unknown command: '%s'.\n", line);
        }

        free(line);
        line = NULL;
        len = 0;
    }



//    pthread_t thid;
//    void *ret;
//
//    if (pthread_create(&thid, NULL, thread, "thread 1") != 0) {
//        perror("pthread_create() error");
//        exit(1);
//    }
//
//    if (pthread_join(thid, &ret) != 0) {
//       perror("pthread_create() error");
//       exit(3);
//     }


//    int soc = createSocket("0.0.0.0", 5000);
//    checkNewClients(soc);
//    printf("Done.");
    return 0;
}
