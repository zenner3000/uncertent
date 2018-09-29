#include "deribit.h"

#include "../utils/httprequest.h"
#include <map>
#include "../utils/codec.h"
#include "../utils/misc.h"
#include <iostream>
#include <thread>
#include <mutex>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

deribit::deribit()
{
    //ctor
}

deribit::~deribit()
{
    //dtor
}



void   deribit::threadfunc_stream()
{

    //uWS::Hub h;
    uWS::Hub h; // can not be pointer

    h.onError([](void *user) {
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

    h.onConnection([this](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) {
        //wsc = ws;

        switch ((long) ws->getUserData()) {
        case 8:
            std::cout << "deribit Client established a remote connection over non-SSL" << std::endl;
            //ws->close(1000);
            break;
        case 9:
            std::cout << "deribit Client established a remote connection over SSL" << std::endl;
            this->ws = ws;
            this->wsconn_state = true;
            cv.notify_one();
            //ws->send("{\'event\':\'addChannel\',\'channel\':\'ok_sub_spot_eos_usdt_depth\'}");

            break;
        default:
            std::cout << "FAILURE: " << ws->getUserData() << " should not connect!" << std::endl;
            exit(-1);
        }
    });

    h.onDisconnection([](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
        std::cout << "deribit Client got disconnected with data: " << ws->getUserData() << ", code: " << code << ", message: <" << std::string(message, length) << ">" << std::endl;
    });


    h.onMessage([this](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
        //cout << string(message, length) << endl;
        string msg = string(message, length);
        cout << msg<< endl;
        if(msg.empty()){
            cout<<"empty msg..."<<endl;
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
    h.connect(this->wssdomain,(void *) 9);
    //h.connect("wss://api.huobi.pro/ws",(void *) 9);
    //h.connect("wss://stream.binance.com:9443/stream?streams=eosusdt@depth.b10",(void *) 9);
    //h.connect("wss://api.zb.com:9999/websocket", (void *) 9);
    //h.connect("ws://127.0.0.1:6000", (void *) 10, {}, 60000);

    cout<<"deribit websocket connect......"<<endl;
    h.run();
    cout << "Falling through testConnections" <<endl;
}

void   deribit::start_stream()
{
    thread t(&deribit::threadfunc_stream,this);
    t.detach();

    unique_lock<mutex> lock(mu);
    cout<<"deribderibit wait the wss connect complete---"<<endl;
    cv.wait(lock, [this] {return wsconn_state;});
    cout<<"deribit wss connect is complete---"<<endl;
}

void   deribit::closestream()
{
    this->ws->terminate();
}

string deribit::makereqstr(int id,string action,string instrument,string event,string sig){
    string r = "";
    r = r + "{\"id\":"  + to_string(id) + ",\"action\":\""+ action +"\",\"arguments\":{\"instrument\":[\""+instrument+"\"],\"event\":[\""+ event +"\"]},\"sig\":\""+ sig +"\"}";
    cout<<r<<endl;
    return r;
    //"API-KEY.XXX.base64str"
}

void   deribit::subscribe_depth(string symbol)
{
    map<string,string> msignstr;
    string tsp             = gettimestamp();
    msignstr["_"]          = tsp;
    msignstr["_ackey"]     = this->apikey;
    msignstr["_acsec"]     = this->seckey;
    msignstr["_action"]     = "/api/v1/private/subscribe";
    msignstr["instrument"] = "futures";
    msignstr["event"]      = "order_book";
    string signstr = parmmap_to_parmstr(msignstr);
    cout<<"signstr---"<<signstr<<endl;
    string hashstr = sha256hashbyte_then_base64(signstr);
    string sig = this->apikey + "." + tsp + "." + hashstr;
    cout<<sig<<endl;
    string reqstr = makereqstr(20111,"/api/v1/private/subscribe","futures", "order_book",sig);

/*
    string subststr = "{'event':'addChannel','channel':'" + symbol + "',}";
    struct askbidtable askbid_table;
    this->symbol_askbid_table[symbol] = askbid_table;  */
    //string t = "{\"action\": \"/api/v1/public/ping\"}";
    //this->sendmsg(t);
    this->sendmsg(reqstr);

    unique_lock<mutex> lock(mu);
    cout<<"deribit wait the sub complete---"<<endl;
    cv.wait(lock, [this] {return sub_state;});
    cout<<"deribit sub is complete---"<<endl;
}

void   deribit::cancel_subscribe_depth(string symbol)
{
    this->sendmsg(symbol);
}

void   deribit::sendmsg(string msg)
{
    this->ws->send(msg.c_str());
}

void   deribit::parse_priceamount_to_map(string symbol,const Value &data)
{

}
