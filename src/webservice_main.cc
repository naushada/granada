#ifndef __webservice_main_cc__
#define __webservice_main_cc__

#include "webservice.h"

int main(int argc, char* argv[])
{
   std::string ip("");
   std::string port("");
   std::string worker("");
   std::string db_uri("");
   std::string db_conn_pool("");
   int _port = 8080;
   int _worker = 10;
   int _pool = 50;
   
   ACE_LOG_MSG->open (argv[0]);

   ACE_Get_Opt opts (argc, argv, ACE_TEXT ("s:p:w:m:c:h:"), 1);
   opts.long_option (ACE_TEXT ("server-ip"), 's', ACE_Get_Opt::ARG_REQUIRED);
   opts.long_option (ACE_TEXT ("server-port"), 'p', ACE_Get_Opt::ARG_REQUIRED);
   opts.long_option (ACE_TEXT ("server-worker"), 'w', ACE_Get_Opt::ARG_REQUIRED);
   opts.long_option (ACE_TEXT ("mongo-db-uri"), 'm', ACE_Get_Opt::ARG_REQUIRED);
   opts.long_option (ACE_TEXT ("mongo-db-connection-pool"), 'c', ACE_Get_Opt::ARG_REQUIRED);
   opts.long_option (ACE_TEXT ("help"), 'h', ACE_Get_Opt::ARG_REQUIRED);
   int c = 0;

   while ((c = opts ()) != EOF) {
     switch (c) {
       case 's':
       {
         ACE_TCHAR* i = opts.opt_arg();
         if(!i) {
           ACE_ERROR((LM_ERROR, ACE_TEXT("%D [master:%t] %M %N:%l IP value is zero\n")));
         } else {
           std::string tmp(i);
           ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [master:%t] %M %N:%l IP %s\n"), tmp.c_str()));
           ip.assign(i);
         }
       }
         break;

       case 'p':
       {
         ACE_TCHAR* p = opts.opt_arg();
         std::string tmp(p);
         ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [master:%t] %M %N:%l PORT %s\n"), tmp.c_str()));
         port.assign(tmp);
       }
         break;

       case 'w':
         worker.assign(opts.opt_arg());
         break;

       case 'm':
         db_uri.assign(opts.opt_arg());
         break;
         
       case 'c':
         db_conn_pool.assign(opts.opt_arg());
         break;

       case 'h':
       default:
          ACE_ERROR_RETURN ((LM_ERROR,
                            ACE_TEXT("%D [Master:%t] %M %N:%l usage: %s\n"
                            " [-s --server-ip]\n"
                            " [-p --server-port]\n"
                            " [-w --server-worker]\n"
                            " [-m --mongo-db-uri]\n"
                            " [-c --mongo-db-connection-pool]\n"
                            " [-h --help]\n"
                            ),
                            argv [0]),
                            -1);
     }
   }

   if(port.length()) {
     _port = std::stoi(port);
     ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Master:%t] %M %N:%l IP %s port %d\n"), ip.c_str(), _port));

   }

   if(worker.length()) {
     _worker = std::stoi(worker);
     ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Master:%t] %M %N:%l the number of worker is %d\n"), _worker));
   }
   
   if(db_conn_pool.length()) {
     _pool = std::stoi(db_conn_pool);
     ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Master:%t] %M %N:%l the db connection pool is %d\n"), _pool));
   } 

   WebServer inst(ip, _port, _worker);
   inst.start();

   return(0);
}

















#endif /* __webservice_main_cc__*/
