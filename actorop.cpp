#include "actor.hpp"
#include "utils.hpp"

corof::entrance Actor::on_MPK_INIT(Message)
{
    for(const int toAddr: {getAddress() - 1, getAddress() + 1}){
        if(toAddr > 0){
            const auto reply = co_await send({toAddr, 0}, MessagePack
            {
                .type = MPK_HELLO,
            });

            // reply message can be used to update actor's internal state
            // an example shows how reply gets consumed

            if(reply.type == MPK_BADADDR){
                m_badAddrCount++;
            }
            else{
                m_replyCharCount += reply.content.size();
                while(co_await queryBool(toAddr)){
                    continue;
                }
            }
        }
    }
}

corof::entrance Actor::on_MPK_HELLO(Message msg)
{
    if(msg.from > getAddress()){
        post(msg.fromAddr(), MessagePack
        {
            .type = MPK_STRING,
            .content = "master",
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
                .content = "friend",
            });
        }
        else{
            post(msg.fromAddr(), MessagePack
            {
                .type = MPK_STRING,
                .content = "stupid",
            });
        }
    }
}

corof::entrance Actor::on_MPK_QUERYNAME(Message msg)
{
    post(msg.fromAddr(), MessagePack
    {
        .type = MPK_STRING,
        .content = m_name,
    });
    return {};
}

corof::entrance Actor::on_MPK_QUERYBOOL(Message msg)
{
    post(msg.fromAddr(), MessagePack
    {
        .type = MPK_STRING,
        .content = std::to_string(randint() % 10),
    });
    return {};
}
