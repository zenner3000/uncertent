#ifndef EXG58COIN_H
#define EXG58COIN_H

#include "uWS.h"
#include <string>
#include <map>
#include "common.h"
#include <mutex>
#include <condition_variable>
using namespace std;

class exg58coin
{
    public:
        exg58coin();
        virtual ~exg58coin();

        //websocket
        void   threadfunc_stream();
        void   start_stream();
        void   closestream();
        void   subscribe_depth(string symbol);
        void   cancel_subscribe_depth(string symbol);
        void   sendmsg(string msg);
        //void   parse_priceamount_to_map(string symbol,const Value &data);
        void   heartbeat();

        uWS::WebSocket<uWS::CLIENT> *ws;
        map<string,askbidtable> symbol_askbid_table;
        bool   wsconn_state;
        mutex mu;
        condition_variable cv;
        bool sub_state;

        const string wssdomain  = "wss://ws.58coin.com/websocket";

    protected:

    private:
};

#endif // EXG58COIN_H
