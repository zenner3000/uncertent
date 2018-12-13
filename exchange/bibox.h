#ifndef BIBOX_H
#define BIBOX_H

#include "uWS.h"
#include "exchange.h"
#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
#include "common.h"
#include "rapidjson/document.h"

using namespace std;
using namespace rapidjson;

class bibox
{
    public:
        bibox();
        virtual ~bibox();


        //websocket
        void   threadfunc_stream();
        void   start_stream();
        void   closestream();
        void   subscribe_depth(string symbol);
        void   cancel_subscribe_depth(string symbol);
        void   sendmsg(string msg);
        void   parse_priceamount_to_map(string symbol,const Value &data);

        map<string,askbidtable> symbol_askbid_table;
        uWS::WebSocket<uWS::CLIENT> *ws;
        bool   wsconn_state;
        mutex mu;
        condition_variable cv;
        bool sub_state;

        const string restdomain = "https://api.huobi.pro";
        const string wssdomain  = "wss://api.zb.com:9999/websocket";

    protected:

    private:
};

#endif // BIBOX_H
