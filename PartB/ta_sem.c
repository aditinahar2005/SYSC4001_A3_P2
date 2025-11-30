#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
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

    sem_t *sem_rubric = sem_open("/rubric_sem", O_CREAT, 0666, 1);
    sem_t *sem_exam = sem_open("/exam_sem", O_CREAT, 0666, 1);
    sem_t *sem_mark = sem_open("/mark_sem", O_CREAT, 0666, 1);

    if (sem_rubric == SEM_FAILED || sem_exam == SEM_FAILED || sem_mark == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    int shmid = shmget(2345, sizeof(SharedData), IPC_CREAT | 0666);
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
        fgets(shared->rubric[i], 32, rf);
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
                    printf("TA %d: reached 9999, exiting\n", id);
                    shmdt(shared);
                    exit(0);
                }

                printf("TA %d reviewing rubric for student %d\n",
                       id, shared->currentStudent);

                for (int q = 0; q < 5; q++) {

                    usleep((rand() % 500 + 500) * 1000);
                    int change = rand() % 2;

                    if (change) {
                        sem_wait(sem_rubric);

                        char *line = shared->rubric[q];
                        int pos = 2;

                        if (line[pos+1] != '\0' && line[pos+1] != '\n') {
                            line[pos+1] = line[pos+1] + 1;
                        }

                        FILE *wf = fopen("rubric.txt", "w");
                        if (wf) {
                            for (int k = 0; k < 5; k++)
                                fputs(shared->rubric[k], wf);
                            fclose(wf);
                        }

                        printf("TA %d updated rubric line %d to %c\n",
                               id, q + 1, line[pos+1]);

                        sem_post(sem_rubric);
                    }
                }

                int picked = 0;

                sem_wait(sem_mark);
                for (int q = 0; q < 5; q++) {
                    if (shared->marked[q] == 0) {
                        shared->marked[q] = 1;
                        picked = q + 1;
                        break;
                    }
                }
                sem_post(sem_mark);

                if (picked != 0) {
                    printf("TA %d marked question %d for student %d\n",
                           id, picked, shared->currentStudent);
                }

                usleep((rand() % 1000 + 1000) * 1000);

                int done = 1;
                for (int k = 0; k < 5; k++) {
                    if (shared->marked[k] == 0)
                        done = 0;
                }

                if (done) {
                    sem_wait(sem_exam);

                    if (shared->examNumber < 20) {
                        shared->examNumber++;
                        sprintf(fname, "exams/%04d.txt", shared->examNumber);
                    } else {
                        sprintf(fname, "exams/9999.txt");
                    }

                    FILE *nf = fopen(fname, "r");
                    if (!nf) {
                        printf("TA %d: could not open %s\n", id, fname);
                        sem_post(sem_exam);
                        shmdt(shared);
                        exit(0);
                    }

                    fscanf(nf, "%d", &shared->currentStudent);
                    fclose(nf);

                    printf("TA %d loaded next exam %s (student %d)\n",
                           id, fname, shared->currentStudent);

                    memset(shared->marked, 0, sizeof(shared->marked));
                    sem_post(sem_exam);
                }
            }
        }
    }

    for (int i = 0; i < numTAs; i++)
        wait(NULL);

    shmdt(shared);
    shmctl(shmid, IPC_RMID, NULL);

    sem_unlink("/rubric_sem");
    sem_unlink("/exam_sem");
    sem_unlink("/mark_sem");

    return 0;
}
