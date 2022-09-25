#ifndef __emailservice_hpp__
#define __emailservice_hpp__

#include<vector>
#include<iostream>
#include<unordered_map>
#include <variant>
#include <type_traits>
#include <optional>

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
#include "ace/Codecs.h"


namespace SMTP {
  enum statusCode: std::uint32_t {
    STATUS_CODE_214_Response_to_HELP = 214,
    STATUS_CODE_211_System_status = 211,
    STATUS_CODE_220_Service_ready = 220,
    STATUS_CODE_221_Service_closing_transmission_channel = 221,
    //STATUS_CODE_221_2_0_0_Goodbye = 211,
    STATUS_CODE_235_2_7_0_Authentication_succeeded = 235,
    STATUS_CODE_240_Quit = 240,
    STATUS_CODE_250_Request_mail_action_okay_completed = 250,
    STATUS_CODE_251_User_not_local_will_forward = 251,
    STATUS_CODE_252_Can_not_verify_the_user = 252,
    STATUS_CODE_334_Server_challenge = 334,
    //STATUS_CODE_354_Start_mail_input = 354,
    /* -ve status code */
    STATUS_CODE_432_4_7_12_A_password_transition_needed = 432,
    STATUS_CODE_421_Service_not_available_closing_channel = 421,
    STATUS_CODE_450_Requested_mail_action_not_taken = 450,
    STATUS_CODE_451_4_4_1_IMAP_server_unavailable = 451,
    //STATUS_CODE_451_Requested_action_aborted = 451,
    STATUS_CODE_452_Requested_action_not_taken = 451,
    STATUS_CODE_454_4_7_0_Temporary_authentication_failure = 454,
    STATUS_CODE_455_Server_unable_to_accomodate_parameters = 455,
    /* -ve status code */
    STATUS_CODE_500_Syntax_error_command_unrecognized = 500,
    STATUS_CODE_500_5_5_6_Authentication_exchange_line_too_long = 500,
    STATUS_CODE_501_Syntax_error_in_parameters = 501,
    //STATUS_CODE_501_5_5_2_Cannot_base64_decode_client_response = 501,
    //STATUS_CODE_501_5_7_0_Client_initiated_authentication_exchange = 501,
    STATUS_CODE_502_Command_not_implemented = 502,
    STATUS_CODE_503_Bad_sequence_of_command = 503,
    STATUS_CODE_504_Command_parameter_is_not_implemented = 504,
    //STATUS_CODE_504_5_5_4_Unrecognized_authentication_type = 504,
    STATUS_CODE_521_Server_does_not_accept_mail = 521,
    STATUS_CODE_523_Encryption_needed = 523,
    STATUS_CODE_530_5_7_0_Authentication_needed = 530,
    STATUS_CODE_534_5_7_9_Authentication_mechanism_is_too_weak = 534,
    STATUS_CODE_535_5_7_8_Authentication_credentials_invalid = 535,
    STATUS_CODE_538_5_7_11_Encryption_required = 538,
    STATUS_CODE_550_Requested_action_not_taken = 550,
    STATUS_CODE_551_User_not_local = 551,
    STATUS_CODE_552_Requested_mail_action_aborted = 552,
    STATUS_CODE_553_Requested_action_not_taken = 553,
    STATUS_CODE_554_Transaction_has_failed = 554,
    //STATUS_CODE_554_5_3_4_Message_too_big_for_system = 554,
    STATUS_CODE_556_Domain_does_not_accept_mail = 556

  };

  //std::unordered_map<statusCode_t, std::string>statusCodeUMap;
  /// @brief Forward declaration of classes
  class User;
  class HELO;
  class MAIL;
  class RCPT;
  class DATA;
  class QUIT;
  class BODY;
  class HELP;
  class NOOP;
  class VRFY;
  class EXPN;
  class RESET;
  class GREETING;

  /// @brief For new state, add into this variant 
  using States = std::variant<GREETING, HELO, MAIL, RCPT, DATA, QUIT, BODY, HELP, NOOP, VRFY, EXPN, RESET>;
  
