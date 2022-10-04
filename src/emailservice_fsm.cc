#ifndef __emailservice_fsm_cc__
#define __emailservice_fsm_cc__

/**
 * @file emailservice_fsm.cc
 * @author your name (naushad.dln@gmail.com)
 * @brief This file implements the FSM for SMTP
 * @version 0.1
 * @date 2022-09-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "emailservice.hpp"
#include <sstream>
#include <regex>

std::uint32_t SMTP::parseSmtpCommand(const std::string in, std::unordered_map<Response, std::string, hFn>& out)
{
    std::stringstream input(in);
    std::int32_t c;
    std::string elm("");
    out.clear();
    std::vector<std::string> arg;
    arg.clear();

    while((c = input.get()) != EOF) {
        switch(c) {
            case ' ':
            case '-':
            {
                arg.push_back(elm);
                auto cc = input.get();
                elm.clear();

                while(cc != ' ' && cc != EOF) {
                    if(cc == '\r') {
                        break;
                    } 
                    elm.push_back(cc);
                    cc = input.get();
                }

                if(cc == EOF) {
                    arg.push_back(elm);
                    out[Response(std::stoi(arg[0]), arg[1])] = "";
                    arg.clear();
                    elm.clear();
                    break;
                }

                /* get rid of ' ' or \n*/
                cc = input.get();
                if(cc == '\n') {
                    arg.push_back(elm);
                    out[Response(std::stoi(arg[0]), arg[1])] = "";
                    arg.clear();
                    elm.clear();
                    break;
                }

                /* get rid of ' ' */
                cc = input.get();
                arg.push_back(elm);
                elm.clear();

                while(cc != '\r' && cc != EOF) {
                    elm.push_back(cc);
                    cc = input.get();
                }

                /* get rid of '\n' */
                cc = input.get();
                arg.push_back(elm);
                out[SMTP::Response(std::stoi(arg[0]), arg[1])] = arg[2];
                arg.clear();
                elm.clear();
            }
            break;
            default:
            {
                elm.push_back(c);
            }
            break;
        }
    }
    return(0);
}

SMTP::Response SMTP::getSmtpStatusCode(const std::string in)
{
    std::unordered_map<Response, std::string, hFn> out;
    out.clear();
    parseSmtpCommand(in, out);
    auto it = out.begin();
    return(it->first);
}

std::uint32_t SMTP::getBase64(const std::string in, std::string& b64Out)
{
    size_t out_len = 0;
    const ACE_Byte* data = (ACE_Byte *)in.data();
    
    ACE_Byte* encName = ACE_Base64::encode(data, in.length(), &out_len);
    std::string b64_((char *)encName, out_len);
    std::stringstream ss("");
    ss << b64_;
    b64Out = ss.str();
    free(encName);
    return(0);
}

void SMTP::display(std::string in)
{
    std::unordered_map<Response, std::string, hFn> commandList;
    parseSmtpCommand(in, commandList);
    for(const auto& elm: commandList) {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l m_reply:%u m_statusCode:%s\n"), 
                   elm.first.m_reply, elm.first.m_statusCode.c_str()));        
    }
}

void SMTP::GREETING::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::GREETING function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::GREETING::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::GREETING function:%s\n"), __PRETTY_FUNCTION__));
}

std::uint32_t SMTP::GREETING::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::GREETING receive length:%d response:%s\n"), in.length(), in.c_str()));
    return(0);
}

std::uint32_t SMTP::GREETING::onResponse()
{
    return(0);
}

std::uint32_t SMTP::GREETING::onResponse(std::string in, std::string& out, States& new_state)
{
    auto statusCode = getSmtpStatusCode(in);
    auto retStatus = 0;

    switch(statusCode.m_reply) {
        case SMTP::STATUS_CODE_220_Service_ready:
            display(in); 
            /* connection established successfully - send the next command */
            retStatus = onCommand(in, out, new_state);
        break;
        case SMTP::STATUS_CODE_554_Transaction_has_failed:
        break;
        default:
            display(in);
        break;
    }
    return(retStatus);
}

