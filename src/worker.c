#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <dos/dos.h>

#define NO_SYSBASE
#include "mydev.h"
#include "debug.h"
#include "worker.h"

#define CMD_TERM     0x7ff0
#define CMD_STARTUP  0x7ff1

struct MyStartMsg {
  struct Message msg;
  struct DevBase *base;
  struct MsgPort *port;
};

static struct MyStartMsg * worker_startup(void)
{
  struct Process *proc;
  struct MyStartMsg *msg;

  /* local sys base */
  struct Library *SysBase = *((struct Library **)4);

  proc = (struct Process *)FindTask((char *)NULL);

  /* get the startup message */
  while((msg = (struct MyStartMsg *)GetMsg(&proc->pr_MsgPort)) == NULL) {
    WaitPort(&proc->pr_MsgPort);
  }

  /* extract DevBase db for lib base access */
  return msg;
}

#define SysBase base->sysBase

static void worker_main(void)
{
  struct IOStdReq *ior;
  struct DevBase *base;
  struct MsgPort *port;
  struct MyStartMsg *msg;

  msg = worker_startup();

  /* get device base - now we can reference SysBase via base->sysBase */
  base = msg->base;

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

  /* reply startup message */
  msg->port = port;
  ReplyMsg((struct Message *)msg);

  /* if port failed quit process */
  if(port == NULL)
  {
    return;
  }

  /* worker loop */
  D(("Worker: enter\n"));
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
        break;
      }
      /* regular command */
      else {
        mydev_worker_cmd(base, ior);
        ReplyMsg(&ior->io_Message);
      }
    }
  }

  D(("Worker: leave\n"));
  /* shutdown worker */
  mydev_worker_exit(base);

  Forbid();
  ReplyMsg(&ior->io_Message);
}

BOOL worker_start(struct DevBase *base)
{
  struct Process *myProc;
  struct MyStartMsg msg;

  D(("Worker: start\n"));
  base->workerPort = NULL;

  /* open dos */
  DOSBase = (struct Library *)OpenLibrary("dos.library", 36);
  if(DOSBase == NULL) {
    return FALSE;
  }

  /* worker process */
  myProc = CreateNewProcTags(NP_Entry, (LONG)worker_main,
                             NP_StackSize, 4096,
                             NP_Name, (LONG)NAME,
                             TAG_DONE);
  if (myProc == NULL) {
    return FALSE;
  }

  /* Send the startup message with the library base pointer */
  msg.msg.mn_Length = sizeof(struct MyStartMsg) -
                      sizeof (struct Message);
  msg.msg.mn_ReplyPort = CreateMsgPort();
  msg.msg.mn_Node.ln_Type = NT_MESSAGE;
  msg.base = base;
  msg.port = NULL;
  PutMsg(&myProc->pr_MsgPort, (struct Message *)&msg);
  WaitPort(msg.msg.mn_ReplyPort);
  DeleteMsgPort(msg.msg.mn_ReplyPort);

  D(("Worker: started: port=%08lx\n", msg.port));
  base->workerPort = msg.port;
  return TRUE;
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

    /* cleanup worker port */
    DeleteMsgPort(base->workerPort);
  }

  if(DOSBase != NULL) {
    /* close dos */
    CloseLibrary(DOSBase);
  }

  D(("Worker: stopped\n"));
}
