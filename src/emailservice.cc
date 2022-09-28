#ifndef __emailservice_cc__
#define __emailservice_cc__

#include "emailservice.hpp"

SMTP::Tls::Tls()
{
    m_ssl = nullptr;
    m_sslCtx = nullptr;
}

SMTP::Tls::~Tls()
{
    SSL_free(m_ssl);
    SSL_CTX_free(m_sslCtx);
}

void SMTP::Tls::init()
{
    const SSL_METHOD *method;
    /* Load cryptos, et.al. */
    OpenSSL_add_all_algorithms();	
    /* Bring in and register error messages */	
    SSL_load_error_strings();			
    /* Create new client-method instance */
    method = SSLv23_client_method();		
    /* Create new context */
    m_sslCtx = SSL_CTX_new(method);			
    if(m_sslCtx == NULL) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D [mailservice:%t] %M %N:%l ssl context creation is failed aborting it.\n")));
        ERR_print_errors_fp(stderr);
        abort();
    }

    /* ---------------------------------------------------------- *
     * Disabling SSLv2 will leave v3 and TSLv1 for negotiation    *
     * ---------------------------------------------------------- */
    SSL_CTX_set_options(m_sslCtx, SSL_OP_NO_SSLv2);
}

std::int32_t SMTP::Tls::start(std::int32_t tcp_handle)
{
    std::int32_t rc = -1;

    init();
    /*create new SSL connection state*/
    m_ssl = SSL_new(m_sslCtx);
    /*continue as long as m_ssl is not NULL*/
    assert(m_ssl != NULL);

    /*attach the tcp socket descriptor to SSL*/
    rc = SSL_set_fd(m_ssl, tcp_handle);
    assert(rc == 1);
  	
    /*Initiate ClientHello Message to TLS Server*/
    if((rc = SSL_connect(m_ssl)) != 1) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D [mailservice:%t] %M %N:%l SSL_connect is failed %d\n"), SSL_get_error(m_ssl, rc)));
        /*TLS/SSL handshake is not successfullu*/
        SSL_free(m_ssl);
        SSL_CTX_free(m_sslCtx);
    }
    return(rc);
}

std::int32_t SMTP::Tls::read(std::array<char, 512> plain_buffer)
{
    std::int32_t rc = -1;
    plain_buffer.fill(0);
    
    rc = SSL_read(m_ssl, plain_buffer.data(), plain_buffer.size());
    return(rc);
}

std::int32_t SMTP::Tls::write(std::array<char, 512> plain_buffer, size_t len)
{
    std::int32_t rc = -1;
    rc = SSL_write(m_ssl, plain_buffer.data(), len);
    return(rc);
}

std::int32_t SMTP::Tls::peek(std::array<char, 512> plain_buffer, size_t len)
{
    std::int32_t rc = -1;
    rc = SSL_peek(m_ssl, plain_buffer.data(), len);
    return(rc);
}

void SMTP::Tls::close()
{

}

SMTP::Client::Client(ACE_UINT16 port, std::string addr, User* user)
{
  m_user = user;
  m_mb = nullptr;
  m_mailServiceAvailable = false;
  m_smtpServerAddress.set(port, addr.c_str());
  m_semaphore = std::make_unique<ACE_Semaphore>();
  m_tls = std::make_unique<Tls>();
}

SMTP::Client::~Client()
{
  m_semaphore.reset(nullptr);
  m_tls.reset(nullptr);
}

int SMTP::Client::svc(void) {
  m_semaphore->release();
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l semaphore is released and going into main-loop\n")));
  main();
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l active object is stopped now\n")));
}

int SMTP::Client::open(void *args) {
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l active object is spawned\n")));
  activate();
  return(0);
}

int SMTP::Client::close(u_long flags) {
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l active object is closed now\n")));
  return(0);
}

ACE_INT32 SMTP::Client::handle_timeout(const ACE_Time_Value &tv, const void *act)
{
  return(0);
}

