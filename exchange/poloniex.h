#ifndef POLONIEX_H
#define POLONIEX_H

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




class poloniex
{
    public:
        poloniex();
        virtual ~poloniex();


        //websocket
        void   server_sign();
        void   threadfunc_stream();
        void   start_stream();
        void   closestream();
        void   subscribe_depth(string symbol);
        void   cancel_subscribe_depth(string symbol);
        void   sendmsg(string msg);
        void   parse_priceamount_to_map(string symbol,const Value &data);

        map<string,askbidtable> symbol_askbid_table;
        //uWS::Hub h; // can not be pointer
        uWS::WebSocket<uWS::CLIENT> *ws;
        bool   wsconn_state;
        mutex mu;
        condition_variable cv;
        bool sub_state;

        const string restdomain = "https://www.okex.com/api/v1";
        const string wssdomain  = "wss://api2.poloniex.com";

        const string apikey = "615H9MDE-PK34W3QN-NQV0JIRS-XJ64XZHU";
        const string seckey = "f54fb0563a9bbb82cd521aebc4c2b0bfbecea30422cf9dd6dc7f9dbd159eba6a199abc93c9e4e21a5702d06ec7730cb4bf0103667a56fe5a6613f0ede830ee6b";

    protected:

    private:
};

#endif // POLONIEX_H
