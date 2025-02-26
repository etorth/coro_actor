#pragma once
#include <string>
#include "utils.hpp"

enum MsgType: int
{
    MPK_NONE = 0,
    MPK_BADADDR,
    MPK_INIT,
    MPK_HELLO,
    MPK_QUERYNAME,
    MPK_QUERYCREATETIME,
    MPK_STRING,
};

inline const char *msgTypeStr(int type)
{
    switch(type){
        case MPK_NONE           : return "MPK_NONE";
        case MPK_BADADDR        : return "MPK_BADADDR";
        case MPK_INIT           : return "MPK_INIT";
        case MPK_HELLO          : return "MPK_HELLO";
        case MPK_QUERYNAME      : return "MPK_QUERYNAME";
        case MPK_QUERYCREATETIME: return "MPK_QUERYCREATETIME";
        case MPK_STRING         : return "MPK_STRING";
        default                 : return "UNKNOWN";
    }
}

struct MessagePack
{
    int type = 0;
    std::string content {};

    std::string str() const
    {
        return str_printf("MessagePack(type: %s, content: %s)", msgTypeStr(type), content.empty() ? "(empty)" : content.c_str());
    }
};

struct Message
{
    int type = 0;
    std::string content {};

    int from   = 0;
    int seqID  = 0;
    int respID = 0;

    std::string str() const
    {
        return str_printf("Message(type: %s, content: %s, from: %d, seqID: %d, respID: %d)", msgTypeStr(type), content.empty() ? "(empty)" : content.c_str(), from, seqID, respID);
    }

    std::pair<int, int> fromAddr() const
    {
        return {from, seqID};
    }
};