/**
 * @brief This member method processes the incoming initial command from smtp server and sends EHLO - Extension Hello
 * 
 * @param in Greeting or Extension capabilities from smtp server
 * @param out response message to be sent to smtp server
 * @param new_state new state for processing of Extention cabalities 
 */
std::uint32_t SMTP::GREETING::onCommand(std::string in, std::string& out, States& new_state)
{
    std::stringstream ss("");
    ss << "EHLO gmail.com" << "\r\n";
    /// @brief modifiying out with response message to be sent to smtp server 
    out = ss.str();
    /// @brief moving to new state for processing of new command or request 
    new_state = HELO{};
    //new_state = MAIL{};
    return(0);
}

void SMTP::HELO::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::HELO function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::HELO::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::HELO function:%s\n"), __PRETTY_FUNCTION__));
}

std::uint32_t SMTP::HELO::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::HELO receive length:%d response:%s\n"), 
               in.length(), in.c_str()));
    return(0);
}

std::uint32_t SMTP::HELO::onResponse()
{
    return(0);
}

std::uint32_t SMTP::HELO::onResponse(std::string in, std::string& out, States& new_state)
{
    auto statusCode = getSmtpStatusCode(in);
    auto retStatus = 0;

    switch(statusCode.m_reply) {
        case SMTP::STATUS_CODE_250_Request_mail_action_okay_completed:
            display(in);
            retStatus = onCommand(in, out, new_state);
        break;

        default:
            display(in);
        break;
    }
    return(retStatus);
}

/**
 * @brief This member method processes the incoming initial command from smtp server and sends EHLO - Extension Hello
 * 
 * @param in Greeting or Extension capabilities from smtp server
 * @param out response message to be sent to smtp server
 * @param new_state new state for processing of Extention cabalities 
 */
std::uint32_t SMTP::HELO::onCommand(std::string in, std::string& out, States& new_state)
{
    std::stringstream ss("");
    //ss << "MAIL FROM: HM Royal<hnm.royal@gmail.com>" << "\r\n";
    ss << "STARTTLS" << "\r\n";
    /// @brief modifiying out with response message to be sent to smtp server 
    out = ss.str();
    /// @brief moving to new state for processing of new command or request 
    new_state = MAIL{};
    return(0);
}

void SMTP::MAIL::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL function:%s\n"), __PRETTY_FUNCTION__));
    m_authStage = AUTH_INIT;
}

void SMTP::MAIL::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL function:%s\n"), __PRETTY_FUNCTION__));
}

std::uint32_t SMTP::MAIL::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL receive length:%d response:%s\n"),
               in.length(), in.c_str()));
    
    return(0);
}

std::uint32_t SMTP::MAIL::onResponse()
{
    return(0);
}

std::uint32_t SMTP::MAIL::onResponse(std::string in, std::string& out, States& new_state)
{
    auto statusCode = getSmtpStatusCode(in);
    auto retStatus = 0;

    switch(statusCode.m_reply) {
        case SMTP::STATUS_CODE_530_5_7_0_Authentication_needed:
        case SMTP::STATUS_CODE_334_Server_challenge:
        case SMTP::STATUS_CODE_250_Request_mail_action_okay_completed:
            display(in);
            retStatus = onCommand(in, out, new_state);
        break;
        case STATUS_CODE_535_5_7_8_Authentication_credentials_invalid:
        
        break;
        case STATUS_CODE_220_Service_ready:
            /* switch to TLS */
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL authnetication over tls started\n")));
            retStatus = onCommand(in, out, new_state);
        break;
        default:
            display(in);
            retStatus = onCommand(in, out, new_state);
        break;

    }
    return(retStatus);
}

