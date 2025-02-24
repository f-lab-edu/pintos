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

bool is_valid_address(void *ptr) {
  if(ptr != NULL && is_user_vaddr(ptr) == true && pagedir_get_page(thread_current()->pagedir, ptr) != NULL)
    return true;
  return false;
}

void halt(void) {
  shutdown_power_off();
}

void exit(int status) {
  if(status < 0)
    status = -1;
  struct thread *current_thread = thread_current();
  printf("%s: exit(%d)\n", current_thread->name, status);
  current_thread->exit_status = status;

  thread_exit();
}

int exec(const char *cmd_line) {
  if(is_valid_address(cmd_line) == false)
    return -1;
  return process_execute(cmd_line);
}

int wait(int pid) {
  return process_wait(pid);
}

bool create(const char *file, unsigned initial_size) {
  if(is_valid_address(file) == false)
    exit(-1);
  return filesys_create(file, initial_size);
}

bool remove(const char *file) {
  return filesys_remove(file);
}

int open(const char *file) {
  if(is_valid_address(file) == false)
    exit(-1);

  struct file *current_file = filesys_open(file);
  if(current_file == NULL)
    return -1;

  struct thread *current_thread = thread_current();
  if(current_thread->next_fd == -1)
    exit(-1);

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
  if(fd < 0 || fd >= FD_TABLE_MAX_SLOT || thread_current()->fd_table[fd] == NULL)
    exit(-1);
  return file_length(thread_current()->fd_table[fd]);
}

int read(int fd, void *buffer, unsigned size) {
  if(fd < 0 || fd >= FD_TABLE_MAX_SLOT || thread_current()->fd_table[fd] == NULL || is_valid_address(buffer) == false || size < 0)
    exit(-1);

  if(fd == STDIN_FILENO) {
    input_getc();
    return -1;    // todo
  }
  else
    return file_read(thread_current()->fd_table[fd], buffer, size);
}

int write(int fd, const void *buffer, unsigned size) {
  if(is_valid_address(buffer) == false || size < 0)
    exit(-1);

  if(fd == STDOUT_FILENO)
    putbuf(buffer, size);
  else {
    if(fd < 0 || fd >= FD_TABLE_MAX_SLOT || thread_current()->fd_table[fd] == NULL)
      exit(-1);
    return file_write(thread_current()->fd_table[fd], buffer, size);
  }
  return size;
}

void seek(int fd, unsigned poisiton) {
  if(fd < 0 || fd >= FD_TABLE_MAX_SLOT || thread_current()->fd_table[fd] == NULL)
    exit(-1);
  file_seek(thread_current()->fd_table[fd], poisiton);
}
unsigned tell(int fd) {
  if(fd < 0 || fd >= FD_TABLE_MAX_SLOT || thread_current()->fd_table[fd] == NULL)
    exit(-1);
  return file_tell(thread_current()->fd_table[fd]);
}
void close(int fd) {
  struct thread *current_thread = thread_current();
  if(fd < 0 || fd >= FD_TABLE_MAX_SLOT || current_thread->fd_table[fd] == NULL)
    exit(-1);

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
