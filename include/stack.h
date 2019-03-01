#include<string.h>
#include"jsmn.h"
#include"struct.h"
#include<stdio.h>
#include<stdlib.h>
#ifndef STACK_H
#define STACK_H

Node* stack_newNode(char *new_log);
int stack_isEmpty(Node *node);
void stack_push(Node** head_ref, char* new_log);
void stack_pop(Node** head_ref, char* ret_str);
void stack_peek(Node* top, char *ret_str) ;
void move2string(char* str_move, Move *move);
Move string2move(char* str_move);

//Print Moves Log to MovesLog.txt
void stack_print_log(Node** head_ref);


#endif