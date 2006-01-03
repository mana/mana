#ifndef _TMW_PROTOCOL_HELPER_
#define _TMW_PROTOCOL_HELPER_

#include "protocol.h"


/**
 * Helper class for using <actionfunc>(cancel), to cancel stuff
 */
class cancel_t {};
extern cancel_t cancel;

class complete_t {};
extern complete_t complete;


class Item;

/** Start trading with someone */
void trade(Being&);

/** add trading item*/
void trade(Item&,int amount);

/** add money to trade */
void trade(int);

/** verify the trade */
void trade();

void trade(complete_t&);

/** cancel courent trade */
void trade(cancel_t&);

#endif
