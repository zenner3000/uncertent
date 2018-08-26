#include "okex.h"
#include <map>
#include "../utils/codec.h"
#include "../utils/misc.h"
#include "../utils/httprequest.h"
#include <string>
#include <thread>
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using  namespace std;
using namespace rapidjson;


okex::okex()
{
    //ctor
    wsconn_state = false;
}

okex::~okex()
{
    //dtor
}


string okex::create_limit_buy_order(string symbol, double amount, double price){
    string path = "/trade.do";
    map<string ,string> mparm;
    mparm["api_key"]    = this->apikey;
    mparm["symbol"]     = "btc_usdt";
    mparm["type"]       = "buy";
    mparm["price"]      = to_string(price);
    mparm["amount"]     = to_string(amount);
    //mparm["sign"]           =;
    //mparm["Timestamp"]          = escapeURL(getutctime());


    string parmstr = parmmap_to_parmstr(mparm);
    return "";
}

string okex::create_limit_sell_order(string symbol, double amount, double price){
    string path = "/trade.do";
    map<string ,string> mparm;
    mparm["api_key"]    = this->apikey;
    mparm["symbol"]     = "eos_usdt";
    mparm["type"]       = "sell";
    mparm["price"]      = to_string(price);
    mparm["amount"]     = to_string(amount);

    string parmstr      = parmmap_to_parmstr(mparm);
    string needsignstr  = parmstr + "&secret_key=" + this->seckey;
    string md5digest    = md5_then_str(needsignstr);
    string postdata = parmstr + "&sign=" + md5digest;
    string retstr   = https_post(this->restdomain + path,postdata,this->headerlist);
    return retstr;
}

string okex::cancel_order(){
    string path = "/cancel_order.do";
    return "";
}

string okex::get_balance(){
    return "";
}


/*h,out
  ws,out
*/
void   okex::start_stream(){

    thread t(&okex::threadfunc_stream,this);
    t.detach();
    //t.join();
    //return t.get_id();
}

void   okex::closestream(){
    //this->h->close();
    this->ws->terminate();
}

/*ws,in
bidtable, out
asktable, out*/
void   okex::subscribe_depth(string symbol){
    string subststr = "{\'event\':\'addChannel\',\'channel\':\'" + symbol + "\'}";
    struct askbidtable askbid_table;
    this->symbol_askbid_table[symbol] = askbid_table;

    this->sendmsg(subststr);
}

/*ws,in
subststr,in*/
void   okex::cancel_subscribe_depth(string subststr){
    this->sendmsg(subststr);
}


void   okex::sendmsg(string msg){
    this->ws->send(msg.c_str());
}


