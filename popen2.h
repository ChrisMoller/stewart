#ifndef POPEN2
#define POPEN2
FILE * popen2(const char *command, const char *type, int * pid);
int pclose2(FILE * fp, pid_t pid);
#endif // POPEN2


