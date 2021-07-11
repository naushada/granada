#ifndef __webservice_h__
#define __webservice_h__

#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>

#include "ace/Reactor.h"
#include "ace/Basic_Types.h"
#include "ace/Event_Handler.h"
#include "ace/Task.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/Task_T.h"
#include "ace/Timer_Queue_T.h"
#include "ace/Reactor.h"
#include "ace/OS_Memory.h"
#include "ace/Thread_Manager.h"

/* Forward declaration */
class WebServer;

enum class MicroServiceType : ACE_UINT16 {
    CREATE,
    UPDATE,
    READ,
    DELETE,
    UNKNOWN
};

enum class MemorySize : std::uint32_t {
    SIZE_1KB = 1024,
    SIZE_5KB = (5 * SIZE_1KB),
    SIZE_10KB = (10 * SIZE_1KB),
    SIZE_1M = (SIZE_1KB * SIZE_1KB)
};

class MicroService : public ACE_Task<ACE_MT_SYNCH> {
    public:
        int svc(void) override;
				int open(void *args=0) override;
        int close (u_long flags=0) override;

        ACE_INT32 handle_signal(int signum, siginfo_t *s, ucontext_t *u) override;

        MicroService(ACE_Thread_Manager *thrMgr);
				~MicroService();
        ACE_INT32 process_request(ACE_HANDLE handle, ACE_Message_Block& mb);

        ACE_thread_t myThreadId() {
          return(m_threadId);
        }
        bool isTaskRunning() const {
          std::uint32_t st = ACE_Thread_Manager::ACE_THR_RUNNING;
          ACE_Thread_Manager::instance()->thr_state(m_threadId, st);
          return(ACE_Thread_Manager::ACE_THR_RUNNING == st);
        }

    private:
        bool m_continue;
        ACE_Message_Block* m_mb;
        ACE_thread_t m_threadId;
        bool m_iAmDone;
        
};

class WebConnection : public ACE_Event_Handler {
    public:
        ACE_INT32 handle_timeout(const ACE_Time_Value &tv, const void *act=0) override;
        ACE_INT32 handle_input(ACE_HANDLE handle) override;
        ACE_INT32 handle_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0) override;
				ACE_INT32 handle_close (ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask = 0) override;
        ACE_HANDLE get_handle() const override;

        WebConnection(WebServer* parent);
        ~WebConnection();

        long timerId() const {
            return(m_timerId);  
        }

        void timerId(long tid) {
            m_timerId = tid;
        }

        ACE_HANDLE handle() const {
            return(m_handle);
        }

        void handle(ACE_HANDLE fd) {
            m_handle = fd;
        }
        void connAddr(ACE_INET_Addr addr) {
          m_connAddr = addr;
        }
        

    private:
        long m_timerId;
        ACE_HANDLE m_handle;
        ACE_INET_Addr m_connAddr;
        WebServer* m_parent;
};

class WebServer : public ACE_Event_Handler {
    public:
        ACE_INT32 handle_timeout(const ACE_Time_Value &tv, const void *act=0) override;
        ACE_INT32 handle_input(ACE_HANDLE handle) override;
        ACE_INT32 handle_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0) override;
				ACE_INT32 handle_close (ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask = 0) override;
        ACE_HANDLE get_handle() const override;

				WebServer(std::string _ip, ACE_UINT16 _port);
				~WebServer();
        bool start();
        bool stop();

        long start_conn_cleanup_timer(ACE_HANDLE handle);
        void stop_conn_cleanup_timer(long timerId);

        std::unordered_map<ACE_HANDLE, WebConnection*>& connectionPool() {
          return(m_connectionPool);
        }

        std::vector<MicroService*>& workerPool() {
            return(m_workerPool);
        }

        std::vector<MicroService*>::iterator currentWorker()
        {
            if(m_currentWorker == std::end(m_workerPool)) {
              m_currentWorker = std::begin(m_workerPool);
            }
            
            auto curr = m_currentWorker;
            ++m_currentWorker;
            return(curr);
        }

    private:
        ACE_Message_Block m_mb;
        ACE_SOCK_Stream m_stream;
				ACE_INET_Addr m_listen;
				ACE_SOCK_Acceptor m_server;
        bool m_stopMe;
        std::unordered_map<ACE_HANDLE, WebConnection*> m_connectionPool;
        std::vector<MicroService*> m_workerPool;
        std::vector<MicroService*>::iterator m_currentWorker;

};

class WebServiceEntry {
};


#endif /*__webservice_h__*/
