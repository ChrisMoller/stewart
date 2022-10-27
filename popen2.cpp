/***
    This file is part of the Stewart platform simulator.

    Copyright 2022 C. H. L. Moller

Stewart is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

Stewart is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
Stewart. If not, see <https://www.gnu.org/licenses/>.
 ***/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <string>
#include <sstream>

using namespace std;

#define READ   0
#define WRITE  1
FILE *
popen2(const char *command, const char *type, int * pid)
{
  pid_t child_pid;
  int fd[2];
  pipe(fd);

  if((child_pid = fork()) == -1) {
    perror("fork");
    exit(1);
  }

  /* child process */
  if (child_pid == 0) {
    if (type == "r") {
      //Close the READ end of the pipe since the child's fd is write-only
      close(fd[READ]);
      dup2(fd[WRITE], 1); //Redirect stdout to pipe
    }
    else {
      //Close the WRITE end of the pipe since the child's fd is read-only
      close(fd[WRITE]);
      dup2(fd[READ], 0);   //Redirect stdin to pipe
    }

    //Needed so negative PIDs can kill children of /bin/sh
    setpgid(child_pid, child_pid);
    execl("/bin/sh", "/bin/sh", "-c", command, NULL);
    exit(0);
  }
  else {
    if (type == "r") {
      //Close the WRITE end of the pipe since parent's fd is read-only
      close(fd[WRITE]);
    }
    else {
      //Close the READ end of the pipe since parent's fd is write-only
      close(fd[READ]);
    }
  }

  if (pid) *pid = child_pid;

  if (type == "r") {
    return fdopen(fd[READ], "r");
  }

  return fdopen(fd[WRITE], "w");
}

int
pclose2(FILE * fp, pid_t pid)
{
  int stat;

  fclose(fp);
  while (waitpid(pid, &stat, 0) == -1) {
    if (errno != EINTR) {
      stat = -1;
      break;
    }
  }

  return stat;
}

#if 0
int
main()
{
  int pid;
  string command = "ping 8.8.8.8"; 
  FILE * fp = popen2(command, "r", pid);
  char command_out[100] = {0};
  stringstream output;

  // Using read() so that I have the option of using select()
  // if I want non-blocking flow
  while (read(fileno(fp), command_out, sizeof(command_out)-1) != 0) {
    output << string(command_out);
    kill(-pid, 9);
    memset(&command_out, 0, sizeof(command_out));
  }

  string token;
  while (getline(output, token, '\n'))
    printf("OUT: %s\n", token.c_str());

  pclose2(fp, pid);

  return 0;
}
#endif

