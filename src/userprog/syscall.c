#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


int write(int fd, void *buffer, int size) {
  putbuf(buffer, size);
  return size;
}

static void syscall_handler(struct intr_frame* f)
{
  int *current_esp = f->esp;
	switch (current_esp[0])
	{
		case SYS_WRITE: write(current_esp[1], current_esp[2], current_esp[3]); break;
		default: printf("Default %d\n",*current_esp);
	}

}
