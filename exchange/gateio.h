#ifndef GATEIO_H
#define GATEIO_H

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

class gateio
{
    public:
        gateio();
        virtual ~gateio();


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
        const string wssdomain  = "wss://ws.gate.io/v3/";

        const string apikey = "F97A4D30-13C6-4F08-BBFD-E550620EFEEE";
        const string seckey = "2e9145c12bd96b4576c40bc672d79271939b1b433a17dc4710dac2dbfb52a9ea";

    protected:

    private:
};

#endif // GATEIO_H
