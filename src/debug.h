#ifdef DEBUG
extern void KPrintF(char *, ...);
#define D(x) KPrintF x ;
#else
#define D(x)
#endif
