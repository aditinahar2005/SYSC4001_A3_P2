#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include "shared.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <numTAs>\n", argv[0]);
        return 1;
    }

    int numTAs = atoi(argv[1]);
    if (numTAs < 2) {
        printf("Need at least 2 TAs\n");
        return 1;
    }

    srand(time(NULL));

    int shmid = shmget(1234, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        return 1;
    }

    SharedData *shared = (SharedData *) shmat(shmid, NULL, 0);
    if (shared == (void *) -1) {
        perror("shmat");
        return 1;
    }

    FILE *rf = fopen("rubric.txt", "r");
    if (!rf) {
        perror("rubric.txt");
        return 1;
    }

    for (int i = 0; i < 5; i++) {
        if (!fgets(shared->rubric[i], 32, rf)) {
            shared->rubric[i][0] = '\0';
        }
    }
    fclose(rf);

    shared->examNumber = 1;

    char fname[64];
    sprintf(fname, "exams/%04d.txt", shared->examNumber);
    FILE *ef = fopen(fname, "r");
    if (!ef) {
        perror("first exam");
        return 1;
    }

    fscanf(ef, "%d", &shared->currentStudent);
    fclose(ef);

    memset(shared->marked, 0, sizeof(shared->marked));

    for (int i = 1; i <= numTAs; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            int id = i;

            while (1) {
                if (shared->currentStudent == 9999) {
                    printf("TA %d: reached student 9999, exiting\n", id);
                    shmdt(shared);
                    exit(0);
                }

                printf("TA %d reviewing rubric for student %d\n",
                       id, shared->currentStudent);

                for (int q = 0; q < 5; q++) {
                    usleep((rand() % 500 + 500) * 1000);

                    int change = rand() % 2;
                    if (change && shared->rubric[q][0] != '\0') {
                        if (shared->rubric[q][3] != '\0' &&
                            shared->rubric[q][3] != '\n') {
                            shared->rubric[q][3] = shared->rubric[q][3] + 1;
                        }

                        FILE *wf = fopen("rubric.txt", "w");
                        if (wf) {
                            for (int k = 0; k < 5; k++) {
                                fputs(shared->rubric[k], wf);
                            }
                            fclose(wf);
                        }

                        printf("TA %d updated rubric line %d to %c\n",
                               id, q + 1, shared->rubric[q][3]);
                    }
                }

                int picked = 0;
                for (int q = 0; q < 5; q++) {
                    if (shared->marked[q] == 0) {
                        shared->marked[q] = 1;
                        picked = q + 1;
                        printf("TA %d marked question %d for student %d\n",
                               id, picked, shared->currentStudent);
                        usleep((rand() % 1000 + 1000) * 1000);
                        break;
                    }
                }

                int done = 1;
                for (int k = 0; k < 5; k++) {
                    if (shared->marked[k] == 0) done = 0;
                }

                if (done) {
                    if (shared->examNumber < 20) {
                        shared->examNumber++;
                        sprintf(fname, "exams/%04d.txt", shared->examNumber);
                    } else if (shared->examNumber == 20) {
                        sprintf(fname, "exams/9999.txt");
                        shared->examNumber++;
                    } else {
                        printf("TA %d: no more exams, exiting\n", id);
                        shmdt(shared);
                        exit(0);
                    }

                    FILE *nf = fopen(fname, "r");
                    if (!nf) {
                        printf("TA %d: could not open %s, exiting\n", id, fname);
                        shmdt(shared);
                        exit(0);
                    }

                    fscanf(nf, "%d", &shared->currentStudent);
                    fclose(nf);

                    printf("TA %d loaded next exam file %s (student %d)\n",
                           id, fname, shared->currentStudent);

                    memset(shared->marked, 0, sizeof(shared->marked));
                }
            }
        }
    }

    for (int i = 0; i < numTAs; i++) {
        wait(NULL);
    }

    shmdt(shared);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
