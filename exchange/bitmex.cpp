#include "bitmex.h"

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

#include "sqlite3.h"
#include <list>

using  namespace std;
using namespace rapidjson;


bitmex::bitmex()
{
    wsconn_state = false;
    sub_state    = false;
    m_dbname     = ":memory:";//"shit.db";
}

bitmex::~bitmex()
{
    //dtor
}



/*h,out
  ws,out
*/
void   bitmex::start_stream(){

    thread t(&bitmex::threadfunc_stream,this);
    t.detach();

    unique_lock<mutex> lock(mu);
    cout<<"bitmex wait the wss connect complete---"<<endl;
    cv.wait(lock, [this] {return wsconn_state;});
    cout<<"bitmex wss connect is complete---"<<endl;
    //t.join();
    //return t.get_id();
}

void   bitmex::closestream(){
    //this->h->close();
    this->ws->terminate();
}



/*ws,in
bidtable, out
asktable, out*/
void   bitmex::subscribe_depth(string symbol){
    string subststr = R"xx({"op": "subscribe", "args": ["orderBookL2:XBTUSD"]})xx";  //,"orderBookL2:ETHUSD"

    //string subststr = "{\'event\':\'addChannel\',\'channel\':\'" + symbol + "\'}";
    struct askbidtable askbid_table;
    this->symbol_askbid_table[symbol] = askbid_table;

    this->sendmsg(subststr);

    unique_lock<mutex> lock(mu);
    cout<<"wait the sub complete---"<<endl;
    cv.wait(lock, [this] {return sub_state;});
    cout<<"sub is complete---"<<endl;

}


void bitmex::server_sign(){
    string nonce = gettimestamp();
    string signature = sha512withkey_thenbase64(nonce,this->seckey);
    string signstr = "{\"id\":" + to_string(12345) +",\"method\":\"server.sign\",\"params\":[\""+this->apikey +"\",\""+ signature +"\","+nonce+"]}";
    cout<<signstr<<endl;
    this->sendmsg(signstr);
}

/*ws,in
subststr,in*/
void   bitmex::cancel_subscribe_depth(string subststr){
    this->sendmsg(subststr);
}


void   bitmex::sendmsg(string msg){
    this->ws->send(msg.c_str());
}


//string symbol, map<double,double>  &bidtable, map<double,double>  &asktable
void   bitmex::threadfunc_stream(){

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
        std::cout << "bitmex Client got disconnected with data: " << ws->getUserData() << ", code: " << code << ", message: <" << std::string(message, length) << ">" << std::endl;
    });


    h.onMessage([this](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
        //cout << string(message, length) << endl;
        string msg = string(message, length);
        //cout << msg<< endl;
        if(msg.empty()){
            cout<<"empty msg..."<<endl;
        }

        Document d;
        d.Parse(msg.c_str());
        if(d.HasMember("table") && d.HasMember("action")){
            string actionstr = d["action"].GetString();
             const Value &data = d["data"];
             string action = d["action"].GetString();
             this->parse_priceamount_to_map(action,data);
        }else if(d.HasMember("subscribe") && d.HasMember("success")){
            bool scs =d["success"].GetBool();
            if(scs){
                cout<<"subscribtion success..."<<endl;
                this->sub_state = true;
                cv.notify_one();
            }
        }

    });

    h.connect(this->wssdomain,(void *) 9);
    cout<<"bitmex websocket connect......"<<endl;
    h.run();
    cout << "Falling through testConnections" <<endl;

}

