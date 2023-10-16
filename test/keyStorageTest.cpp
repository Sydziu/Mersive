#include "../src/keyStorage.c"
/*******************************************************
 * Copyright (C) 2020 Ftd Aero
 *
 * RequestParsing.c
 *
 *  Created on: Oct 14, 2023
 *      Author: p.sydow
 *
 *******************************************************/

#include <string>
#include "gtest/gtest.h"


TEST(keyStorage, add_get_remove) {
    struct KeyStorage keyStg = createKeyStorage();

    struct SingleRecord record;
    memset(&record, 0, sizeof(record));

    /*************************************************************/
    /* Create and save first key                                 */
    /*************************************************************/
    record.content = strdup("content1");
    record.contentLength = 13;
    record.contentType = strdup("content-type1");
    record.url = strdup("url1");
    keyStg.addKey(&keyStg, &record);
    destroySingleRecord(&record);

    /*************************************************************/
    /* Create and save second key                                */
    /*************************************************************/
    record.content = strdup("content2");
    record.contentLength = 14;
    record.contentType = strdup("content-type2 ");
    record.url = strdup("url2");
    keyStg.addKey(&keyStg, &record);
    destroySingleRecord(&record);

    /*************************************************************/
    /* Retrieve first keys                                       */
    /*************************************************************/
    struct SingleRecord* storedKey;
    storedKey = keyStg.getKey(&keyStg, "url1");
    ASSERT_TRUE(storedKey != NULL);
    EXPECT_EQ(std::string(storedKey->content), "content1");
    EXPECT_EQ(std::string(storedKey->contentType), "content-type1");
    EXPECT_EQ(std::string(storedKey->url), "url1");

    storedKey = keyStg.getKey(&keyStg, "url2");
    ASSERT_TRUE(storedKey);
    EXPECT_EQ(std::string(storedKey->content), "content2");
    EXPECT_EQ(std::string(storedKey->contentType), "content-type2 ");
    EXPECT_EQ(std::string(storedKey->url), "url2");

    /*************************************************************/
    /* Remove key                                                */
    /*************************************************************/
    EXPECT_EQ(keyStg.removeKey(&keyStg, "url1"), 0);


    /*************************************************************/
    /* Retrieve unexisting key                                   */
    /*************************************************************/
    storedKey = keyStg.getKey(&keyStg, "url1");
    ASSERT_TRUE(storedKey == NULL);



    EXPECT_NE(keyStg.removeKey(&keyStg, "url1"), 0);
    destroyKeyStorage(&keyStg);
    EXPECT_EQ(1, 1);
}