ACE_INT32 SMTP::Client::handle_input(ACE_HANDLE handle)
{
  std::array<std::uint8_t, 2048> in;
  in.fill(0);

  auto ret = m_secureDataStream.recv((void *)in.data(), in.max_size());
  if(ret > 0) {

    std::string ss((char *)in.data(), ret);
    std::string out("");
    SMTP::States new_state;

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l receive length:%d response:%s\n"), ret, ss.c_str()));

    /// @brief feed to FSm for processing of incoming request
    auto nxtAction = user().fsm().onRx(ss, out, new_state);
    
    /// @brief  send the response for received request.
    if(out.length()) {
        ret = m_secureDataStream.send_n(out.c_str(), out.length());
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l sent length:%d command:%s\n"), ret, out.c_str()));
    }

    /// @brief  move to new state for processing of next Request.
    if(!nxtAction) {
        user().fsm().set_state(new_state);
    }
    
  } else {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D [mailservice:%t] %M %N:%l handle_input failed\n")));
    return(-1);
  }

  /// @return upon success returns zero meaning middleware will continue the reactor loop else it breaks the loop
  return(0);
}

ACE_INT32 SMTP::Client::handle_signal(int signum, siginfo_t *s, ucontext_t *u)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Master:%t] %M %N:%l Signal Number %d and its name %S is received for emailservice\n"), signum, signum));
  ACE_Reactor::instance()->remove_handler(m_secureDataStream.get_handle(), 
                                                         ACE_Event_Handler::ACCEPT_MASK | 
                                                         ACE_Event_Handler::TIMER_MASK | 
                                                         ACE_Event_Handler::SIGNAL_MASK);
  return(-1);

}

std::int32_t SMTP::Client::tx(const std::string in)
{
  std::int32_t txLen = -1;

  if(m_mailServiceAvailable) {

    txLen = m_secureDataStream.send_n(in.c_str(), in.length());

    if(txLen < 0) {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D [mailservice:%t] %M %N:%l send_n to %s and port %u is failed for length %u\n"), 
        m_smtpServerAddress.get_host_name(), m_smtpServerAddress.get_port_number(), in.length()));
    }

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l successfully sent of length %d on handle %u\n"), in.length(), m_secureDataStream.get_handle()));
  }
  return(txLen);
}

/**
 * @brief This member function is invoked when remove_handler or any of handle_xxx returns -1
 * 
 * @param fd the handle to be closed
 * @param mask mask of this handle
 * @return ACE_INT32 always 0
 */
ACE_INT32 SMTP::Client::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask)
{
  m_mailServiceAvailable = false;
  close(fd);
  return(0);
}

/**
 * @brief This member method is invoked when we invoke register_handle
 * 
 * @return ACE_HANDLE returns the registered handle
 */
ACE_HANDLE SMTP::Client::get_handle() const
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l get_handle handle:%u\n"), m_secureDataStream.get_handle()));
  return(m_secureDataStream.get_handle());
}

/**
 * @brief This function starts the SMTP client by establishing secure connection
 * 
 */
void SMTP::Client::start()
{
  do {
    if(m_secureSmtpServerConnection.connect(m_secureDataStream, m_smtpServerAddress)) {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D [mailservice:%t] %M %N:%l connect to host-name:%s and port-number:%u is failed\n"), 
           m_smtpServerAddress.get_host_name(), m_smtpServerAddress.get_port_number()));
      m_mailServiceAvailable = false;
      break;
    }

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l connect host-name:%s, ip-address:%s and port:%u is success\n"), 
           m_smtpServerAddress.get_host_name(),m_smtpServerAddress.get_host_addr(), m_smtpServerAddress.get_port_number()));

    /* Feed this new handle to event Handler for read/write operation. */
    ACE_Reactor::instance()->register_handler(m_secureDataStream.get_handle(), this, ACE_Event_Handler::READ_MASK |
                                                                                     ACE_Event_Handler::TIMER_MASK |
                                                                                     ACE_Event_Handler::SIGNAL_MASK);

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [emailservice:%t] %M %N:%l The emailservice is connected at handle:%u\n"), m_secureDataStream.get_handle()));
    m_mailServiceAvailable = true;

    /* subscribe for signal */
    ss.empty_set();
    ss.sig_add(SIGINT);
    ss.sig_add(SIGTERM);
    ACE_Reactor::instance()->register_handler(&ss, this);
    // spawn an active object for waiting on reactor
    open();

  }while(0);

}

