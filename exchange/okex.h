#ifndef OKEX_H
#define OKEX_H

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

class okex : public exchange
{
    public:
        okex();
        virtual ~okex();

        //rest API
        string create_limit_buy_order(string symbol, double amount, double price);
        string create_limit_sell_order(string symbol, double amount, double price);
        string cancel_order();
        string get_balance();

        //websocket
        void   threadfunc_stream();
        void   start_stream();
        void   threadfunc_heartbeat();
        void   start_heartbeat();
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
        const string wssdomain  = "wss://real.okex.com:10441/websocket";

        const string apikey = "667b485e-cf97-4c5d-b01d-18ffb8ede443";
        const string seckey = "C0458B74EDF4439C59B8EF0B3952A943";

        const string getheader  = "Content-Type:application/x-www-form-urlencoded";
        const vector<string> headerlist ={"Content-Type:application/x-www-form-urlencoded"};
    protected:

    private:
};

#endif // OKEX_H
