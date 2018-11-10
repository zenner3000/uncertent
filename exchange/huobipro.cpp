#include "huobipro.h"
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




using namespace std;



huobipro::huobipro()
{
    wsconn_state    = false;
    sub_state    = false;
}

huobipro::~huobipro()
{
    //dtor
}

string huobipro::get_balance(){
    string u = "/v1/account/accounts";
    map<string ,string> mparm;
    mparm["AccessKeyId"] = this->apikey;
    mparm["SignatureMethod"] = "HmacSHA256";
    mparm["SignatureVersion"] = "2";
    mparm["Timestamp"] = urlEncode(getutctime());

    string parmstr = parmmap_to_parmstr(mparm);

    string h = "GET\n";
    h.append("api.huobi.pro\n");
    h.append(u + "\n");
    h.append(parmstr);
    //cout<< "head--"<< h <<endl;

    string signedstr = sha256_then_base64(h,seckey);
    string url = this->restdomain +  u  + "?" + parmstr + "&" + "Signature=" + urlEncode(signedstr);
    string retstr = https_get(url,this->headerlist_get);
    return retstr;
}

/*symbol converto huobi symbol*/
string huobipro::create_limit_buy_order(string symbol, double amount, double price){
    string u = "/v1/order/orders/place";
    map<string ,string> mparm;
    mparm["AccessKeyId"]        = this->apikey;
    mparm["SignatureMethod"]    = "HmacSHA256";
    mparm["SignatureVersion"]   = "2";
    mparm["Timestamp"]          = urlEncode(getutctime());
    //mparm["Timestamp"]          = escapeURL(getutctime());


    string parmstr = parmmap_to_parmstr(mparm);

    string h = "POST\n";
    h.append("api.huobi.pro\n");
    h.append(u + "\n");
    h.append(parmstr);

    //cout<<h<<endl;
    string signedstr = sha256_then_base64(h,seckey);
    string url = this->restdomain +  u  + "?" + parmstr + "&" + "Signature=" + urlEncode(signedstr);
    //string retstr = https_get(url,this->httpheader);
    //string url = this->restdomain +  u;
    //string data = parmstr + "&" + "Signature=" + urlEncode(signedstr);

    string json = "{'account-id': '501137', 'amount': '1.58', 'symbol': 'xrpusdt', 'type': 'sell_limit', 'source': 'api', 'price': '0.6'}";
    string retstr = https_post(url,json,this->headerlist_post);
    return retstr;
}


/*symbol converto huobi symbol*/
string huobipro::create_limit_sell_order(string symbol, double amount, double price){
    string u = "/v1/order/orders/place";
    map<string ,string> mparm;
    mparm["AccessKeyId"]        = this->apikey;
    mparm["SignatureMethod"]    = "HmacSHA256";
    mparm["SignatureVersion"]   = "2";
    mparm["Timestamp"]          = urlEncode(getutctime());
    mparm["account-id"]         = "501137";
    mparm["amount"]             = to_string(amount);
    mparm["symbol"]             = symbol;
    mparm["type"]               = "sell-limit";
    mparm["price"]              = to_string(price);
    //parm add to mparm

    string parmstr = parmmap_to_parmstr(mparm);

    string h = "POST\n";
    h.append("api.huobi.pro\n");
    h.append(u + "\n");
    h.append(parmstr);

    cout<<"my need sign--"<<h<<endl;
    string signedstr = sha256_then_base64(h,seckey);
    string url = this->restdomain +  u  + "?" + parmstr +  "&Signature=" + urlEncode(signedstr);

    //must be double quote json
    string json = "{\"account-id\":501137,\"amount\":1.58,\"symbol\":\"xrpusdt\",\"type\":\"sell-limit\",\"price\":\"0.6000\"}";
    string retstr = https_post(url,json,this->headerlist_post);

    return retstr;
}

