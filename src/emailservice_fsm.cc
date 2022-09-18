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

void SMTP::INIT::onRequest(std::string in, std::string& out, States& new_state)
{
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

void SMTP::DATA::onRequest(std::string in, std::string& out, States& new_state)
{
    new_state = INIT{};
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

void SMTP::RCPT::onRequest(std::string in, std::string& out, States& new_state)
{
    new_state = DATA{};
    
}


void SMTP::MAIL::onEntry()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:MAIL function:%s\n"), __PRETTY_FUNCTION__));
}

void SMTP::MAIL::onExit()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:MAIL function:%s\n"), __PRETTY_FUNCTION__));
}

std::int32_t SMTP::MAIL::onResponse(std::string in)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [mailservice:%t] %M %N:%l ST:MAIL receive length:%d response:%s\n"), in.length(), in.c_str()));
    
    return(0);
}

std::int32_t SMTP::MAIL::onResponse()
{
    return(0);
}

void SMTP::MAIL::onRequest(std::string in, std::string& out, States& new_state)
{
    new_state = INIT{};

}

#endif /* __emailservice_fsm_cc__ */