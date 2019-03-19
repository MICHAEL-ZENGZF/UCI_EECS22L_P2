#include<string.h>
#include"jsmn.h"
#include<stdlib.h>
#include"vectorStr.h"
#include"map.h"
#include"util.h"
#include"queueChat.h"

#ifndef DATABASE_H
#define DATABASE_H

//initialize database
void database_intialize();

//add a user to the database
void database_add_user( char* user_name, char* user_password, int port, bool user_online_status);

//reset the password of a user
void database_set_passwd(char* user, char* passwd);

//set the host address for user
void database_set_host(char* user, char* host);

//reset the port that a user uses to listen to other users
void database_set_port(char* user, int port);

//add/delete a friend to a user`s friend list
bool database_add_friend(char* user, char* friend);
bool database_delete_friend(char* user, char *friend);

//add a message to a user`s message queue
void database_add_msg(char* user, char* msg, char *srcUser);

//add a challenger to a user`s challenger queue
void database_add_challenger(char* user, char* challenger);

//set the online status of a user
void database_set_onlineStatus(char* user, bool onlineStatus);

//get the message queue of a user
QueueChat* database_get_msgQueue(char* user);

//get the next challenger
QNodeChallenger database_get_nextChallenger(char* user);

//get password
char* database_get_password(char* user);

//read from function name
char* database_get_host(char* user);

//same as function name
bool database_get_onlineStatus(char* user);

//returns true if user does not exist
bool database_isUserExist(char* user);

//get friends list
vectorStr database_get_friends(char* user);

//get port
int database_get_port(char* user);

//just a test function
void test_database();

#endif