#include "exg58coin.h"

#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "../utils/misc.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace std;

exg58coin::exg58coin()
{
    //ctor
    wsconn_state = false;
    sub_state    = false;
}

exg58coin::~exg58coin()
{
    //dtor
}


void exg58coin::threadfunc_stream(){

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
        cout << "58coin Client got disconnected with data: " << ws->getUserData() << ", code: " << code << ", message: <" << string(message, length) << ">" << endl;
    });

    h.onMessage([this](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
        //cout << string(message, length) << endl;
        string msg = string(message, length);
        if(uWS::OpCode::TEXT == opCode){
            cout <<"text--"<<msg<< endl;
            Document d;
            d.Parse(msg.c_str());
            if(d.HasMember("event")){
                string event = d["event"].GetString();
                if(event.compare("SUB")==0){
                    string mesg = d["msg"].GetString();
                    if(mesg.compare("success")==0){
                        cout<<"subscribtion success..."<<endl;
                        this->sub_state = true;
                        cv.notify_one();
                    }
                }
            }
        }else if((uWS::OpCode::BINARY == opCode)){
            string uncmprmsg;
            //deflatetest(message,length,tmp);
            //inflate_uncompress(msg,tmp);
            inflate_uncompress(msg,uncmprmsg);
            //cout<<"uncmprmsg--->"<<uncmprmsg<<endl;

            Document d;
            d.Parse(uncmprmsg.c_str());
            if(d.HasMember("product") && d.HasMember("type")){
                string symbol = d["product"].GetString();
                if(d.HasMember("data")){
                    const Value &data = d["data"];
                    this->parse_priceamount_to_map(symbol,data);
                }
            }
        }

    });
    h.connect(this->wssdomain,(void *) 9);
    cout<<"58coin websocket connect......"<<endl;
    h.run();
    cout << "Falling through testConnections" <<endl;

}

void exg58coin::heartbeat(){
    string s = "{\"event\" : \"PING\"}";
    while(1){
        this->sendmsg(s);
        chrono::milliseconds dura(30000);
        this_thread::sleep_for(dura);
    }

}

void exg58coin::start_stream(){
    thread t(&exg58coin::threadfunc_stream,this);
    t.detach();

    unique_lock<mutex> lock(mu);
    cout<<"exg58coin wait the wss connect complete---"<<endl;
    cv.wait(lock, [this] {return wsconn_state;});
    cout<<"exg58coin  wss connect is complete---"<<endl;

    thread tt(&exg58coin::heartbeat,this);
    tt.detach();
}

/*
ORDER_BOOK , xian huo
SWAPS_ORDER , futures
*/
void   exg58coin::subscribe_depth(string symbol,string type){
    string subststr = "{\"event\" : \"SUB\", \"type\" : \"" + type + "\", \"product\" : \"" + symbol + "\"}";
    cout<<"subs---"<<subststr<<endl;
    struct askbidtable askbid_table;
    this->symbol_askbid_table[symbol] = askbid_table;

    this->sendmsg(subststr);

    unique_lock<mutex> lock(mu);
    cout<<"exg58coin wait the sub complete---"<<endl;
    cv.wait(lock, [this] {return sub_state;});
    cout<<"exg58coin  sub is complete---"<<endl;
}


void   exg58coin::sendmsg(string msg){
    this->ws->send(msg.c_str());
}


void   exg58coin::parse_priceamount_to_map(string symbol,const Value &data){

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