std::uint32_t SMTP::MAIL::onCommand(std::string in, std::string& out, States& new_state)
{
    std::stringstream ss("");

    if(AUTH_INIT == m_authStage) {
        std::string result("");
        ss << "AUTH LOGIN" << "\r\n";
        /// @brief modifiying out with response message to be sent to smtp server 
        out = ss.str();
        m_authStage = AUTH_USRNAME;
        /// @brief remain in same state
        return(1);
    } else if(AUTH_USRNAME == m_authStage) {
        if(!onUsername(in, out)) {
            m_authStage = AUTH_PASSWORD;
        }
        /// @brief remain in same state
        return(1);

    } else if(AUTH_PASSWORD == m_authStage) {
        if(!onPassword(in, out)) { 
            m_authStage = AUTH_SUCCESS;
        }
        /// @brief remain in same state
        return(1);
    } else if(AUTH_SUCCESS == m_authStage) {
        if(!onLoginSuccess(in, out)) {
          ss << "RESET" << "\r\n";
          out = ss.str();
          m_authStage = AUTH_INIT;
          new_state = RESET{};
          return(0);
        }
    } 
    /*go to next state */
    new_state = RCPT{};
    
    return(0);
}

std::uint32_t SMTP::MAIL::onUsername(const std::string in, std::string& base64Username)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL function:%s\n"),__PRETTY_FUNCTION__));
    size_t out_len = 0;
    auto status = SMTP::getSmtpStatusCode(in);
    
    if((SMTP::STATUS_CODE_334_Server_challenge == status.m_reply) && !status.m_statusCode.empty()) {
        const ACE_Byte* data = (ACE_Byte *)(status.m_statusCode.data());
        ACE_Byte* plain = ACE_Base64::decode(data, &out_len);

        if(plain) {
            std::string usrName((char *)plain, out_len);
            if(!usrName.compare("Username:")) {
                std::string nm("hnm.royal@gmail.com");
                std::string b64_;
                auto len = SMTP::getBase64(nm, b64_);
                std::stringstream ss("");
                ss << b64_;
                base64Username = ss.str();
                return(0);
            }
        }
    }
    return(1);
}

std::uint32_t SMTP::MAIL::onPassword(const std::string in, std::string& base64Username)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL function:%s\n"),__PRETTY_FUNCTION__));
    size_t out_len = 0;
    auto status = SMTP::getSmtpStatusCode(in);
    if(SMTP::STATUS_CODE_334_Server_challenge == status.m_reply && 
       !status.m_statusCode.empty()) {

        const ACE_Byte* data = (ACE_Byte *)(status.m_statusCode.data());
        ACE_Byte* plain = ACE_Base64::decode(data, &out_len);
        if(plain) {
            std::string pwd((char *)plain, out_len);
            if(!pwd.compare("Password:")) {
                std::string nm("");
                std::string b64_;
                auto len = SMTP::getBase64(nm,b64_);
                std::stringstream ss("");
                ss << b64_;
                base64Username = ss.str();
                return(0);
            }
        }
    }
    return(1);
}

bool SMTP::MAIL::onLoginSuccess(const std::string in, std::string& out)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL function:%s\n"),__PRETTY_FUNCTION__));

    auto status = SMTP::getSmtpStatusCode(in);
    if(SMTP::STATUS_CODE_235_2_7_0_Authentication_succeeded == status.m_reply && 
       !status.m_statusCode.compare("2.7.0")) {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL authentication over tls is sucessful\n")));

        std::stringstream ss("");
        ss << "MAIL FROM: <hnm.royal@gmail.com>" << "\r\n";
        /// @brief modifiying out with response message to be sent to smtp server 
        out = ss.str();
        return(true);
    }
    return(false);
}

void SMTP::DATA::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::DATA function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::DATA::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::DATA function:%s\n"), __PRETTY_FUNCTION__));
}

std::uint32_t SMTP::DATA::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::DATA receive length:%d response:%s\n"), in.length(), in.c_str()));
    return(0);
}

std::uint32_t SMTP::DATA::onResponse()
{
    return(0);
}

