#ifndef HUOBI_H
#define HUOBI_H

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

/*
every method
1 parm list  --> map parm
2 make parm string by map , (using & connect)
3 sign = sha256base64
4 domain + parmlist  + & + sign =value
5 http request

class
sign
httprequest(string url)
create order (cointain ,/xxx/v1/vyyy)
cancel order
get open orders
get specific order detail
fetch symbols
get balance
domain ,seckey,acckey

*/
class huobipro : public exchange
{
    public:
        huobipro();
        string sign(string data,string seckey);
        string create_limit_buy_order(string symbol, double amount, double price);
        string create_limit_sell_order(string symbol, double amount, double price);
        string cancel_order();
        string get_openorders();
        string get_symbols();
        string get_balance();
        virtual ~huobipro();
        void   get_symbol_streamdata(string symbol, map<double,double>  &bidtable, map<double,double>  &asktable);

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
        const string wssdomain  = "wss://api.huobi.pro/ws";

        const string apikey = "f86e967d-3119bb92-bb6b5a14-4f538";
        const string seckey = "ae135856-91dbb36a-cee4149f-8e037";


        const vector<string> headerlist_get ={"Content-Type:application/x-www-form-urlencoded",
                                          "User-Agent:Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.71 Safari/537.36"};
        const vector<string> headerlist_post ={"Content-Type:application/json",
                                          "User-Agent:Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.71 Safari/537.36"};
    protected:
    private:
};

#endif
