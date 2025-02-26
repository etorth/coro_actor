#include "actor.hpp"
#include "utils.hpp"

FreeMsgCoro Actor::on_MPK_INIT(Message)
{
    for(const int toAddr: {getAddress() - 1, getAddress() + 1}){
        if(toAddr > 0){
            const auto reply = co_await send({toAddr, 0}, MessagePack
            {
                .type = MPK_HELLO,
            });
            m_replyList.push_back(std::move(reply.content));
        }
    }
}

FreeMsgCoro Actor::on_MPK_HELLO(Message msg)
{
    if(msg.from > getAddress()){
        post(msg.fromAddr(), MessagePack
        {
            .type = MPK_STRING,
            .content = str_printf("Hi master %d, I am %d", msg.from, getAddress()),
        });
    }
    else{
        const auto reply = co_await send({msg.from, 0}, MessagePack
        {
            .type = MPK_QUERYNAME,
        });

        if(reply.content > m_name){
            post(msg.fromAddr(), MessagePack
            {
                .type = MPK_STRING,
                .content = str_printf("Hi friend %d, I am %d", msg.from, getAddress()),
            });
        }
        else{
            post(msg.fromAddr(), MessagePack
            {
                .type = MPK_STRING,
                .content = str_printf("Hi little %d, I am %d", msg.from, getAddress()),
            });
        }
    }
}

FreeMsgCoro Actor::on_MPK_QUERYNAME(Message msg)
{
    post(msg.fromAddr(), MessagePack
    {
        .type = MPK_STRING,
        .content = m_name,
    });
    return {};
}

FreeMsgCoro Actor::on_MPK_QUERYCREATETIME(Message msg)
{
    post(msg.fromAddr(), MessagePack
    {
        .type = MPK_STRING,
        .content = str_printf("%llu", m_createTime),
    });
    return {};
}