  /// @brief the Client instance will be active object
  class Client : public ACE_Task<ACE_MT_SYNCH> {

    public:
      Client(ACE_UINT16 port, std::string smtpAddress, User* user);
      ~Client();

      int svc(void) override;
      int open(void *args=0) override;
      int close (u_long flags=0) override;

      ACE_INT32 handle_timeout(const ACE_Time_Value &tv, const void *act=0) override;
      ACE_INT32 handle_input(ACE_HANDLE handle) override;
      ACE_INT32 handle_signal(int signum, siginfo_t *s = 0, ucontext_t *u = 0) override;
      ACE_INT32 handle_close (ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask = 0) override;
      ACE_HANDLE get_handle() const override;
      void start();
      void stop();
      void main();

      std::int32_t tx(const std::string in);
      std::int32_t rx(std::string &out);
      User& user() {
        return(*m_user);
      }

    public:
      ACE_INET_Addr m_smtpServerAddress;
      ACE_SSL_SOCK_Connector m_secureSmtpServerConnection;
      ACE_SSL_SOCK_Stream m_secureDataStream;
      //ACE_SOCK_Connector m_secureSmtpServerConnection;
      //ACE_SOCK_Stream m_secureDataStream;
      ACE_Message_Block *m_mb;
      bool m_mailServiceAvailable;
      User *m_user;
      ACE_Sig_Set ss;
      std::unique_ptr<ACE_Semaphore> m_semaphore;
  };
  /*  _ ___
   * ||   ||
   * ||_ _//
   * ||   \\
   * ||    \\
   */
  template<typename ST>
  class Transaction {
    public:

      Transaction() = default;
      ~Transaction() = default;

      template<typename... Args>
      void set_state(auto new_state, Args... args) {

        std::visit([&](auto st) -> void {
            st.onExit(args...);
          }, m_state);

        m_state = std::move(new_state);

        std::visit([&](auto st) -> void {
            st.onEntry(args...);
          }, m_state);
      }

      auto get_state() {
        std::visit([&](auto st) {
          return(st);
        }, m_state);
      }
#if 0
      std::int32_t onCommand(std::string in, std::string& cmd)
      {
        std::visit([&](auto st) -> std::int32_t {
            return(st.onCommand(in, cmd));
          }, m_state);
      }
#endif
      std::uint32_t onRx(std::string in, std::string& out, ST& new_state) 
      {
          std::int32_t ret_status;
          std::visit([&](auto&& active_st) -> void {
              ret_status = active_st.onResponse(in, out, new_state);
          }, m_state);
        return(ret_status);
      }

    private:
      ST m_state;
  };

  class GREETING {
    public:
      GREETING() = default;
      ~GREETING() = default;

      void onEntry();
      void onExit();
      std::uint32_t onResponse(std::string in);
      std::uint32_t onResponse();
      std::uint32_t onCommand(std::string in, std::string& out, States& new_state);
      std::uint32_t onResponse(std::string in, std::string& out, States& new_state);

  };

  class MAIL {
    enum AuthSteps : std::uint32_t {
      AUTH_INIT,
      AUTH_USRNAME,
      AUTH_PASSWORD,
      AUTH_SUCCESS,
      AUTH_FAILURE
    };

    AuthSteps m_authStage;

    public:
      MAIL() : m_authStage(AUTH_INIT) {

      }

      ~MAIL() = default;

      void onEntry();
      void onExit();
      std::uint32_t onResponse(std::string in);
      std::uint32_t onResponse();
      std::uint32_t onResponse(std::string in, std::string& out, States& new_state);
      std::uint32_t onUsername(const std::string in, std::string& base64Username);
      std::uint32_t onPassword(const std::string in, std::string& base64Password);
      bool onLoginSuccess(const std::string in, std::string& base64Username);
      std::uint32_t onCommand(std::string in, std::string& out, States& new_state);
  };

  class RCPT {
    public:
      RCPT() = default;
      ~RCPT() = default;

