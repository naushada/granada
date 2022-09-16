#ifndef __emailservice_hpp__
#define __emailservice_hpp__

#include<vector>
#include<iostream>
#include<unordered_map>
#include <variant>
#include <type_traits>

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
#include "ace/Get_Opt.h"
#include "ace/Signal.h"
#include "ace/SSL/SSL_SOCK.h"
#include "ace/SSL/SSL_SOCK_Stream.h"
#include "ace/SSL/SSL_SOCK_Connector.h"
#include "ace/Semaphore.h"

namespace SMTP {
  std::unordered_map<std::uint32_t, std::string> statusCodeUMap;
  class User;

  class Client : public ACE_Event_Handler {

    public:
      Client(std::string smtpAddress, User* user);
      ~Client();
      ACE_INT32 handle_timeout(const ACE_Time_Value &tv, const void *act=0) override;
      ACE_INT32 handle_input(ACE_HANDLE handle) override;
      ACE_INT32 handle_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0) override;
      ACE_INT32 handle_close (ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask = 0) override;
      ACE_HANDLE get_handle() const override;
      void start();
      void stop();
      std::int32_t tx(const std::string in);
      std::int32_t rx(std::string &out);
      User& user() {
        return(*m_user);
      }

    public:
      ACE_INET_Addr m_smtpServerAddress;
      ACE_SSL_SOCK_Connector m_secureSmtpServerConnection;
      ACE_SSL_SOCK_Stream m_secureDataStream;
      ACE_Message_Block *m_mb;
      bool m_mailServiceAvailable;
      User *m_user;
  };

  template<typename ST>
  class Transaction {
    public:
      Transaction() = default;
      ~Transaction() = default;

      template<typename... Args>
      void set_state(auto new_state, Args... args) {

        std::visit([&](auto arg) -> void {
          arg.onExit(args...);
          }, m_state);

        m_state = std::move(new_state);

        std::visit([&](auto arg) -> void {
          arg.onEntry(args...);
          }, m_state);
      }

      std::int32_t onResponse(std::string in)
      {
        std::visit([&](auto arg) -> std::int32_t {
          return(arg.onResponse(in));
          }, m_state);
      }

      std::int32_t onRequest(std::string in) 
      {
        return(0);
      }

    private:
      ST m_state;
  };

  class MAIL {
    public:
      MAIL() = default;
      ~MAIL() = default;

      void onEntry() {

      }

      void onExit() {

      }

      std::int32_t onResponse(std::string in) {
      
        return(0);
      }

      std::int32_t onResponse() {
      
        return(0);
      }

      std::int32_t onRequest(std::string in) {
        return(0);
      }
  };

  class RCPT {
    public:
      RCPT() = default;
      ~RCPT() = default;

      void onEntry() {

      }

      void onExit() {

      }

      std::int32_t onResponse(std::string in) {
      
        return(0);
      }

      std::int32_t onResponse() {
      
        return(0);
      }

      std::int32_t onRequest(std::string in) {
        return(0);
      }
  };

  class DATA {
    public:
      DATA() = default;

      ~DATA() = default;

      void onEntry() {

      }

      void onExit() {

      }

      std::int32_t onResponse(std::string in) {
      
        return(0);
      }

      std::int32_t onResponse() {
      
        return(0);
      }

      std::int32_t onRequest(std::string in) {
        return(0);
      }
  };

  class INIT {
    public:
      INIT() = default;
      ~INIT() = default;

      void onEntry() {

      }

      void onExit() {

      }
      std::int32_t onResponse(std::string in) {
      
        return(0);
      }

      std::int32_t onResponse() {
      
        return(0);
      }

      std::int32_t onRequest(std::string in) {
        return(0);
      }
  };

  using States = std::variant<INIT, MAIL, RCPT, DATA>;

  class Account {
    public:
      Account() = default;
      ~Account() = default;

      Account& name(std::string in) {
        m_name = in;
        return(*this);
      }

      std::string name() const {
        return(m_name);
      }

      Account& email(std::string in) {
        m_email = in;
        return(*this);
      }

      std::string email() const {
        return(m_email);
      }

      Account& userid(std::string in) {
        m_userid = in;
        return(*this);
      }

      std::string userid() const {
        return(m_userid);
      }

      Account& password(std::string in) {
        m_password = in;
        return(*this);
      }

      std::string password() const {
        return(m_password);
      }

    private:
      std::string m_name;
      std::string m_email;
      std::string m_userid;
      std::string m_password;
  };

  class User {
    public:
      User() {
        m_client = std::make_unique<Client>("smtp.gmail.com:465", this);

      }

      ~User();
      std::int32_t startEmailTransaction();
      std::int32_t endEmailTransaction();
      /**
       * @brief This member function stores the email id for to list
       * 
       * @param toList list of email id for to list
       */
      SMTP::User& to(std::vector<std::string> toList);
      /**
       * @brief This member function stores the email id for cc list
       * 
       * @param ccList list of email id for cc list
       */
      SMTP::User& cc(std::vector<std::string> ccList);
      /**
       * @brief This member function stores the email id for bcc list
       * 
       * @param bccList list of email id for bcc list
       */
      SMTP::User& bcc(std::vector<std::string> bccList);
      /**
       * @brief This member function returns the list of email id of to list
       * 
       * @return std::vector<std::string>& list of email id for to list
       */
      const std::vector<std::string>& to() const;
      /**
       * @brief This member function returns the list of email id of cc list
       * 
       * @return std::vector<std::string>& list of email id for cc list
       */
      const std::vector<std::string>& cc() const;
      /**
       * @brief This member function returns the list of email id of bcc list
       * 
       * @return std::vector<std::string>& list of email id for bcc list
       */
      const std::vector<std::string>& bcc() const;
      /**
       * @brief This member function stores the subject of e-mail
       * 
       * @param subj email subject
       */
      SMTP::User& subject(std::string subj);
      /**
       * @brief This member function returns the subject of the e-mail
       * 
       * @return std::string& subject title of e-mail
       */
      const std::string& subject() const;
      /**
       * @brief This member function stores the e-mail body 
       * 
       * @param emailBody email body
       */
      SMTP::User& data(std::string emailBody);
      /**
       * @brief This member function return the email body
       * 
       * @return std::string& email body
       */
      const std::string& data() const;
      
      void client(std::unique_ptr<SMTP::Client> smtpClient);
      const std::unique_ptr<SMTP::Client>& client() const;
      std::int32_t rx(std::string out);
      Account& account() {
        return(m_account);
      }

    private:
      Account m_account;
      std::string m_subject;
      std::vector<std::string> m_to;
      std::vector<std::string> m_cc;
      std::vector<std::string> m_bcc;
      std::string m_data;
      /* Instance of SMTP User state is initialized with INIT */
      Transaction<States> m_fsm;
      /* Instance of SMTP Client */
      std::unique_ptr<Client> m_client;
  };

}
#endif /* __emailservice_hpp__ */