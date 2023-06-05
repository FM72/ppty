#include <string.h>
#include <util.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#define UNUSED(x) (void)(x)

// create a pty and return the master and slave file descriptors
// additionally turn off echo on the master side of the pty
int create_pty(int* p_master, int* p_slave)
{
  int result = 0;

  struct termios tty;
  // <string.h> : void *memset(void *s, int c, size_t n);
  memset(&tty, 0, sizeof(struct termios));

  // <util.h> : int openpty(int *amaster, int *aslave, char *name, const struct termios *termp, const struct winsize *winp);
  result = openpty(p_master, p_slave, NULL, NULL, NULL);
  if (result)
    return result;

  // <termios.h>, <unistd.h> : int tcgetattr(int fd, struct termios *termios_p);
  result = tcgetattr(*p_master, &tty);
  if (result)
    return 1;

  tty.c_lflag &= ~ECHO;

  // <termios.h>, <unistd.h> : int tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
  result = tcsetattr(*p_master, TCSANOW, &tty);
  if (result)
    return 1;

  return 0;
}

// exec a command on the slave side of the pty
// this command reads from STDIN_FILENO and writes to the slave side of the pty
int fork_and_exec_on_slave(int master, int slave, char* argv[])
{
  int result = 0;

  pid_t pid = 0;

  // <unistd.h> : pid_t fork(void);
  pid = fork();
  if (pid == -1)
    return 1;

  if (pid == 0)
  {
    // child

    // <unistd.h> : int close(int fd);
    result = close(master);
    if (result)
      return 1;

    // <unistd.h> : int dup2(int oldfd, int newfd);
    result = dup2(slave, 1);
    if (result == -1)
      return 1;
    // <unistd.h> : int dup2(int oldfd, int newfd);
    result = dup2(slave, 2);
    if (result == -1)
      return 1;

    // <unistd.h> : int close(int fd);
    result = close(slave);
    if (result)
      return 1;

    result = execvp(argv[1], &argv[1]);
    return result;
  }
  else
  {
    // parent

    // <unistd.h> : int close(int fd);
    result = close(slave);
    if (result)
      return 1;

    return 0; 
  }
}

// write a buffer into a file descriptor
// returns 0 on success or any other value on error 
int write_buffer(int fd, const void* buffer, ssize_t size)
{
  ssize_t bytes_written = 0;
  while (bytes_written < size)
  {
    // unistd.h : ssize_t write(int fd, const void *buf, size_t count);
    // size - bytes_written is >0, so the cast to size_t is safe
    ssize_t result = write(fd, buffer + bytes_written, size - bytes_written);
    if (result <= 0)
      return 1;

    bytes_written += result;
  }
  return 0;
}

// continuously read from master and write to stdout
int read_from_master_and_write_to_stdout(int master)
{
  int result = 0;

  char buffer[1024];
  // <string.h> : void *memset(void *s, int c, size_t n);
  memset(buffer, 0, 1024);
  ssize_t bytes_read = 0;

  while(1)
  {
      // unistd.h : ssize_t read(int fd, void *buf, size_t count);
      bytes_read = read(master, buffer, 1024);
      if (bytes_read < 0)
        return 1;
      else if (bytes_read == 0)
        return 0;

      result = write_buffer(STDOUT_FILENO, buffer, bytes_read);
      if (result)
        return result;
  }
}

int main(int argc, char *argv[])
{
  UNUSED(argc);

  int result = 0;

  int master = -1;
  int slave = -1;

  result = create_pty(&master, &slave);
  if (result)
    return result;
  
  result = fork_and_exec_on_slave(master, slave, argv);
  if (result)
    return result;

  return read_from_master_and_write_to_stdout(master);
}
