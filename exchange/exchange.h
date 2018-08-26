#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <string>
using namespace std;


class exchange
{
    public:
        exchange();
        virtual ~exchange();
        virtual string create_limit_buy_order(string symbol, double amount, double price)=0;
        virtual string create_limit_sell_order(string symbol, double amount, double price)=0;
        //virtual string cancel_order()=0;
        //virtual string get_openorders()=0;
        //virtual string get_symbols()=0;
        virtual string get_balance()=0;
    protected:
    private:
};


#endif // EXCHANGE_H