//string symbol, map<double,double>  &bidtable, map<double,double>  &asktable
void   okex::threadfunc_stream(){

    //uWS::Hub h;

    this->h.onError([](void *user) {
        switch ((long) user) {
        case 1:
            std::cout << "Client emitted error on invalid URI" << std::endl;
            break;
        case 2:
            std::cout << "Client emitted error on resolve failure" << std::endl;
            break;
        case 3:
            std::cout << "Client emitted error on connection timeout (non-SSL)" << std::endl;
            break;
        case 5:
            std::cout << "Client emitted error on connection timeout (SSL)" << std::endl;
            break;
        case 6:
            std::cout << "Client emitted error on HTTP response without upgrade (non-SSL)" << std::endl;
            break;
        case 7:
            std::cout << "Client emitted error on HTTP response without upgrade (SSL)" << std::endl;
            break;
        case 10:
            std::cout << "Client emitted error on poll error" << std::endl;
            break;
        case 11:
            static int protocolErrorCount = 0;
            protocolErrorCount++;
            std::cout << "Client emitted error on invalid protocol" << std::endl;
            if (protocolErrorCount > 1) {
                std::cout << "FAILURE:  " << protocolErrorCount << " errors emitted for one connection!" << std::endl;
                exit(-1);
            }
            break;
        default:
            std::cout << "FAILURE: " << user << " should not emit error!" << std::endl;
            exit(-1);
        }
    });

    this->h.onConnection([this](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) {
        //wsc = ws;
        this->ws = ws;
        this->wsconn_state = true;
        switch ((long) ws->getUserData()) {
        case 8:
            std::cout << "Client established a remote connection over non-SSL" << std::endl;
            ws->close(1000);
            break;
        case 9:
            std::cout << "Client established a remote connection over SSL" << std::endl;

            //ws->send("{\'event\':\'addChannel\',\'channel\':\'ok_sub_spot_eos_usdt_depth\'}");

            break;
        default:
            std::cout << "FAILURE: " << ws->getUserData() << " should not connect!" << std::endl;
            exit(-1);
        }
    });

    this->h.onDisconnection([](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
        std::cout << "Okex Client got disconnected with data: " << ws->getUserData() << ", code: " << code << ", message: <" << std::string(message, length) << ">" << std::endl;
    });


    this->h.onMessage([this](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {

        //cout << string(message, length) << endl;
        string msg = string(message, length);
        msg.pop_back();
        msg.erase(0,1);
        //cout << msg<< endl;
        if(msg.empty()){
            cout<<"empty msg..."<<endl;
        }
//https://github.com/Tencent/rapidjson/blob/master/example/tutorial/tutorial.cpp

        Document d;
        d.Parse(msg.c_str());
        if(d.HasMember("channel")){
            string symbol = d["channel"].GetString();
            if(d.HasMember("data")){
                const Value &data = d["data"];
                this->parse_priceamount_to_map(symbol,data);
            }
        }

    });



    //h.connect("invalid URI", (void *) 1);
    //h.connect("invalid://validButUnknown.yolo", (void *) 11);
    //h.connect("ws://validButUnknown.yolo", (void *) 2);
    //h.connect("ws://echo.websocket.org", (void *) 3, {}, 10);
    //h.connect("ws://echo.websocket.org", (void *) 8);
    //h.connect("wss://echo.websocket.org", (void *) 5, {}, 10);
    //h.connect("wss://echo.websocket.org", (void *) 9);
    //h.connect("ws://google.com", (void *) 6);
    this->h.connect(this->wssdomain,(void *) 9);
    //h.connect("wss://api.huobi.pro/ws",(void *) 9);
    //h.connect("wss://stream.binance.com:9443/stream?streams=eosusdt@depth.b10",(void *) 9);
    //h.connect("wss://api.zb.com:9999/websocket", (void *) 9);
    //h.connect("ws://127.0.0.1:6000", (void *) 10, {}, 60000);

    cout<<"okex websocket connection!"<<endl;
    this->h.run();
    cout << "Falling through testConnections" <<endl;

}

void   okex::parse_priceamount_to_map(string symbol,const Value &data){

    if(data.HasMember("asks")){
          const Value &asks = data["asks"];
          for(SizeType i = 0;i<asks.Size();i++){
               const Value &item = asks[i];
               //cout<<item[0].GetString() <<"    "<< item[1].GetString() <<endl;
               double price  = stod(item[0].GetString());
               double amount = stod(item[1].GetString());
               if(amount==0){
                    this->symbol_askbid_table[symbol].ask_table.erase(price);
               }else{
                    this->symbol_askbid_table[symbol].ask_table[price] = amount;
               }
          }
    }
    if(data.HasMember("bids")){
        const Value &bids = data["bids"];
        for(SizeType i = 0;i<bids.Size();i++){
            const Value &item = bids[i];
            //cout<<item[0].GetString() <<"    "<< item[1].GetString() <<endl;
            double price  = stod(item[0].GetString());
            double amount = stod(item[1].GetString());
            if(amount==0){
                this->symbol_askbid_table[symbol].bid_table.erase(price);
            }else{
                this->symbol_askbid_table[symbol].bid_table[price] = amount;
                //cout<<"price--"<<price<<"   amount---"<<amount<<endl;
            }
        }
    }
}