std::uint32_t SMTP::DATA::onResponse(std::string in, std::string& out, States& new_state)
{
    auto statusCode = getSmtpStatusCode(in);
    auto retStatus = 0;
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::DATA m_reply:%u m_statusCode:%s\n"), 
              statusCode.m_reply, statusCode.m_statusCode.c_str()));

    switch(statusCode.m_reply) {
        case SMTP::STATUS_CODE_235_2_7_0_Authentication_succeeded:
            display(in);
            retStatus = onCommand(in, out, new_state);
        break;
        case STATUS_CODE_535_5_7_8_Authentication_credentials_invalid:
        
        break;
        case STATUS_CODE_220_Service_ready:
            /* switch to TLS */
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL authnetication over tls started\n")));
            retStatus = onCommand(in, out, new_state);
        break;
        default:
            display(in);
            retStatus = onCommand(in, out, new_state);
        break;

    }
    return(retStatus);
}
std::uint32_t SMTP::DATA::onCommand(std::string in, std::string& out, States& new_state)
{

    std::stringstream ss("");
    ss << "DATA" << "\r\n";
    //ss << "RCPT TO: <naushad.dln@gmail.com>" << "\r\n";
    /// @brief modifiying out with response message to be sent to smtp server 
    out = ss.str();
 
    new_state = BODY{};
    return(0);
}

void SMTP::BODY::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::BODY function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::BODY::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::BODY function:%s\n"), __PRETTY_FUNCTION__));
}

std::uint32_t SMTP::BODY::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::BODY receive length:%d response:%s\n"), in.length(), in.c_str()));
    return(0);
}

std::uint32_t SMTP::BODY::onResponse()
{
    return(0);
}
std::uint32_t SMTP::BODY::onResponse(std::string in, std::string& out, States& new_state)
{
    auto statusCode = getSmtpStatusCode(in);
    auto retStatus = 0;
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::BODY m_reply:%u m_statusCode:%s\n"), 
              statusCode.m_reply, statusCode.m_statusCode.c_str()));

    switch(statusCode.m_reply) {
        case SMTP::STATUS_CODE_250_Request_mail_action_okay_completed:
            display(in);
            retStatus = onCommand(in, out, new_state);
        break;
        case STATUS_CODE_535_5_7_8_Authentication_credentials_invalid:
        
        break;
        case STATUS_CODE_220_Service_ready:
            /* switch to TLS */
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL authnetication over tls started\n")));
            retStatus = onCommand(in, out, new_state);
        break;
        default:
            display(in);
            retStatus = onCommand(in, out, new_state);
        break;

    }
    return(retStatus);
}
std::uint32_t SMTP::BODY::onCommand(std::string in, std::string& out, States& new_state)
{

    std::stringstream ss("");
    //ss << "DATA" << "\r\n";
    
    ss << "\r\nFrom: \"HNM Royal\" <hnm.royal@gmail.com>" << "\r\n"
       << "To: <naushad.dln@gmail.com>" << "\r\n"
       << "Subject: Test e-mail" << "\r\n"
       << "\r\n" << "This is from SMTP Client\r\n" 
       <<"\r\n"<< "." <<"\r\n";
    

    /// @brief modifiying out with response message to be sent to smtp server 
    out = ss.str();
 
    new_state = QUIT{};
    return(1);
}

void SMTP::RCPT::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::RCPT function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::RCPT::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::RCPT function:%s\n"), __PRETTY_FUNCTION__));
}

std::uint32_t SMTP::RCPT::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::RCPT receive length:%d response:%s\n"), in.length(), in.c_str()));
    return(0);
}

std::uint32_t SMTP::RCPT::onResponse()
{
    return(0);
}

std::uint32_t SMTP::RCPT::onResponse(std::string in, std::string& out, States& new_state)
{
    auto statusCode = getSmtpStatusCode(in);
    auto retStatus = 0;
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::RCPT m_reply:%u m_statusCode:%s\n"), 
              statusCode.m_reply, statusCode.m_statusCode.c_str()));

    switch(statusCode.m_reply) {
        case SMTP::STATUS_CODE_250_Request_mail_action_okay_completed:
            display(in);
            retStatus = onCommand(in, out, new_state);
        break;
        case STATUS_CODE_535_5_7_8_Authentication_credentials_invalid:
        
        break;
        case STATUS_CODE_220_Service_ready:
            /* switch to TLS */
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL authnetication over tls started\n")));
            retStatus = onCommand(in, out, new_state);
        break;
        default:
            display(in);
            retStatus = onCommand(in, out, new_state);
        break;

    }
    return(retStatus);
}

