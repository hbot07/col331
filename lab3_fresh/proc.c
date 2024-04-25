#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"

struct
{
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void trapret(void);

int cpuid()
{
  return 0;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu *
mycpu(void)
{
  return &cpus[0];
}

// Read proc from the cpu structure
struct proc *
myproc(void)
{
  struct cpu *c = mycpu();
  return c->proc;
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc *
allocproc(void)
{
  struct proc *p;
  char *sp;

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if (p->state == UNUSED)
      goto found;

  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  if ((p->offset = kalloc()) == 0)
  {
    p->state = UNUSED;
    return 0;
  }
  p->sz = PGSIZE - KSTACKSIZE;

  sp = (char *)(p->offset + PGSIZE);

  // Allocate kernel stack.
  p->kstack = sp - KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe *)sp;

  sp -= sizeof *p->context;
  p->context = (struct context *)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)trapret;

  return p;
}

// Set up first process.
void pinit(int pol)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  p->policy = pol;
  initproc = p;

  memmove(p->offset, _binary_initcode_start, (int)_binary_initcode_size);
  memset(p->tf, 0, sizeof(*p->tf));

  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;

  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE - KSTACKSIZE;
  p->tf->eip = 0; // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
}

// process scheduler.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  int fore_bg = 0; // tracks ratio of foreground to background process execution.

  struct proc *fg_proc = 0; // Next runnable foreground process
  struct proc *bg_proc = 0; // Next runnable background process

  int counter = 0;

  while (1)
  {
    sti(); // Enable interrupts on this processor.

    if (fg_proc == 0 || fg_proc == &ptable.proc[NPROC]){
      fg_proc = ptable.proc;
    }
    if (bg_proc == 0 || bg_proc == &ptable.proc[NPROC]){
      bg_proc = ptable.proc;
    }
    
    counter = 0;
    while(1){
      if(counter>1){
        fg_proc = 0;
        break;
      }
      if(fg_proc->state == RUNNABLE && fg_proc->policy == 0){
        break;
      }
      fg_proc++;
      if(fg_proc == &ptable.proc[NPROC]){
        fg_proc = ptable.proc;
        counter++;
      }
    }

    counter = 0;
    while(1){
      if(counter>1){
        bg_proc = 0;
        break;
      }
      if(bg_proc->state == RUNNABLE && bg_proc->policy == 1){
        break;
      }
      bg_proc++;
      if(bg_proc == &ptable.proc[NPROC]){
        bg_proc = ptable.proc;
        counter++;
      }
    }

    // Now, decide which process to run based on the scheduler's state and the presence of runnable processes
    if (fore_bg < 9 && fg_proc)
    { // Prefer foreground processes
      p = fg_proc;
      fore_bg++;
      fg_proc++;
    }
    else if (bg_proc)
    { // Time for a background process or no runnable foreground processes
      p = bg_proc;
      fore_bg = 0; // Reset after a background process runs
      bg_proc++;
    }
    else if (fg_proc)
    { // No background processes ready, but there are runnable foreground ones
      p = fg_proc;
      // fore_bg = (fore_bg + 1) % 10;
      fg_proc++;
    }
    else
    {
      // No runnable processes, so skip scheduling
      continue;
    }

    // Run the selected process
    c->proc = p;
    p->state = RUNNING;
    switchuvm(p);                       
    swtch(&(c->scheduler), p->context);
    c->proc = 0;                        // No current process once switched
  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void)
{
  int intena;
  struct cpu *c = mycpu();
  struct proc *p = myproc();

  if (p->state == RUNNING)
    panic("sched running");
  if (readeflags() & FL_IF)
    panic("sched interruptible");
  intena = c->intena;
  swtch(&p->context, c->scheduler);
  c->intena = intena;
}

// Give up the CPU for one scheduling round.
void yield(void)
{
  myproc()->state = RUNNABLE;
  sched();
}

void procdump(void)
{
  static char *states[] = {
      [UNUSED] "unused",
      [EMBRYO] "embryo",
      [RUNNABLE] "runble",
      [RUNNING] "run   ",
  };
  struct proc *p;
  char *state;

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if (p->state == UNUSED)
      continue;
    if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    cprintf("\n");
  }
}