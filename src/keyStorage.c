/*******************************************************
 * Copyright (C) 2023
 *
 *  Created on: Oct 16, 2023
 *      Author: p.sydow
 *
 *******************************************************/


#include <stdlib.h>
#include <string.h>
#include "keyStorage.h"



/*************************************************************/
/* Helpers functions                                         */
/*************************************************************/

static struct SingleRecord* findKeyThreadUnsafe(struct KeyStorage *This, const char* p_url) {
    for (int inx = 0; inx < MAX_KEYS; inx++) {
        if (This->Private.keys[inx].url == NULL) {
            continue;
        }
        if (strcmp(This->Private.keys[inx].url, p_url) == 0) {
            return &This->Private.keys[inx];
        }
    }
    return NULL;
}

struct SingleRecord* findEmptyKeyThreadUnsafe(struct KeyStorage *This) {
    for (int inx = 0; inx < MAX_KEYS; inx++) {
        if (This->Private.keys[inx].url == NULL) {
            return &This->Private.keys[inx];
        }
    }
    return NULL;
}
//
//void addOrUpdateKey(char* content, int contentLength, char* contentType, char* url) {
//    pthread_mutex_lock(&lock);
//    /*************************************************************/
//    /* Try to find existing                                      */
//    /*************************************************************/
//    struct SingleRecord* item = findKeyThreadUnsafe(url);
//
//    if (item == NULL) {
//        /*************************************************************/
//        /* Try to find empty place to store data                     */
//        /*************************************************************/
//        item = findEmptyKeyThreadUnsafe(url);
//        if (item == NULL) {
//            printf("No free space to add key: %S\n", url);
//        } else {
//            printf("Update existing key: %S\n", url);
//        }
//    } else {
//        printf("Update Key: %s\n", url);
//    }
//
//
//
//    pthread_mutex_unlock(&lock);
//}

/*************************************************************/
/* The memers of interface                                   */
/*************************************************************/
static int addKey(struct KeyStorage *This, struct SingleRecord *p_record) {
    pthread_mutex_lock(&This->Private.lock);
    struct SingleRecord * placeToStore;

    /*************************************************************/
    /* -Try to find already existing key                         */
    /*************************************************************/
    placeToStore =  findKeyThreadUnsafe(This, p_record->url);

    /*************************************************************/
    /* -Try to find empty place in tab                           */
    /*************************************************************/
    if (placeToStore == NULL) {
        placeToStore = findEmptyKeyThreadUnsafe(This);
    }
    if (placeToStore == NULL) {
        return -1;  // TODO(p.sydow): return enum indicate what is the problem
    }


    placeToStore->content = strdup(p_record->content);
    placeToStore->contentType = strdup(p_record->contentType);
    placeToStore->url = strdup(p_record->url);
    placeToStore->contentLength = p_record->contentLength;
    pthread_mutex_unlock(&This->Private.lock);
    return 0;
}

static struct SingleRecord* getKey(struct KeyStorage *This, const char* p_key) {
    pthread_mutex_lock(&This->Private.lock);
    struct SingleRecord *  theItem =  findKeyThreadUnsafe(This, p_key);


    pthread_mutex_unlock(&This->Private.lock);
    return theItem;
}

static int removeKey(struct KeyStorage *This, const char* p_key) {
    pthread_mutex_lock(&This->Private.lock);
    int ret = 0;
    struct SingleRecord *  theItem =  findKeyThreadUnsafe(This, p_key);
    if (theItem != NULL) {
        destroySingleRecord(theItem);
    } else {
        // Unable to delete record since it is not found
        ret = -1;  // TODO(p.sydow) use enum to indicate error
    }
    pthread_mutex_unlock(&This->Private.lock);
    return ret;
}


/*************************************************************/
/* The interface for the KeyStorage                          */
/*************************************************************/

void destroySingleRecord(struct SingleRecord* This) {
    free(This->content); This->content = NULL;
    free(This->contentType); This->contentType = NULL;
    free(This->url); This->url = NULL;
}

struct KeyStorage createKeyStorage() {
    struct KeyStorage ret;
    memset(ret.Private.keys, 0 , sizeof(ret.Private.keys));
    ret.addKey = addKey;
    ret.getKey = getKey;
    ret.removeKey = removeKey;
    return ret;
};

void destroyKeyStorage(struct KeyStorage *This) {
    pthread_mutex_lock(&This->Private.lock);
    for (int i=0; i<MAX_KEYS;i++) {
        destroySingleRecord(&This->Private.keys[i]);
    }
    pthread_mutex_unlock(&This->Private.lock);
}