      void onEntry();
      void onExit();
      std::uint32_t onResponse(std::string in);
      std::uint32_t onResponse();
      std::uint32_t onResponse(std::string in, std::string& out, States& new_state);
      std::uint32_t onCommand(std::string in, std::string& out, States& new_state);
  };

  class DATA {
    public:
      DATA() = default;
      ~DATA() = default;

      void onEntry();
      void onExit();
      std::uint32_t onResponse(std::string in);
      std::uint32_t onResponse();
      std::uint32_t onResponse(std::string in, std::string& out, States& new_state);
      std::uint32_t onCommand(std::string in, std::string& out, States& new_state);
  };

  class BODY {
    public:
      BODY() = default;
      ~BODY() = default;

      void onEntry();
      void onExit();
      std::uint32_t onResponse(std::string in);
      std::uint32_t onResponse();
      std::uint32_t onResponse(std::string in, std::string& out, States& new_state);
      std::uint32_t onCommand(std::string in, std::string& out, States& new_state);
  };
  class HELO {
    public:
      HELO() = default;
      ~HELO() = default;

      void onEntry();
      void onExit();
      std::uint32_t onResponse(std::string in);
      std::uint32_t onResponse();
      std::uint32_t onResponse(std::string in, std::string& out, States& new_state);
      std::uint32_t onCommand(std::string in, std::string& out, States& new_state);
  };

  class QUIT {
    public:
      QUIT() = default;
      ~QUIT() = default;

      void onEntry();
      void onExit();
      std::uint32_t onResponse(std::string in);
      std::uint32_t onResponse();
      std::uint32_t onResponse(std::string in, std::string& out, States& new_state);
      std::uint32_t onCommand(std::string in, std::string& out, States& new_state);
  };

  class RESET {
    public:
      RESET() = default;
      ~RESET() = default;

      void onEntry();
      void onExit();
      std::uint32_t onResponse(std::string in);
      std::uint32_t onResponse();
      std::uint32_t onResponse(std::string in, std::string& out, States& new_state);
      std::uint32_t onCommand(std::string in, std::string& out, States& new_state);
  };

  class VRFY {
    public:
      VRFY() = default;
      ~VRFY() = default;

      void onEntry();
      void onExit();
      std::uint32_t onResponse(std::string in);
      std::uint32_t onResponse();
      std::uint32_t onResponse(std::string in, std::string& out, States& new_state);
      std::uint32_t onCommand(std::string in, std::string& out, States& new_state);
  };

  class NOOP {
    public:
      NOOP() = default;
      ~NOOP() = default;

      void onEntry();
      void onExit();
      std::uint32_t onResponse(std::string in);
      std::uint32_t onResponse();
      std::uint32_t onResponse(std::string in, std::string& out, States& new_state);
      std::uint32_t onCommand(std::string in, std::string& out, States& new_state);
  };

  class EXPN {
    public:
      EXPN() = default;
      ~EXPN() = default;

      void onEntry();
      void onExit();
      std::uint32_t onResponse(std::string in);
      std::uint32_t onResponse();
      std::uint32_t onResponse(std::string in, std::string& out, States& new_state);
      std::uint32_t onCommand(std::string in, std::string& out, States& new_state);
  };

  class HELP {
    public:
      HELP() = default;
      ~HELP() = default;

      void onEntry();
      void onExit();
      std::uint32_t onResponse(std::string in);
      std::uint32_t onResponse();
      std::uint32_t onResponse(std::string in, std::string& out, States& new_state);
      std::uint32_t onCommand(std::string in, std::string& out, States& new_state);
  };

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
        /// For secure smtp the port is 465 and plain smtp the port is 25.
        //m_client = std::make_unique<Client>(/*"smtp.gmail.com:465"*/"142.251.12.108:465", this);
        m_client = std::make_unique<Client>(465, "smtp.gmail.com", this);

        account().email("naushad.dln@gmail.com").name("Naushad Ahmed")
                 .password("abcd").userid("naushad.dln");
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

      auto& fsm() {
        return(m_fsm);
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