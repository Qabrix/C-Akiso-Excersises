    #include <stdio.h>
    #include <unistd.h>

    int main()
    {
        setuid(0);
        char *args[]={"/bin/bash",NULL}; 
        execvp(args[0],args); 
        return 0;
    }