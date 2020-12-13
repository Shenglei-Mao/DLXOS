//
//	synch.c
//
//	Routines for synchronization
//
//

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"

//----------------------------------------------------------------------
//
//	SemInit
//
//	Initialize a semaphore to a particular value.  This just means
//	initting the process queue and setting the counter.
//
//----------------------------------------------------------------------
void
SemInit (Sem *sem, int count)
{
    QueueInit (&sem->waiting);
    sem->count = count;
}

//----------------------------------------------------------------------
//
//	SemWait
//
//	Wait on a semaphore.  As described in Section 6.4 of _OSC_,
//	we decrement the counter and suspend the process if the
//	semaphore's value is less than 0.  To ensure atomicity,
//	interrupts are disabled for the entire operation.  Note that,
//	if the process is put to sleep, interrupts will be OFF when
//	it returns from sleep.  Thus, we enable interrupts at the end of
//	the routine.
//
//----------------------------------------------------------------------
void
SemWait (Sem *sem)
{
    Link	*l;
    int		intrval;

    intrval = DisableIntrs ();
    dbprintf ('I', "Old interrupt value was 0x%x.\n", intrval);
    dbprintf ('s', "Proc 0x%x waiting on sem 0x%x, count=%d.\n", (int)currentPCB,
	      (int)sem, sem->count);
    sem->count -= 1;
    if (sem->count < 0) {
	l = QueueAllocLink ();
	QueueLinkInit (l, (void *)currentPCB);
	dbprintf ('s', "Suspending current proc (0x%x).\n", (int)currentPCB);
	QueueInsertLast (&sem->waiting, l);
	ProcessSleep ();
    }
    RestoreIntrs (intrval);
}

//----------------------------------------------------------------------
//
//	SemSignal
//
//	Signal on a semaphore.  Again, details are in Section 6.4 of
//	_OSC_.
//
//----------------------------------------------------------------------
void
SemSignal (Sem *sem)
{
    Link	*l;
    int		intrs;

    intrs = DisableIntrs ();
    dbprintf ('s', "Signalling on sem 0x%x, count=%d.\n", (int)sem, sem->count);
    sem->count += 1;
    if (sem->count <= 0) {
	l = QueueFirst (&sem->waiting);
	QueueRemove (l);
	dbprintf ('s', "Waking up PCB 0x%x.\n", (int)(l->object));
	ProcessWakeup ((PCB *)(l->object));
	QueueFreeLink (l);
    }
    RestoreIntrs (intrs);
}

//----------------------------------------------------------------------
//
//	Your assignment is to implement locks and condition variables
//	using Mesa-style monitor synchronization.
//
//----------------------------------------------------------------------
void
LockInit (Lock *lock)
{
}

void
LockAcquire (Lock *lock)
{
}

void	LockRelease (Lock *lock)
{
}

void	CondInit (Cond *cond, Lock *lock)
{
}

void	CondWait (Cond *cond)
{
}

void	CondSignal (Cond *cond)
{
}

// Where CondSignal restarts one process waiting on the condition
// variable, CondBroadcast restarts ALL processes waiting on this
// condition variable.
void	CondBroadcast (Cond *cond)
{
}

