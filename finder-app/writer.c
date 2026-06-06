#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
int main(int argc, char *argv[]) {
    openlog("writer", LOG_PID | LOG_PERROR, LOG_USER);
    if (argc != 3) {
        syslog(LOG_ERR, "Error: Invalid arguments. Usage: %s <file_path> <string_to_write>", argv[0]);
        closelog();
        return 1;
    }
    char* filename=argv[1];
    char* text=argv[2];
    FILE* fp=fopen(filename,"w");
    //file not exist
    if (fp==NULL)
    {
        syslog(LOG_ERR, "Failed to open file %s: %m", filename);
        closelog();
        return 1;
    }
    fprintf(fp,"%s",text);
    syslog(LOG_DEBUG,"Writing %s to %s",text,filename);
    fclose(fp);
    closelog();
    return 0;
    
}