void   huobipro::threadfunc_stream()
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
            std::cout << "Client established a remote connection over non-SSL" << std::endl;
            //ws->close(1000);
            break;
        case 9:
            std::cout << "Client established a remote connection over SSL" << std::endl;
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
        std::cout << "huobipro Client got disconnected with data: " << ws->getUserData() << ", code: " << code << ", message: <" << std::string(message, length) << ">" << std::endl;
    });


    h.onMessage([this](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
        //cout << string(message, length) << endl;
        string msg = string(message, length);
        //cout << msg<< endl;
        if(msg.empty()){
            cout<<"empty msg..."<<endl;
        }
        string msgout;
        gzip_uncompress(msg,msgout);
        //cout<<"----"<<msgout<<endl;

        //ping handle
        Document d;
        d.Parse(msgout.c_str());
        if(d.HasMember("ping")){
            uint64_t pingvalue = d["ping"].GetUint64();
           // cout<<"ping--->"<<pingvalue<<endl;
            string msg = "{\"ping\":" + to_string(pingvalue) + "}";
            this->sendmsg(msg);
        }else if(d.HasMember("ch")){   //parse trade detail
            string symbol = d["ch"].GetString();
            if(d.HasMember("tick")){
                const Value &data = d["tick"]["data"];
                this->parse_tradedetail(symbol,data);
            }
        }else if(d.HasMember("status") && d.HasMember("subbed")){
            string status = d["status"].GetString();
            if(status.compare("ok")==0){
                this->sub_state = true;
                cv.notify_one();
            }
        }




     /*   if(msgout.substr(2,4).compare("ping")==0){
        //    cout<<msgout.substr(2,4)<<endl;

        }*/

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

    cout<<"huobipro websocket connect......"<<endl;
    h.run();
    cout << "Falling through testConnections" <<endl;
}

void   huobipro::start_stream()
{
    thread t(&huobipro::threadfunc_stream,this);
    t.detach();

    unique_lock<mutex> lock(mu);
    cout<<"huobipro wait the wss connect complete---"<<endl;
    cv.wait(lock, [this] {return wsconn_state;});
    cout<<"huobipro wss connect is complete---"<<endl;
}

void   huobipro::closestream()
{
    this->ws->terminate();
}

void   huobipro::subscribe_depth(string symbol)
{
    string subststr = "{\"sub\": \"market." + symbol+ ".depth.step5\", \"id\": \"id111111111\"}";
    struct askbidtable askbid_table;
    this->symbol_askbid_table[symbol] = askbid_table;

    this->sendmsg(subststr);

    unique_lock<mutex> lock(mu);
    cout<<"wait the sub depth complete---"<<endl;
    cv.wait(lock, [this] {return sub_state;});
    cout<<"sub depth is complete---"<<endl;
    this->sub_state = false;
}

void   huobipro::subscribe_trade_detail(string symbol)
{
    string subststr = "{\"sub\": \"market." + symbol+ ".trade.detail\", \"id\": \"id111111111\"}";
    struct askbidtable askbid_table;
    this->symbol_askbid_table[symbol] = askbid_table;

    this->sendmsg(subststr);
    unique_lock<mutex> lock(mu);
    cout<<"wait the trade detail sub complete---"<<endl;
    cv.wait(lock, [this] {return sub_state;});
    cout<<"sub  trade detail  is complete---"<<endl;
    this->sub_state = false;
}

void   huobipro::cancel_subscribe_depth(string symbol)
{
    this->sendmsg(symbol);
}

void   huobipro::sendmsg(string msg)
{
    this->ws->send(msg.c_str());
}

void   huobipro::parse_priceamount_to_map(string symbol,const Value &data)
{

}

void   huobipro::parse_tradedetail(string symbol,const Value &data)
{
      for(SizeType i = 0;i<data.Size();i++){
           const Value &item = data[i];

           string drt = item["direction"].GetString();
           if(drt.compare("sell")==0){
               double amount    = item["amount"].GetDouble();
               this->ask_amount += amount;
               double sum       = item["price"].GetDouble() * amount;
               this->ask_sum    += sum;
           }else{
               double amount    = item["amount"].GetDouble();
               this->bid_amount += amount;
               double sum       = item["price"].GetDouble() * amount;
               this->bid_sum    += sum;
           }

      }
}
