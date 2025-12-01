#ifndef SHARED_H
#define SHARED_H

typedef struct {
    char rubric[5][32];
    int currentStudent;
    int marked[5];
    int examNumber;
} SharedData;

#endif
