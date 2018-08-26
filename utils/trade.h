#ifndef TRADE_H_INCLUDED
#define TRADE_H_INCLUDED


#include <thread>
#include <iostream>
#include "exchange.h"
using namespace std;

void buy(exchange *exg, string symbol,double buyquantity, double buyprice);
void sell(exchange *exg, string symbol,double sellquantity, double sellprice);
void buy_sell(exchange *exgbuy, string symbol_buy, exchange *exgsell, string symbol_sell, double quantity,double buyprice, double sellprice);
double profitcalc(double sellprice,
                double sellquantity,
                double sell_feerate,
                double buyprice,
                double buyquantity,
                double buy_feerate);


#endif // PROFIT_H_INCLUDED