std::uint32_t SMTP::RCPT::onCommand(std::string in, std::string& out, States& new_state)
{

    std::stringstream ss("");
    ss << "RCPT TO: <naushad.dln@gmail.com>" << "\r\n";
    /// @brief modifiying out with response message to be sent to smtp server 
    out = ss.str();
    new_state = DATA{};
    return(0);
}

void SMTP::QUIT::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::QUIT function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::QUIT::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::QUIT function:%s\n"), __PRETTY_FUNCTION__));
}

std::uint32_t SMTP::QUIT::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::uint32_t SMTP::QUIT::onResponse()
{
    return(0);
}

std::uint32_t SMTP::QUIT::onResponse(std::string in, std::string& out, States& new_state)
{

}

std::uint32_t SMTP::QUIT::onCommand(std::string in, std::string& out, States& new_state)
{

    std::stringstream ss("");
    ss << "QUIT" << "\r\n";
    /// @brief modifiying out with response message to be sent to smtp server 
    out = ss.str();

    //new_state = INIT{};

}

void SMTP::RESET::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::RESET function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::RESET::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::RESET function:%s\n"), __PRETTY_FUNCTION__));
}

std::uint32_t SMTP::RESET::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::uint32_t SMTP::RESET::onResponse()
{
    return(0);
}

std::uint32_t SMTP::RESET::onResponse(std::string in, std::string& out, States& new_state)
{

}

std::uint32_t SMTP::RESET::onCommand(std::string in, std::string& out, States& new_state)
{
    //new_state = INIT{};

}

void SMTP::VRFY::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::VRFY function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::VRFY::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::VRFY function:%s\n"), __PRETTY_FUNCTION__));
}

std::uint32_t SMTP::VRFY::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::uint32_t SMTP::VRFY::onResponse()
{
    return(0);
}

std::uint32_t SMTP::VRFY::onResponse(std::string in, std::string& out, States& new_state)
{

}

std::uint32_t SMTP::VRFY::onCommand(std::string in, std::string& out, States& new_state)
{
    //new_state = INIT{};

}


void SMTP::NOOP::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::NOOP function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::NOOP::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::NOOP function:%s\n"), __PRETTY_FUNCTION__));
}

std::uint32_t SMTP::NOOP::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::uint32_t SMTP::NOOP::onResponse()
{
    return(0);
}

std::uint32_t SMTP::NOOP::onResponse(std::string in, std::string& out, States& new_state)
{

}
std::uint32_t SMTP::NOOP::onCommand(std::string in, std::string& out, States& new_state)
{
    //new_state = INIT{};

}

void SMTP::EXPN::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::EXPN function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::EXPN::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::EXPN function:%s\n"), __PRETTY_FUNCTION__));
}

std::uint32_t SMTP::EXPN::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::uint32_t SMTP::EXPN::onResponse()
{
    return(0);
}

std::uint32_t SMTP::EXPN::onResponse(std::string in, std::string& out, States& new_state)
{

}
std::uint32_t SMTP::EXPN::onCommand(std::string in, std::string& out, States& new_state)
{
    //new_state = INIT{};

}

void SMTP::HELP::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::HELP function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::HELP::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::HELP function:%s\n"), __PRETTY_FUNCTION__));
}

std::uint32_t SMTP::HELP::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::uint32_t SMTP::HELP::onResponse()
{
    return(0);
}

std::uint32_t SMTP::HELP::onResponse(std::string in, std::string& out, States& new_state)
{
    auto statusCode = SMTP::getSmtpStatusCode(in);
    switch(statusCode.m_reply) {
        case SMTP::STATUS_CODE_250_Request_mail_action_okay_completed:
        break;
        default:
        break;
    }
}

std::uint32_t SMTP::HELP::onCommand(std::string in, std::string& out, States& new_state)
{
    std::stringstream ss("");
    ss << "AUTH LOGIN" << "\r\n";
    /// @brief modifiying out with response message to be sent to smtp server 
    out = ss.str();
    /// @brief moving to new state for processing of new command or request 
    new_state = MAIL{};
    return(0);
}

#endif /* __emailservice_fsm_cc__ */