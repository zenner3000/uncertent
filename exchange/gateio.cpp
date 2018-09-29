#include "gateio.h"


#include <map>
#include "../utils/codec.h"
#include "../utils/misc.h"
#include "../utils/httprequest.h"
#include <string>
#include <thread>
#include <iostream>
#include <mutex>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using  namespace std;
using namespace rapidjson;


gateio::gateio()
{
    wsconn_state = false;
    sub_state    = false;
}

gateio::~gateio()
{
    //dtor
}



/*h,out
  ws,out
*/
void   gateio::start_stream(){

    thread t(&gateio::threadfunc_stream,this);
    t.detach();

    unique_lock<mutex> lock(mu);
    cout<<"gateio wait the wss connect complete---"<<endl;
    cv.wait(lock, [this] {return wsconn_state;});
    cout<<"gateio wss connect is complete---"<<endl;
    //t.join();
    //return t.get_id();
}

void   gateio::closestream(){
    //this->h->close();
    this->ws->terminate();
}

/*ws,in
bidtable, out
asktable, out*/
void   gateio::subscribe_depth(string symbol){
    string subststr = R"xx({"id":99999, "method":"depth.subscribe", "params":[["BTC_USDT", 5, "0.01"]]})xx";

    //string subststr = "{\'event\':\'addChannel\',\'channel\':\'" + symbol + "\'}";
    struct askbidtable askbid_table;
    this->symbol_askbid_table[symbol] = askbid_table;

    this->sendmsg(subststr);

    unique_lock<mutex> lock(mu);
    cout<<"wait the sub complete---"<<endl;
    cv.wait(lock, [this] {return sub_state;});
    cout<<"sub is complete---"<<endl;

}

/*
    def get_sign(self,secret_key, message):
        h = hmac.new(secret_key.encode('utf-8'), message.encode('utf-8'), hashlib.sha512)
        return base64.b64encode(h.digest())

    def server_sign(self):
        nonce = int(time.time() * 1000)
        signature = self.get_sign(self.secret_key, str(nonce))
        #print(signature.decode('utf-8'))
        body = {
            "id": 12345,
            "method": "server.sign",
            "params": [self.api_key, signature.decode('utf-8'), nonce]
        }
        request = json.dumps(body)
        self.ws.send(request)
        #print(ws.recv())
*/

void gateio::server_sign(){
    string nonce = gettimestamp();
    string signature = sha512withkey_thenbase64(nonce,this->seckey);
    string signstr = "{\"id\":" + to_string(12345) +",\"method\":\"server.sign\",\"params\":[\""+this->apikey +"\",\""+ signature +"\","+nonce+"]}";
    cout<<signstr<<endl;
    this->sendmsg(signstr);
}

/*ws,in
subststr,in*/
void   gateio::cancel_subscribe_depth(string subststr){
    this->sendmsg(subststr);
}


void   gateio::sendmsg(string msg){
    this->ws->send(msg.c_str());
}


//string symbol, map<double,double>  &bidtable, map<double,double>  &asktable
void   gateio::threadfunc_stream(){

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
        std::cout << "Okex Client got disconnected with data: " << ws->getUserData() << ", code: " << code << ", message: <" << std::string(message, length) << ">" << std::endl;
    });


    h.onMessage([this](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
        //cout << string(message, length) << endl;
        string msg = string(message, length);
        cout << msg<< endl;
        if(msg.empty()){
            cout<<"empty msg..."<<endl;
        }
//https://github.com/Tencent/rapidjson/blob/master/example/tutorial/tutorial.cpp

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

    cout<<"gateio websocket connect......"<<endl;
    h.run();
    cout << "Falling through testConnections" <<endl;

}

void   gateio::parse_priceamount_to_map(string symbol,const Value &data){

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
