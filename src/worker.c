#include <proto/exec.h>
#include <proto/dos.h>
#include <clib/alib_protos.h>

#include <exec/types.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <dos/dos.h>

#define NO_SYSBASE
#include "compiler.h"
#include "mydev.h"
#include "debug.h"
#include "worker.h"

#define CMD_TERM     0x7ff0

struct InitData
{
  ULONG           initSigMask;
  struct Task    *initTask;
  struct DevBase *base;
};

static const char WorkerTaskName[] = MYDEV_WORKER ".task";

static struct InitData * worker_startup(void)
{
  /* retrieve global sys base */
  struct Library *SysBase = *((struct Library **)4);

  struct Task *task = FindTask(NULL);

  return (struct InitData *)task->tc_UserData;
}

#define SysBase base->sysBase

static SAVEDS ASM void worker_main(void)
{
  struct IOStdReq *ior;
  struct MsgPort *port;

  /* retrieve dev base stored in user data of task */
  struct InitData *id = worker_startup();
  struct DevBase *base = id->base;
  D(("Task: id=%08lx base=%08lx\n", id, base));

  /* create worker port */
  port = CreateMsgPort();

  /* call user init */
  if(port != NULL) {
    if(!mydev_worker_init(base)) {
      /* user aborted worker */
      DeleteMsgPort(port);
      port = NULL;
    }
  }

  /* setup port or NULL and trigger signal to caller task */
  base->workerPort = port;
  D(("Task: signal task=%08lx mask=%08lx\n", id->initTask, id->initSigMask));
  Signal(id->initTask, id->initSigMask);

  /* only if port is available then enter work loop. otherwise quit task */
  if(port != NULL)
  {
    /* worker loop */
    D(("Task: enter\n"));
    BOOL stay = TRUE;
    while (stay) {
      WaitPort(port);
      while (1) {
        ior = (struct IOStdReq *)GetMsg(port);
        if(ior == NULL) {
          break;
        }
        /* terminate? */
        if(ior->io_Command == CMD_TERM) {
          stay = FALSE;
          ReplyMsg(&ior->io_Message);
          break;
        }
        /* regular command */
        else {
          mydev_worker_cmd(base, ior);
          ReplyMsg(&ior->io_Message);
        }
      }
    }

    /* call shutdown only if worker was entered */
    D(("Task: exit\n"));
    /* shutdown worker */
    mydev_worker_exit(base);
  }

  D(("Task: delete port\n"));
  DeleteMsgPort(port);
  base->workerPort = NULL;

  /* kill myself */
  D(("Task: die\n"));
  struct Task *me = FindTask(NULL);
  DeleteTask(me);
  Wait(0);
  D(("Task: NEVER!\n"));
}

BOOL worker_start(struct DevBase *base)
{
  D(("Worker: start\n"));
  base->workerPort = NULL;

  /* alloc a signal */
  BYTE signal = AllocSignal(-1);
  if(signal == -1) {
    D(("Worker: NO SIGNAL!\n"));
    return FALSE;
  }

  /* setup init data */
  struct InitData id;
  id.initSigMask = 1 << signal;
  id.initTask = FindTask(NULL);
  id.base = base;
  D(("Worker: init data %08lx\n", &id));

  /* now launch worker task and inject dev base
     make sure worker_main() does not run before base is set.
  */
  Forbid();
  struct Task *myTask = CreateTask(WorkerTaskName, 0, (CONST APTR)worker_main, 4096);
  if(myTask != NULL) {
    myTask->tc_UserData = (APTR)&id;
  }
  Permit();
  if(myTask == NULL) {
    D(("Worker: NO TASK!\n"));
    FreeSignal(signal);
    return FALSE;
  }

  /* wait for start signal of new task */
  D(("Worker: wait for task startup. sigmask=%08lx\n", id.initSigMask));
  Wait(id.initSigMask);

  FreeSignal(signal);

  /* ok everything is fine. worker is ready to receive commands */
  D(("Worker: started: port=%08lx\n", base->workerPort));
  return (base->workerPort != NULL) ? TRUE : FALSE;
}

void worker_stop(struct DevBase *base)
{
  struct IORequest newior;

  D(("Worker: stop\n"));

  if(base->workerPort != NULL) {
    /* send a message to the child process to shut down. */
    newior.io_Message.mn_ReplyPort = CreateMsgPort();
    newior.io_Command = CMD_TERM;

    /* send term message and wait for reply */
    PutMsg(base->workerPort, &newior.io_Message);
    WaitPort(newior.io_Message.mn_ReplyPort);
    DeleteMsgPort(newior.io_Message.mn_ReplyPort);
  }

  D(("Worker: stopped\n"));
}
