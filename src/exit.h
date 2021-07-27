#ifndef EXIT_H
#define EXIT_H

int errno2exit(void);

/*
 * Terminate the process. Status indicates whether the process terminated
 * successfully or not. 0 indicates successful termination, otherwise the
 * function will indicate an error in the exit status based on the value of
 * errno.
 */
void gdiot_exit(int status);

#endif /* EXIT_H */
