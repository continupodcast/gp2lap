#ifndef F1CSV_H
#define F1CSV_H
/* Recreate headers if an open-process log was removed or truncated. */
static FILE *F1CsvOpen(const char *name,int *ready,const char *header)
{
 FILE *f=fopen(name,*ready?"a":"w");
 if(!f)return 0;
 if(fseek(f,0,SEEK_END)!=0){fclose(f);return 0;}
 if(ftell(f)==0 && fputs(header,f)==EOF){fclose(f);return 0;}
 *ready=1;return f;
}
#endif
