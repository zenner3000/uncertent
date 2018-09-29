#ifndef DERIBIT_H
#define DERIBIT_H

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



struct req{
    int     id;
    string  action;
    string  instrument;
    string  event;
    string  sig;
};

class deribit
{
    public:
        deribit();
        virtual ~deribit();


        string makereqstr(int id,string action,string instrument,string event,string sig);

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


        const string restdomain     = "https://api.huobi.pro";
        const string wssdomain      = "wss://www.deribit.com/ws/api/v1/";
        const string wssdomaintest  = "wss://test.deribit.com/ws/api/v1/";

        const string apikey = "2Z3eSWDjK7hQ5";
        const string seckey = "YA45DYDRHQL4RDRT7QYXY5SUC4ZKKMHM";
    protected:

    private:
};

#endif // DERIBIT_H