void   bitmex::parse_priceamount_to_map(string action, const Value &data)
{
    if(action.compare("update")==0){
         for(SizeType i = 0;i<data.Size();i++){
               const Value &item = data[i];
               //cout<<item[0].GetString() <<"    "<< item[1].GetString() <<endl;
               long         id      =  item["id"].GetUint64();
               string       symbol  =  item["symbol"].GetString();
               string       side    =  item["side"].GetString();
               double       amount  =  item["size"].GetDouble();

               this->updateprice(symbol+side,id,amount);
          }
    }else if(action.compare("insert")==0){
         for(SizeType i = 0;i<data.Size();i++){
               const Value &item = data[i];
               //cout<<item[0].GetString() <<"    "<< item[1].GetString() <<endl;
               long         id      =  item["id"].GetUint64();
               string       symbol  =  item["symbol"].GetString();
               string       side    =  item["side"].GetString();
               double       amount  =  item["size"].GetDouble();
               double       price   =  item["price"].GetDouble();

               this->insertprice(symbol+side,id,price,amount);
          }
    }else if(action.compare("delete")==0){
         for(SizeType i = 0;i<data.Size();i++){
               const Value &item = data[i];
               //cout<<item[0].GetString() <<"    "<< item[1].GetString() <<endl;
               long         id      =  item["id"].GetUint64();
               string       symbol  =  item["symbol"].GetString();
               string       side    =  item["side"].GetString();

               this->deleteprice(symbol+side,id);
          }
    }else if(action.compare("partial")==0){
         for(SizeType i = 0;i<data.Size();i++){
               const Value &item = data[i];
               //cout<<item[0].GetString() <<"    "<< item[1].GetString() <<endl;
               long         id      =  item["id"].GetUint64();
               string       symbol  =  item["symbol"].GetString();
               string       side    =  item["side"].GetString();
               double       amount  =  item["size"].GetDouble();
               double       price   =  item["price"].GetDouble();

               this->insertprice(symbol+side,id,price,amount);
          }
    }

}


static int callback(void *NotUsed,int cnt,char **column_value,char **column_name)
{
    return 0; // must return 0
}

static int callback_select(void *NotUsed,int cnt,char **column_value,char **column_name)
{
 /*   for(int i=0;i<cnt;i++){
        cout<<column_value[i] <<"---"<<column_name[i]<<endl;
    } */
    cout<<column_value[0]  <<"   " <<column_value[1] <<endl;
    return 0; // must return 0
}


bool bitmex::insertprice(string tablename,long msgid,double price,double amount)
{
    char *zErrMsg = 0;
    string statement = "insert into " + tablename + "(msgid,price,amount)VALUES(" + to_string(msgid) + "," + to_string(price) + "," +to_string(amount) + ")";
    int rc = sqlite3_exec(m_db,statement.c_str(),callback,0,&zErrMsg);
    if(rc!=SQLITE_OK){
        cout<<"not ok"<<endl;
     }
    return true;
}

bool bitmex::deleteprice(string tablename,long msgid)
{
    char *zErrMsg = 0;
    string statement = "delete from " + tablename + " where msgid="+to_string(msgid);
    int rc = sqlite3_exec(m_db,statement.c_str(),callback,0,&zErrMsg);
    if(rc!=SQLITE_OK){
        cout<<"not ok"<<endl;
     }
    return true;
}


bool bitmex::updateprice(string tablename,long msgid,int amount)
{
     char *zErrMsg = 0;
     string statement = "update " + tablename + " set amount="+to_string(amount) + " where msgid="+to_string(msgid);
     int rc = sqlite3_exec(m_db,statement.c_str(),callback,0,&zErrMsg);
     if(rc!=SQLITE_OK){
        cout<<"not ok"<<endl;
     }
     return true;
}


bool bitmex::selectprice(string tablename)
{
     char *zErrMsg = 0;
     string statement = "select /*id,msgid,*/price,amount from " + tablename + " order by price asc limit 10";//
     sqlite3_exec(m_db,statement.c_str(),callback_select,0,&zErrMsg);
     return true;
}

void bitmex::clear_table_data(string tablename)
{
    char *zErrMsg = 0;
    string statement = "delete from " + tablename;
    sqlite3_exec(m_db,statement.c_str(),callback,0,&zErrMsg);
}


bool bitmex::init_table()
{
     char *zErrMsg = 0;
     if(sqlite3_open(m_dbname.c_str(),&m_db)){   //change to memory mode
         sqlite3_close(m_db);
         return false;
     }

    list<string> tables;
    tables.push_back("XBTUSDSell");
    tables.push_back("XBTUSDBuy");
    tables.push_back("ETHUSDSell");
    tables.push_back("ETHUSDBuy");
    list<string>::iterator it;
    for(it=tables.begin();it!=tables.end();it++){
        //cout<<*it<<endl;
        string statement = "create table " + *it + " (id INTEGER PRIMARY KEY AUTOINCREMENT, msgid INTEGER NOT NULL, price REAL, amount REAL)";
        int rc = sqlite3_exec(m_db,statement.c_str(),callback,0,&zErrMsg);
    }
    return true;
}
