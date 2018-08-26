#include "binance.h"
#include <string>
#include <map>
#include <iostream>
#include <thread>
#include "../utils/misc.h"
#include "../utils/codec.h"
#include "../utils/httprequest.h"
#include "common.h"

#include "uWS.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"



using namespace rapidjson;
using namespace std;

binance::binance()
{
    //ctor
    this->wsconn_state=false;
}

binance::~binance()
{
    //dtor
}


string binance::create_limit_buy_order(string symbol, double amount, double price){
    string u = "/api/v3/order";
    map<string ,string> mparm;
    mparm["symbol"]         = "ETHUSDT";
    mparm["side"]           = "BUY";
    mparm["type"]           = "LIMIT";
    mparm["quantity"]       = to_string(amount);
    mparm["price"]          = to_string(price);
    mparm["timestamp"]      = gettimestamp();
    mparm["newOrderRespType"] = "RESULT";
    mparm["recvWindow"]     = "5000";
    mparm["timeInForce"]    = "GTC";

    string parmstr = parmmap_to_parmstr(mparm);
    //cout << parmstr<<endl;
    string signedstr = sha256_then_str(parmstr,seckey);
    string data   = parmstr + "&" + "signature=" + signedstr;
    string retstr = https_post(this->restdomain + u,data,this->headerlist);
    return retstr;
}

string binance::create_limit_sell_order(string symbol, double amount, double price){
    string u = "/api/v3/order";
    map<string ,string> mparm;
    mparm["symbol"]         = "ETHUSDT";
    mparm["side"]           = "SELL";
    mparm["type"]           = "LIMIT";
    mparm["quantity"]       = to_string(amount);
    mparm["price"]          = to_string(price);
    mparm["timestamp"]      = gettimestamp();
    mparm["newOrderRespType"] = "RESULT";
    mparm["recvWindow"]     = "5000";
    mparm["timeInForce"]    = "GTC";

    string parmstr = parmmap_to_parmstr(mparm);
    //cout << parmstr<<endl;
    string signedstr = sha256_then_str(parmstr,seckey);
    string data   = parmstr + "&" + "signature=" + signedstr;
    string retstr = https_post(this->restdomain + u,data,this->headerlist);
    return retstr;
}


string binance::test(string symbol, double amount, double price){
    string u = "/api/v3/order/test";
    map<string ,string> mparm;
    mparm["symbol"]         = "ETHUSDT";
    mparm["side"]           = "SELL";
    mparm["type"]           = "LIMIT";
    mparm["quantity"]       = to_string(amount);
    mparm["price"]          = to_string(price);
    mparm["timestamp"]      = gettimestamp();


    string parmstr = parmmap_to_parmstr(mparm);

    //string signedstr = sha256_then_base64(h,seckey);
    return "";
}


string binance::cancel_order(){
    return "";
}

string binance::get_balance(){
    return "";
}

