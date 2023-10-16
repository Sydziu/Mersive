/*******************************************************
 * Copyright (C) 2023
 *
 *  Created on: Oct 16, 2023
 *      Author: p.sydow
 *
 *******************************************************/
#pragma once

#include <stdio.h>
#include <pthread.h>


struct SingleRecord {
    char* content;
    char* url;
    int contentLength;
    char* contentType;
};

#define MAX_KEYS 100

/*************************************************************/
/* The interface for the KeyStorage                          */
/*************************************************************/
struct KeyStorage {
    int (*addKey)(struct KeyStorage *This, struct SingleRecord *p_record);
    struct SingleRecord* (*getKey)(struct KeyStorage *This, const char* key);
    int (*removeKey)(struct KeyStorage *This, const char* key);
    int (*getSize)(struct KeyStorage *This);
    void (*dumpKeys)(struct KeyStorage *This);
    struct {
        struct SingleRecord keys[MAX_KEYS];
        volatile int size;
        //pthread_mutex_t lock; // we cannot keep it here. It is not copyable
    } Private;
};

extern void destroyKeyStorage(struct KeyStorage *This);
extern struct KeyStorage createKeyStorage();
extern void destroySingleRecord(struct SingleRecord* This);


