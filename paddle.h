#ifndef PADDLE_H
#define PADDLE_H
#include "point.h"
#include <stdio.h>

// we install paddle is a vertically-appearance string like "-----" or "====="
typedef struct paddle {
    int halfLength; // 1/2 chieu dai
    Point* center;
    /* A paddle is a rectangle. Rectangle have 4 point (x1,y1) (x1+a,y1) (x1+a, y1-b) (x1,y1-b)
    in this case, x1 is left, y1 is top, x1 +a is right, y1-b is bottom*/
} Paddle;

Paddle* setPaddle(int cx, int cy, int halfLength) {
    Paddle* p = (Paddle*)malloc(sizeof(Paddle));
    p->center = setPoint(cx, cy);
    p->halfLength = halfLength;
    return p;
}

void displace( Paddle* p, int step, int rows) { 
    if( step >= 0) { // go down
        if( p->center->y + p->halfLength <= rows - 2) {
            p->center->y += step;
        }
    } else { // go up
        if( p->center->y - p->halfLength >= 1) {
            p->center->y += step;
        }
    }
    
}

#endif
