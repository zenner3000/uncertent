#ifndef BITMEX_H
#define BITMEX_H

#include "uWS.h"
#include "exchange.h"
#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
#include "common.h"
#include "rapidjson/document.h"
#include "sqlite3.h"

using namespace std;
using namespace rapidjson;


class bitmex
{
    public:
        bitmex();
        virtual ~bitmex();


        //websocket
        void   server_sign();
        void   threadfunc_stream();
        void   start_stream();
        void   closestream();
        void   subscribe_depth(string symbol);
        void   cancel_subscribe_depth(string symbol);
        void   sendmsg(string msg);
        void   parse_priceamount_to_map(string action,const Value &data);

        map<string,askbidtable> symbol_askbid_table;
        //uWS::Hub h; // can not be pointer
        uWS::WebSocket<uWS::CLIENT> *ws;
        bool   wsconn_state;
        mutex mu;
        condition_variable cv;
        bool sub_state;

        //sqlite3
        sqlite3 *m_db;
        string m_dbname;
        bool insertprice(string tablename,long msgid,double price,double amount);
        bool deleteprice(string tablename,long msgid);
        bool updateprice(string tablename,long msgid,int amount);
        bool selectprice(string tablename);
        void clear_table_data(string table);
        bool init_table();

        const string restdomain = "https://api.binance.com";
        const string wssdomain  = "wss://www.bitmex.com/realtime";

        const string apikey     = "riROnR5C3hZvUA1aZ3Hrc6Mov4byFAko4mkTPLjMFEFpeTfczvVPlkhO7MDy0PfF";
        const string seckey     = "nley1XkqtVJXs6PEt5k0FCimRqTUL22BdzzWihNCMjayVt21IZfjcqvTTEjj2Hyh";

        const string getheader  = "Content-Type:application/x-www-form-urlencoded";
        const vector<string> headerlist ={"Content-Type:application/x-www-form-urlencoded",
                                          "X-MBX-APIKEY: " + this->apikey};
                                          //"Accept-Encoding: gzip, deflate"
    protected:

    private:
};

#endif // BITMEX_H
