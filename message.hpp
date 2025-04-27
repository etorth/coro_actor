#pragma once
#include <string>
#include <string_view>
#include "utils.hpp"

enum MsgType: int
{
    MPK_NONE = 0,
    MPK_BADADDR,
    MPK_INIT,
    MPK_HELLO,
    MPK_QUERYNAME,
    MPK_QUERYBOOL,
    MPK_STRING,
    MPK_END,
};

inline const char *msgTypeStr(int type)
{
    switch(type){
        case MPK_NONE     : return "MPK_NONE";
        case MPK_BADADDR  : return "MPK_BADADDR";
        case MPK_INIT     : return "MPK_INIT";
        case MPK_HELLO    : return "MPK_HELLO";
        case MPK_QUERYNAME: return "MPK_QUERYNAME";
        case MPK_QUERYBOOL: return "MPK_QUERYBOOL";
        case MPK_STRING   : return "MPK_STRING";
        default           : return "UNKNOWN";
    }
}

constexpr int msgTypeStrMaxLength()
{
    int maxLen = 0;
    for(int n = MPK_NONE; n < MPK_END; ++n){
        maxLen = std::max<int>(maxLen, std::string_view(msgTypeStr(n)).size());
    }
    return maxLen;
}

struct MessagePack
{
    int type = 0;
    std::string content {};

    std::string str() const
    {
        return str_printf("MsgPack(type: %-*s, content: %6s)", msgTypeStrMaxLength(), msgTypeStr(type), content.c_str());
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
        return str_printf("Message(type: %-*s, content: %6s, from: %2d, seqID: %2d, respID: %2d)", msgTypeStrMaxLength(), msgTypeStr(type), content.c_str(), from, seqID, respID);
    }

    std::pair<int, int> fromAddr() const
    {
        return {from, seqID};
    }
};
