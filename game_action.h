#ifndef GAMEACTION_H
#define GAMEACTION_H
#include "ball.h"
#include <stdio.h>
#include "paddle.h"
#include <time.h>
#include <ncurses.h>
#include <ctype.h>

int checkConfilctWithLeftPaddle(Ball* b, Paddle* left) {
    int floor = left->center->y - left->halfLength;
    int ceiling = left->center->y + left->halfLength;
    if( floor <= b->center->y && b->center->y <= ceiling ) {
        if( b->center->x + b->plus_x <  left->center->x )
            return 1;
    }
    return 0;
}

int checkConfilctWithRightPaddle(Ball* b, Paddle* right) {
    int floor = right->center->y - right->halfLength;
    int ceiling = right->center->y + right->halfLength;
    if( floor <= b->center->y && b->center->y <= ceiling) {
        if( b->center->x + b->plus_x > right->center->x )
            return 1;
    }
    return 0;
}

int checkConflictWithWindow(Ball* b, int cols, int rows) {
    if(b->center->y + b->plus_y < 1 || b->center->y + b->plus_y > rows - 2 )
        return 1;
    return 0;
}


#endif