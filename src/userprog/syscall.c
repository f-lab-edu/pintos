#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "threads/vaddr.h"
#include "devices/shutdown.h"

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

pid_t exec(const char *cmd_line) {
  ;
}

int wait(pid_t pid) {
  ;
}

bool create(const char *file, unsigned initial_size) {
  ;
}

bool remove(const char *file) {
  ;
}

int open(const char *file) {
  ;
}

int filesize(int fd) {
  ;
}

int read(int fd, void *buffer, unsigned size) {
  ;
}

int write(int fd, const void *buffer, unsigned size) {
  putbuf(buffer, size);
  return size;
}

void seek(int fd, unsigned poisiton) {
  ;
}
unsigned tell(int fd) {
  ;
}
void close(int fd) {
  ;
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
