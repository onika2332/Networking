#ifndef BALL_H
#define BALL_H

#include <stdio.h>
#include "point.h"

typedef struct ball{
    Point* center;
    int plus_x;
    int plus_y;   
} Ball;

// Ball in screen is just 'O' character.
Ball* setBall( int cx, int cy, int px, int py) {
    Ball* b = (Ball*)malloc(sizeof(Ball));
    b->center = setPoint(cx, cy);
    b->plus_x = px;
    b->plus_y = py;
    return b;
}
void updatePosition(Ball* b) {
    b->center->x += b->plus_x;
    b->center->y += b->plus_y;
}

#endif
