#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "threads/vaddr.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


void halt(void) {
  shutdown_power_off();
}

void exit(int status) {
  struct thread *current_thread = thread_current();
  printf("%s: exit(%d)\n", current_thread->name, status);
  current_thread->exit_status = status;

  thread_exit();
}

int exec(const char *cmd_line) {
  return process_execute(cmd_line);
}

int wait(int pid) {
  return process_wait(pid);
}

bool create(const char *file, unsigned initial_size) {
  if(file == NULL)
    return false;

  return filesys_create(file, initial_size);
}

bool remove(const char *file) {
  return filesys_remove(file);
}

int open(const char *file) {
  if(file == NULL)
    return -1;

  struct file *current_file = filesys_open(file);
  if(current_file == NULL)
    return -1;

  struct thread *current_thread = thread_current();
  if(current_thread->next_fd == -1) {
    printf("out of fd table\n");
    return -1;
  }

  current_thread->fd_table[current_thread->next_fd] = current_file;
  int current_fd = current_thread->next_fd;

  while(true) {
    if(current_thread->fd_table[current_thread->next_fd] == NULL)
      break;
    ++current_thread->next_fd;
    if(current_thread->next_fd == FD_TABLE_MAX_SLOT) {
      current_thread->next_fd = -1;
      break;
    }
  }

  return current_fd;
}

int filesize(int fd) {
  return file_length(thread_current()->fd_table[fd]);
}

int read(int fd, void *buffer, unsigned size) {
  if(fd == STDIN_FILENO) {
    input_getc();
    return -1;    // todo
  }
  else {
    if(thread_current()->fd_table[fd] == NULL)
      return -1;
    return file_read(thread_current()->fd_table[fd], buffer, size);
  }
}

int write(int fd, const void *buffer, unsigned size) {
  if(fd == STDOUT_FILENO)
    putbuf(buffer, size);
  else {
    if(thread_current()->fd_table[fd] == NULL)
      return -1;
    return file_write(thread_current()->fd_table[fd], buffer, size);
  }
  return size;
}

void seek(int fd, unsigned poisiton) {
  if(thread_current()->fd_table[fd] == NULL)
    return;
  file_seek(thread_current()->fd_table[fd], poisiton);
}
unsigned tell(int fd) {
  if(thread_current()->fd_table[fd] == NULL)
    return;
  return file_tell(thread_current()->fd_table[fd]);
}
void close(int fd) {
  struct thread *current_thread = thread_current();
  if(current_thread->fd_table[fd] == NULL)
    return;

  file_close(current_thread->fd_table[fd]);

  current_thread->fd_table[fd] = NULL;
  current_thread->next_fd = fd;
}


static void syscall_handler(struct intr_frame* f)
{
  int *current_esp = f->esp;

  /* validation check */
  if(is_user_vaddr(current_esp) == false)
    return; 

  switch (current_esp[0])
  {
    case SYS_HALT: halt(); break;
    case SYS_EXIT: exit(current_esp[1]); break;
    case SYS_EXEC: f->eax = exec(current_esp[1]); break;
    case SYS_WAIT: f->eax = wait(current_esp[1]); break;
    case SYS_CREATE: f->eax = create(current_esp[1], current_esp[2]); break;
    case SYS_REMOVE: f->eax = remove(current_esp[1]); break;
    case SYS_OPEN: f->eax = open(current_esp[1]); break;
    case SYS_FILESIZE: f->eax = filesize(current_esp[1]); break;
    case SYS_READ: f->eax = read(current_esp[1], current_esp[2], current_esp[3]); break;
    case SYS_WRITE: f->eax = write(current_esp[1], current_esp[2], current_esp[3]); break;
    case SYS_SEEK: seek(current_esp[1], current_esp[2]); break;
    case SYS_TELL: f->eax = tell(current_esp[1]); break;
    case SYS_CLOSE: close(current_esp[1]); break;

    default: printf("Default: %d\n",current_esp[0]);
  }
}
