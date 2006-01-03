
#include "protocol_trade.h"

#include "../item.h"
#include "../gui/trade.h"

#include "messageout.h"

cancel_t cancel;
complete_t complete;

/** Start trading with someone */
void trade(Being&);

void trade(int id,int amount)
{
    MessageOut outMsg;
    outMsg.writeInt16(CMSG_TRADE_ITEM_ADD_REQUEST);
    outMsg.writeInt16(id);
    outMsg.writeInt32(amount);
};

/** add trading item*/

void trade(Item& item,int amount)
{
trade(item.getInvIndex(),amount);
}

/** add money to trade */
void trade(int i){trade(0,i);};

/** cancel courent trade */
void trade(cancel_t&)
{
    MessageOut outMsg;
    outMsg.writeInt16(CMSG_TRADE_CANCEL_REQUEST);
};

void trade()
{
    MessageOut outMsg;
    outMsg.writeInt16(CMSG_TRADE_OK);
};

void trade(complete_t&)
{
    MessageOut outMsg;
    outMsg.writeInt16(CMSG_TRADE_ADD_COMPLETE);
};
