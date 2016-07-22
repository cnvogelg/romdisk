struct DiskHeader;
struct PackHeader;
struct BufferMap;
struct DevBase;

typedef void (*read_func_t)(struct IOStdReq *ior, struct DevBase *base);
typedef ULONG (*unpack_func_t)(UBYTE *packed_data, UBYTE *out_data);

struct DevBase
{
  /* common */
  struct Library        libBase;
  struct Library        *sysBase;
  ULONG                 segList;
  /* worker */
  struct MsgPort        *workerPort;
  /* romdisk */
  struct DiskHeader     *diskHeader;
  UBYTE                 *diskData;
  read_func_t           readFunc;
  /* pack only */
  struct PackHeader     *packHeader;
  unpack_func_t         unpackFunc;
  /* open state */
  UBYTE *unpackBuffer;
  ULONG  curPackId;
};

#ifndef NO_SYSBASE
#define SysBase base->sysBase
#endif

extern struct DevBase *mydev_init(struct DevBase *base);
extern void mydev_expunge(struct DevBase *base);
extern struct DevBase * mydev_open(struct IOStdReq *ior, ULONG unit, ULONG flags, struct DevBase *base);
extern void mydev_close(struct IOStdReq *ior, struct DevBase *base);
extern void mydev_begin_io(struct IOStdReq *ior, struct DevBase *base);
extern LONG mydev_abort_io(struct IOStdReq *ior, struct DevBase *base);

extern BOOL mydev_worker_init(struct DevBase *base);
extern void mydev_worker_cmd(struct DevBase *base, struct IOStdReq *ior);
extern void mydev_worker_exit(struct DevBase *base);