void   binance::threadfunc_stream(){

    //uWS::Hub h;
    uWS::Hub h; // can not be pointer
    //uWS::WebSocket<uWS::CLIENT> *ws;

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
            cout << "FAILURE: " << user << " should not emit error!" << endl;
            exit(-1);
        }
    });

    h.onConnection([this](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) {
        this->wsconn_state = true;
        this->ws = ws;
        switch ((long) ws->getUserData()) {
        case 8:
            cout << "Client established a remote connection over non-SSL" << endl;
            ws->close(1000);
            break;
        case 9:
            cout << "Client established a remote connection over SSL" << endl;
            //okex
            //ws->send("{\'event\':\'addChannel\',\'channel\':\'ok_sub_spot_eos_usdt_depth\'}");

            //huobi
            //ws->send("{\"sub\": \"market.eosusdt.depth.step0\", \"id\": \"333888\"}");

            //zb
            //ws->send("{	\'event\':\'addChannel\',	\'channel\':\'ltcbtc_depth\',}");

            //ws->close(1000);
            break;
        default:
            cout << "FAILURE: " << ws->getUserData() << " should not connect!" << endl;
            exit(-1);
        }
    });

    h.onDisconnection([this](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
        this->wsconn_state = false;
        cout << "Binance Client got disconnected with data: " << ws->getUserData() << ", code: " << code << ", message: <" << string(message, length) << ">" << endl;
    });


    h.onMessage([this](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
        //std::cout << std::string(message, length) << std::endl;
        //ws->send(message, length, opCode);
        //string eosusdt = "eosusdt";
        Document d;
        d.Parse(string(message, length).c_str());    //must use the lenght, or parse failed

        if(d.IsObject()){
           if(d.HasMember("data")){
                //cout <<"-----" <<string(message, length) << endl;
                //cout<<d["stream"].GetString()<<endl;
                string symbolflag = d["stream"].GetString();
               	int index = symbolflag.find_first_of('@');
                string symbol = symbolflag.substr(0, index);
                //cout<<"---symbol---"<<symbol<<endl;
                const Value &data = d["data"];
                parse_priceamount_to_map(symbol,data);

           }
        }else{
            cout<<"not a object"<<endl;
            cout <<"-----" <<string(message, length) << endl;
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
    //h.connect("wss://real.okex.com:10441/websocket",(void *) 9);
    //h.connect("wss://api.huobi.pro/ws",(void *) 9);

    if( !(this->symbol_askbid_table.empty()) ){
        //re-connect to the server
        string smbolstr = get_symbol_uri();
        string addr = this->wssdomain + "/stream?streams=" +  smbolstr;
        cout<<"wss---"<<addr<<endl;
        h.connect(addr.c_str(),(void *) 9);
        this->wsconn_state = true;
        //h.connect("wss://api.zb.com:9999/websocket", (void *) 9);
        //h.connect("ws://127.0.0.1:6000", (void *) 10, {}, 60000);

        cout<<"binance wss start!"<<endl;
        h.run();


        cout << "Falling through testConnections" << endl;
    }

}

void binance::start_stream(){
    this->t = thread(&binance::threadfunc_stream,this);
    t.detach();

    std::thread::id tid = t.get_id();
    cout<<tid<<endl;
    //t.join();
}

void binance::closestream(){
    //this->ws->close();
}

void binance::subscribe_depth(string symbol){
    struct askbidtable askbid_table;
    this->symbol_askbid_table[symbol] = askbid_table;

    //close then connect
    if(this->wsconn_state)
        this->ws->terminate();

    //if thread exist
    //wait until the thread exit ,within 10 sec
    //re-connect
    string depth = get_snapshot_depth(to_uppercase(symbol),10);
    parse_snapshot_to_map(depth,symbol);
    this->start_stream();
    //
}

//void   subscribe_mutil_depth();
void binance::cancel_subscribe_depth(string symbol){
    //send msg to thread to re-conect
    map<string,askbidtable>::iterator it =  this->symbol_askbid_table.find(symbol);
    if( it!= this->symbol_askbid_table.end()){
        this->symbol_askbid_table.erase(it);
    }

    this->depth_info.erase(symbol);

    //close then connect
    if(this->wsconn_state)
        this->ws->terminate();

    //re-connect
    if( !(this->symbol_askbid_table.empty()) )
        this->start_stream();
}


void binance::subscribe_mutil_depth(){

}

string binance::get_symbol_uri(){
    string retstr;

    try{
        if( !(this->symbol_askbid_table.empty()) ){
            cout<<"not empty symbol_askbid_table"<<endl;
            map<string,askbidtable>::iterator it;
            for(it=this->symbol_askbid_table.begin();it!=this->symbol_askbid_table.end();it++){
                retstr.append(it->first + "@depth/");
            }
        }else{
            cout<<"empty symbol_askbid_table"<<endl;
        }
    }
    catch(exception exp){

    }


    return retstr;
}

/*
symbol , Must be upper case
*/
string   binance::get_snapshot_depth(string symbol,int limit){
    string url    = this->uri_snapshot_depth + symbol + "&limit=" + to_string(limit);
    string retstr = https_get(url,this->headerlist);
    return retstr;
}
/*
smb, must be lower case
*/
void binance::parse_snapshot_to_map(string json,string smb){
        Document d;
        d.Parse(json.c_str());    //must use the lenght, or parse failed

        if(d.IsObject()){


                depthinfo di;
                di.lastUpdateId = d["lastUpdateId"].GetInt();
                this->depth_info[smb] = di;

                const Value &bids = d["bids"];
                for(SizeType i = 0;i<bids.Size();i++){
                   const Value &item = bids[i];
                   //cout<<"bids--"<<item[0].GetString() <<"    "<< item[1].GetString() <<endl;
                   double price  = stod(item[0].GetString());
                   double amount = stod(item[1].GetString());
                   this->symbol_askbid_table[smb].bid_table[price] = amount;

                }

                const Value &asks = d["asks"];
                for(SizeType i = 0;i<asks.Size();i++){
                    const Value &item = asks[i];
                    //cout<<"asks--"<<item[0].GetString() <<"    "<< item[1].GetString() <<endl;
                    double price  = stod(item[0].GetString());
                    double amount = stod(item[1].GetString());
                    this->symbol_askbid_table[smb].ask_table[price] = amount;

                }


        }else{
            cout<<"not a object"<<endl;
        }
}


void    binance::parse_priceamount_to_map(string symbol,const Value &data)
{
        int u = data["u"].GetInt();

        if(u < this->depth_info[symbol].lastUpdateId){
            cout<<"u < lastUpdateId-----drop..."<<endl;
            return;
        }
        int U = data["U"].GetInt();
        //cout<<"U---"<<U<<"   u---"<<u<<endl;
        if(U <= this->depth_info[symbol].lastUpdateId+1 && u >= this->depth_info[symbol].lastUpdateId+1){
            cout<<"binggo--------"<<endl;
            //
        }

        this->depth_info[symbol].prev_u = u;
        //cout<<"prev_u---"<<prev_u<<endl;
       /* if(this->depth_info.contains(symbol)){

        } */

        //cout<<"U---"<<U<<endl;
        if(data.HasMember("b")){
              const Value &biddata = data["b"];
              for(SizeType i = 0;i<biddata.Size();i++){
                   const Value &item = biddata[i];
                   double price  = stod(item[0].GetString());
                   double amount = stod(item[1].GetString());
                   //cout<<"bid---"<<price<<"    "<<amount<<endl;

                   //If the quantity is 0, remove the price level
                   if(amount==0){
                        this->symbol_askbid_table[symbol].bid_table.erase(price);
                   }else{
                        this->symbol_askbid_table[symbol].bid_table[price] = amount;
                   }

              }
        }
        if(data.HasMember("a")){
              const Value &askdata = data["a"];
              for(SizeType i = 0;i<askdata.Size();i++){
                   const Value &item = askdata[i];
                   double price  = stod(item[0].GetString());
                   double amount = stod(item[1].GetString());
                   //cout<<"ask---"<<price<<"    "<<amount<<endl;

                   //If the quantity is 0, remove the price level
                    if(amount==0){
                        this->symbol_askbid_table[symbol].ask_table.erase(price);
                    }else{
                        this->symbol_askbid_table[symbol].ask_table[price] = amount;
                    }

              }
        }

        //send a msg(max ,min) to another waiting thread

        //test
      /*  map<double,double>::iterator  it;
        for(it=this->symbol_askbid_table[symbol].bid_table.begin();it!=this->symbol_askbid_table[symbol].bid_table.end();it++){
            cout<<"bid-----"<<it->first<<"     "<<it->second<<endl;
        } */

}
