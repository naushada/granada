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

std::uint32_t parseSmtpCommand(const std::string in, std::vector<std::string>& out)
{
    std::stringstream input(in);
    std::int32_t c;
    std::string elm("");
    out.clear();
   
    std::cout << "Request:" << in << std::endl;

    while((c = input.get()) != EOF) {
        switch(c) {
            case ' ':
            {
                out.push_back(elm);
                auto cc = input.get();
                elm.clear();

                while(cc != '\r') {
                    elm.push_back(cc);
                    cc = input.get();
                }
                /* get rid of '\n' */
                cc = input.get();
                out.push_back(elm);
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

std::uint32_t getBase64(const std::string in, std::string& b64Out)
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

void SMTP::INIT::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:INIT function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::INIT::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:INIT function:%s\n"), __PRETTY_FUNCTION__));
}

std::int32_t SMTP::INIT::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:INIT receive length:%d response:%s\n"), in.length(), in.c_str()));
    return(0);
}

std::int32_t SMTP::INIT::onResponse()
{
    return(0);
}

/**
 * @brief This member method processes the incoming initial command from smtp server and sends EHLO - Extension Hello
 * 
 * @param in Greeting or Extension capabilities from smtp server
 * @param out response message to be sent to smtp server
 * @param new_state new state for processing of Extention cabalities 
 */
std::uint32_t SMTP::INIT::onRequest(std::string in, std::string& out, States& new_state)
{
    std::stringstream ss("");
    ss << "EHLO gmail.com" << "\r\n";
    /// @brief modifiying out with response message to be sent to smtp server 
    out = ss.str();
    /// @brief moving to new state for processing of new command or request 
    new_state = MAIL{};
}

void SMTP::DATA::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:DATA function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::DATA::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:DATA function:%s\n"), __PRETTY_FUNCTION__));
}

std::int32_t SMTP::DATA::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:DATA receive length:%d response:%s\n"), in.length(), in.c_str()));
    return(0);
}

std::int32_t SMTP::DATA::onResponse()
{
    return(0);
}

std::uint32_t SMTP::DATA::onRequest(std::string in, std::string& out, States& new_state)
{

    std::stringstream ss("");
    ss << "DATA" << "\r\n";
    /// @brief modifiying out with response message to be sent to smtp server 
    out = ss.str();
 
    new_state = BODY{};
}

void SMTP::BODY::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::BODY function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::BODY::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::BODY function:%s\n"), __PRETTY_FUNCTION__));
}

std::int32_t SMTP::BODY::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::BODY receive length:%d response:%s\n"), in.length(), in.c_str()));
    return(0);
}

std::int32_t SMTP::BODY::onResponse()
{
    return(0);
}

std::uint32_t SMTP::BODY::onRequest(std::string in, std::string& out, States& new_state)
{

    std::stringstream ss("");
    ss << "This is fromemail service " << "\r\n"
       << "This is line1 " << "\r\n"
       << "This is line2 " << "\r\n"
       << "." <<"\r\n";

    /// @brief modifiying out with response message to be sent to smtp server 
    out = ss.str();
 
    new_state = QUIT{};
}

void SMTP::RCPT::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:RCPT function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::RCPT::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:RCPT function:%s\n"), __PRETTY_FUNCTION__));
}

std::int32_t SMTP::RCPT::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:RCPT receive length:%d response:%s\n"), in.length(), in.c_str()));
    return(0);
}

std::int32_t SMTP::RCPT::onResponse()
{
    return(0);
}

