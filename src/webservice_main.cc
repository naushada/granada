#ifndef __webservice_main_cc__
#define __webservice_main_cc__

#include "webservice.h"

int main(int argc, char* argv[])
{
   std::string ip("");
   std::string port("");
   int _port = 8080;

   if(argc > 1) {

     ip.assign(argv[1]);
     port.assign(argv[2]);
     _port = std::stoi(port);
     ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l IP %s port %d\n"), ip.c_str(), _port));

   } else {
     ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l the ip 0.0.0.0 and port 8080\n")));
   }

    
   WebServer inst(ip, _port);
   inst.start();

   return(0);
}

















#endif /* __webservice_main_cc__*/
