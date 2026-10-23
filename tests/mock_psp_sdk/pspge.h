typedef struct { unsigned x[1024]; } PspGeContext;
int sceGeSaveContext(PspGeContext*);
int sceGeRestoreContext(PspGeContext*);