std::uint32_t SMTP::RCPT::onRequest(std::string in, std::string& out, States& new_state)
{

    std::stringstream ss("");
    ss << "RCPT TO:<naushad.dln@gmail.com>" << "\r\n";
    /// @brief modifiying out with response message to be sent to smtp server 
    out = ss.str();
    new_state = DATA{};
    
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

std::int32_t SMTP::MAIL::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::int32_t SMTP::MAIL::onResponse()
{
    return(0);
}

std::uint32_t SMTP::MAIL::onRequest(std::string in, std::string& out, States& new_state)
{
    std::stringstream ss("");

    if(AUTH_INIT == m_authStage) {
        ss << "MAIL FROM:<naushad.dln@gmail.com>" << "\r\n"
           << "AUTH LOGIN" << "\r\n";
        
        /// @brief modifiying out with response message to be sent to smtp server 
        out = ss.str();
        m_authStage = AUTH_USRNAME;
        
        return(0);
    } else if(AUTH_USRNAME == m_authStage) {
        if(!onUsername(in, out)) {
            m_authStage = AUTH_PASSWORD;
        }
        return(0);

    } else if(AUTH_PASSWORD == m_authStage) {
        if(!onPassword(in, out)) { 
            m_authStage = AUTH_SUCCESS;
        }
        return(0);
    } else if(AUTH_SUCCESS == m_authStage) {
        if(!onLoginSuccess(in, out)) {
          ss << "RESET" << "\r\n";
          out = ss.str();
          m_authStage = AUTH_INIT;
          new_state = RESET{};
          return(1);
        }
    }

    new_state = RCPT{};
    return(1);
}

std::uint32_t SMTP::MAIL::onUsername(const std::string in, std::string& base64Username)
{
    size_t out_len = 0;
    
    std::vector<std::string> commandList;
    auto ret = parseSmtpCommand(in, commandList);
    auto statusCode = std::stoi(commandList.at(0));
    for(auto& ss: commandList) {
        std::cout << ss << std::endl;
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::MAIL element:%s\n"), ss.c_str()));
    }

    if(334 == statusCode) {
        const ACE_Byte* data = (ACE_Byte *)(commandList.at(1)).data();
        ACE_Byte* plain = ACE_Base64::decode(data, &out_len);

        if(plain) {
            std::string usrName((char *)plain, out_len);

            if(!usrName.compare("Username:")) {
                std::string nm("naushad.dln@gmail.com");
                const ACE_Byte* out = (unsigned char *)nm.data();
                out_len = 0;
                ACE_Byte* encName = ACE_Base64::encode(out, nm.length(), &out_len);
                std::string b64_((char *)encName, out_len);
                std::stringstream ss("");
                ss << b64_ << "\r\n";
                base64Username = ss.str();
                free(plain);
                return(0);
            }
        }
    }
    return(1);
}

std::uint32_t SMTP::MAIL::onPassword(const std::string in, std::string& base64Username)
{
    size_t out_len = 0;
    
    std::vector<std::string> commandList;
    auto ret = parseSmtpCommand(in, commandList);
    auto statusCode = std::stoi(commandList.at(0));
   
    if(334 == statusCode) {
        const ACE_Byte* data = (ACE_Byte *)(commandList.at(1)).data();
        ACE_Byte* plain = ACE_Base64::decode(data, &out_len);

        if(plain) {
            std::string usrName((char *)plain, out_len);
            if(!usrName.compare("Password:")) {
                std::string nm("");
                const ACE_Byte* out = (unsigned char *)nm.data();
                out_len = 0;
                ACE_Byte* encName = ACE_Base64::encode(out, nm.length(), &out_len);
                std::string b64_((char *)encName, out_len);
                std::stringstream ss("");
                ss << b64_ << "\r\n";
                base64Username = ss.str();
                free(plain);
                return(0);
            }
        }
    }
    return(1);
}
bool SMTP::MAIL::onLoginSuccess(const std::string in, std::string& out)
{
    std::vector<std::string> commandList;
    auto ret = parseSmtpCommand(in, commandList);
    auto statusCode = (commandList.size() > 0) ? std::stoi(commandList.at(0)) : 0;
   
    if(235 /*Authentication Successfull*/ == statusCode) {
        /*
        std::stringstream ss("");
        ss << "MAIL FROM: <naushad.dln@gmail.com>" << "\r\n";
        out = ss.str();
        */
        return(true);
    }
    return(false);
}
void SMTP::QUIT::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:QUIT function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::QUIT::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:QUIT function:%s\n"), __PRETTY_FUNCTION__));
}

std::int32_t SMTP::QUIT::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::int32_t SMTP::QUIT::onResponse()
{
    return(0);
}

std::uint32_t SMTP::QUIT::onRequest(std::string in, std::string& out, States& new_state)
{

    std::stringstream ss("");
    ss << "QUIT" << "\r\n";
    /// @brief modifiying out with response message to be sent to smtp server 
    out = ss.str();

    new_state = INIT{};

}

void SMTP::RESET::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:RESET function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::RESET::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:RESET function:%s\n"), __PRETTY_FUNCTION__));
}

std::int32_t SMTP::RESET::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::int32_t SMTP::RESET::onResponse()
{
    return(0);
}

std::uint32_t SMTP::RESET::onRequest(std::string in, std::string& out, States& new_state)
{
    new_state = INIT{};

}

void SMTP::VRFY::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:VRFY function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::VRFY::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:VRFY function:%s\n"), __PRETTY_FUNCTION__));
}

std::int32_t SMTP::VRFY::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::int32_t SMTP::VRFY::onResponse()
{
    return(0);
}

std::uint32_t SMTP::VRFY::onRequest(std::string in, std::string& out, States& new_state)
{
    new_state = INIT{};

}


void SMTP::NOOP::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:NOOP function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::NOOP::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:NOOP function:%s\n"), __PRETTY_FUNCTION__));
}

std::int32_t SMTP::NOOP::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::int32_t SMTP::NOOP::onResponse()
{
    return(0);
}

std::uint32_t SMTP::NOOP::onRequest(std::string in, std::string& out, States& new_state)
{
    new_state = INIT{};

}

void SMTP::EXPN::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::EXPN function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::EXPN::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::EXPN function:%s\n"), __PRETTY_FUNCTION__));
}

std::int32_t SMTP::EXPN::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::int32_t SMTP::EXPN::onResponse()
{
    return(0);
}

std::uint32_t SMTP::EXPN::onRequest(std::string in, std::string& out, States& new_state)
{
    new_state = INIT{};

}

void SMTP::HELP::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::HELP function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::HELP::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST::HELP function:%s\n"), __PRETTY_FUNCTION__));
}

std::int32_t SMTP::HELP::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::int32_t SMTP::HELP::onResponse()
{
    return(0);
}

std::uint32_t SMTP::HELP::onRequest(std::string in, std::string& out, States& new_state)
{
    new_state = INIT{};

}

#endif /* __emailservice_fsm_cc__ */