void SMTP::Client::main() {
    ACE_Time_Value to(1,0);
    while(m_mailServiceAvailable) ACE_Reactor::instance()->handle_events();
}

void SMTP::Client::stop()
{
  ACE_Reactor::instance()->remove_handler(m_secureDataStream.get_handle(), ACE_Event_Handler::ACCEPT_MASK |
                                                                           ACE_Event_Handler::TIMER_MASK |
                                                                           ACE_Event_Handler::SIGNAL_MASK);

}

SMTP::User::~User()
{
  m_client.reset(nullptr);
}

std::int32_t SMTP::User::startEmailTransaction()
{
  std::int32_t ret = 0;
  std::string req;
  m_client->start();
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [emailservice:%t] %M %N:%l acquiring semaphore\n")));
  m_client->m_semaphore->acquire();
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [emailservice:%t] %M %N:%l semaphore is released\n")));
  /* upon connection establishment to smtp server, smtp server replies with greeting */
  m_fsm.set_state(GREETING());
  
  return(ret);
}

std::int32_t SMTP::User::endEmailTransaction()
{
  std::int32_t ret = 0;
  return(ret);
}

/**
 * @brief This member method is invoked by SMTP client upon receipt of response from SMTP server
 * 
 * @param out response string from SMTP server
 * @return std::int32_t returns > 0 upon success else less than 0
 */
std::int32_t SMTP::User::rx(std::string out)
{
  //auto stateType = decltype(m_fsm)
  //auto idx = m_fsm.index();
  //m_fsm.onCommand(out);
  //return(get_state().onResponse(out));
}

/**
 * @brief This member function stores the email id for to list
 * 
 * @param toList list of email id for to list
 */
SMTP::User& SMTP::User::to(std::vector<std::string> toList)
{
  m_to = toList;
  return(*this);
}

/**
 * @brief This member function stores the email id for cc list
 * 
 * @param ccList list of email id for cc list
 */
SMTP::User& SMTP::User::cc(std::vector<std::string> ccList)
{
  m_cc = ccList;
  return(*this);
}

/**
 * @brief This member function stores the email id for bcc list
 * 
 * @param bccList list of email id for bcc list
 */
SMTP::User& SMTP::User::bcc(std::vector<std::string> bccList)
{
  m_bcc = bccList;
  return(*this);
}

/**
 * @brief This member function returns the list of email id of to list
 * 
 * @return std::vector<std::string>& list of email id for to list
 */
const std::vector<std::string>& SMTP::User::to() const
{
  return(m_to);
}

/**
 * @brief This member function returns the list of email id of cc list
 * 
 * @return std::vector<std::string>& list of email id for cc list
 */
const std::vector<std::string>& SMTP::User::cc() const
{
  return(m_cc);
}

/**
 * @brief This member function returns the list of email id of bcc list
 * 
 * @return std::vector<std::string>& list of email id for bcc list
 */
const std::vector<std::string>& SMTP::User::bcc() const
{
  return(m_bcc);
}

/**
 * @brief This member function stores the subject of e-mail
 * 
 * @param subj email subject
 */
SMTP::User& SMTP::User::subject(std::string subj)
{
  m_subject = subj;
  return(*this);
}

/**
 * @brief This member function returns the subject of the e-mail
 * 
 * @return std::string& subject title of e-mail
 */
const std::string& SMTP::User::subject() const
{
  return(m_subject);
}

/**
 * @brief This member function stores the e-mail body 
 * 
 * @param emailBody email body
 */
SMTP::User& SMTP::User::data(std::string emailBody)
{
  m_data = emailBody;
  return(*this);
}

/**
 * @brief This member function return the email body
 * 
 * @return std::string& email body
 */
const std::string& SMTP::User::data() const
{
  return(m_data);
}

void SMTP::User::client(std::unique_ptr<SMTP::Client> smtpClient)
{
  m_client = std::move(smtpClient);
}

const std::unique_ptr<SMTP::Client>& SMTP::User::client() const
{
  return(m_client);
}

#endif /* __emailservice_cc__ */