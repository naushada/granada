#ifndef __webservice_main_cc__
#define __webservice_main_cc__

#include <webservice.h>

int main()
{
    WebServer inst("", 8080);
    
    inst.start();

    return(0);
}

















#endif /* __webservice_main_cc__*/