#ifndef BINANCE_H
#define BINANCE_H

#include "exchange.h"
#include <string>
#include <vector>
#include <map>
#include <thread>
#include "uWS.h"
#include "common.h"
#include "rapidjson/document.h"
using namespace rapidjson;
using namespace std;

struct depthinfo{
        int     lastUpdateId;
        int     prev_u;
};

class binance : public exchange
{
    public:
        binance();
        virtual ~binance();

        //rest api
        string create_limit_buy_order(string symbol, double amount, double price);
        string create_limit_sell_order(string symbol, double amount, double price);
        string cancel_order();
        string test(string symbol, double amount, double price);
        string get_balance();

        //websocket
        void    threadfunc_stream();
        void    start_stream();
        void    closestream();
        void    subscribe_depth(string symbol);
        void    subscribe_mutil_depth();
        void    cancel_subscribe_depth(string symbol);
        void    sendmsg(string msg);
        string  get_symbol_uri();
        string  get_snapshot_depth(string symbol,int limit);
        void    parse_snapshot_to_map(string json,string smb);
        void    parse_priceamount_to_map(string symbol,const Value &data);
        map<string,depthinfo> depth_info;



        map<string,askbidtable> symbol_askbid_table;  //save all symbols ask bid info
        uWS::Hub h; // can not be pointer
        uWS::WebSocket<uWS::CLIENT> *ws;
        bool   wsconn_state;
        thread t;

        const string uri_snapshot_depth = "https://www.binance.com/api/v1/depth?symbol=";
        const string restdomain = "https://api.binance.com";
        const string wssdomain  = "wss://stream.binance.com:9443";

        const string apikey     = "riROnR5C3hZvUA1aZ3Hrc6Mov4byFAko4mkTPLjMFEFpeTfczvVPlkhO7MDy0PfF";
        const string seckey     = "nley1XkqtVJXs6PEt5k0FCimRqTUL22BdzzWihNCMjayVt21IZfjcqvTTEjj2Hyh";

        const string getheader  = "Content-Type:application/x-www-form-urlencoded";
        const vector<string> headerlist ={"Content-Type:application/x-www-form-urlencoded",
                                          "X-MBX-APIKEY: " + this->apikey};
                                          //"Accept-Encoding: gzip, deflate"
    protected:

    private:
};

#endif // BINANCE_H
