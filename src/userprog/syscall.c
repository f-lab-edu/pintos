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

int write(int fd, void *buffer, int size) {
  putbuf(buffer, size);
  return size;
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
		case SYS_WRITE: f->eax = write(current_esp[1], current_esp[2], current_esp[3]); break;
		default: printf("Default %d\n",*current_esp);
	}

}
