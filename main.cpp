// NOTE: This is not part of the library, this file holds examples and tests

#include "uWS.h"
#include <string>
#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <vector>
#include <atomic>
#include <iomanip>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "exchange/huobipro.h"
#include "exchange/binance.h"
#include "exchange/okex.h"
#include "exchange/exg58coin.h"
#include "exchange/zb.h"
#include "exchange/deribit.h"
#include "exchange/gateio.h"
#include "exchange/bitmex.h"
#include "exchange/poloniex.h"
#include "exchange/bitfinex.h"


#include "utils/httprequest.h"
#include "utils/misc.h"
#include "utils/trade.h"


#include "mail.h"

#include "SmtpMail.h"
#include "service/zmqservice.h"

using namespace rapidjson;
using namespace std;


void testWSS() {
    uWS::Hub h;

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


    h.onConnection([](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) {
    //wsc = ws;

        switch ((long) ws->getUserData()) {
        case 8:
            std::cout << "Client established a remote connection over non-SSL" << std::endl;
            //ws->close(1000);
            break;
        case 9:
            cout << "Client established a remote connection over SSL" << endl;
            //string subststr = "{\'event\':\'addChannel\',\'channel\':\'ok_sub_spot_ada_btc_depth\'}";
            //ws->send(subststr.c_str());
            ws->send("{\'event\':\'addChannel\',\'channel\':\'ok_sub_spot_eos_usdt_depth\'}");
            break;
        default:
            std::cout << "FAILURE: " << ws->getUserData() << " should not connect!" << std::endl;
            exit(-1);
        }
    });

    h.onMessage([](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
        //cout << string(message, length) << endl;
        string msg = string(message, length);
        msg.pop_back();
        msg.erase(0,1);
        cout<<msg<<endl;
    });

    h.onDisconnection([](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
        std::cout << "Client got disconnected with data: " << ws->getUserData() << ", code: " << code << ", message: <" << std::string(message, length) << ">" << std::endl;
    });

    string wssdomain  = "wss://real.okex.com:10441/websocket";
    h.connect(wssdomain.c_str(), (void *) 9);
 /*   h.connect("invalid URI", (void *) 1);
    h.connect("invalid://validButUnknown.yolo", (void *) 11);
    h.connect("ws://validButUnknown.yolo", (void *) 2);
    h.connect("ws://echo.websocket.org", (void *) 3, {}, 10);
    h.connect("ws://echo.websocket.org", (void *) 8);
    h.connect("wss://echo.websocket.org", (void *) 5, {}, 10);
    h.connect("wss://echo.websocket.org", (void *) 9);
    h.connect("ws://google.com", (void *) 6);
    h.connect("wss://google.com", (void *) 7);
    h.connect("ws://127.0.0.1:6000", (void *) 10, {}, 60000); */

    h.run();
    std::cout << "Falling through testConnections" << std::endl;
}


//
void sendmailtest()
{
    string smtpserver   = "smtp.qq.com";
    string username     = "595780836@qq.com";
    string password     = "bcvtbjbknlgsbdeb";
    string toaddr       = "11019537@qq.com";
    //sendmail();
    SmtpMail smtpmail = SmtpMail("utf-8");
    smtpmail.SetSmtpServer(username,password,smtpserver,"465");
    smtpmail.SetSendName("holyshit");
    smtpmail.SetSendMail(username);
    smtpmail.AddRecvMail(toaddr);
    smtpmail.SetSubject("Buy signal...Go!!!");
    smtpmail.SetBodyContent("just doe it~");
    smtpmail.SendMail();
}

void bitmextrade(){
    cout<<"--------BITMEX TRADE MONITOR---------"<<endl;
    int time=60*1;
    bitmex * bm = new bitmex();
    bm->init_table();
    bm->start_stream();
    //bm->subscribe_depth("xx");  //sub string not correct
    bm->subscribe_trade("xx");

    //this_thread::sleep_until(chrono::system_clock::now() + chrono::hours(numeric_limits<int>::max()));
    cout<<"sleep 5s.."<<endl;
    chrono::milliseconds dura(5000);
    this_thread::sleep_for(dura);

    zmq::socket_t socket (zmqservice::context, ZMQ_DEALER);
    cout<<"connect to inproc tunneltrade..."<<endl;
    socket.connect("inproc:///tmp/tunneltrade");

    while(true){
        chrono::milliseconds dura(1000*time);
        this_thread::sleep_for(dura);

        char data[200]={0};
        zmq::message_t msg(128);
        memset(msg.data(),0,128);


        cout<<"------TRADE------  "<<getCurrentSystemTime()<<"  ----------------"<<endl;
        cout<<"ask_amt: "<<bm->ask_amount<<"        bid_amt: "<<bm->bid_amount<<endl;
        cout<<"ask sum: "<<bm->ask_sum<<"   bid sum: "<<bm->bid_sum<<endl;
        if( bm->ask_sum==0 ){
            bm->ask_sum=1;
        }
        if( bm->bid_sum==0 ){
            bm->bid_sum=1;
        }
        double askbid = bm->ask_sum/bm->bid_sum;
        double bidask = bm->bid_sum/bm->ask_sum;
        cout<<"ask/bid:   "<<askbid<<endl;
        cout<<"bid/ask:   "<<bidask<<endl;

        int t = unix_timestamp();
        sprintf(data,"tradedata %f %f %f %f %.2f %.2f %d end",bm->ask_amount,bm->bid_amount,bm->ask_sum,bm->bid_sum,askbid,bidask,t);
        memcpy(msg.data(),data,strlen(data));

        socket.send(msg);


        bm->ask_amount=0;
        bm->ask_sum=0;
        bm->bid_amount=0;
        bm->bid_sum=0;

        if(askbid>3.0){
            cout<<"####### signal----->sell-----sell/buy ####### "<<askbid<<endl;
        }
        if(bidask>3.0){
            cout<<"####### signal----->buy------buy/sell ####### "<<bidask<<endl;
        }

    }
}

void bitmexbook(){
    cout<<"--------BITMEX BOOK MONITOR---------"<<endl;
    int time =6;

    bitmex * bm = new bitmex();
    bm->init_table();
    bm->start_stream();
    bm->subscribe_depth("xx");  //sub string not correct

    cout<<"wait 5s..."<<endl;
    chrono::milliseconds dura(5000);
    this_thread::sleep_for(dura);
    //bm->select_askamount("XBTUSDSell");


    zmq::socket_t socket (zmqservice::context, ZMQ_DEALER);
    cout<<"connect to inproc tunnelbook..."<<endl;
    socket.connect("inproc:///tmp/tunnelbook");

    while(true){
        chrono::milliseconds dura(1000*time);    //100ms is the best  for bitmex orderbook display . 3000ms is the best for sum amount
        this_thread::sleep_for(dura);

        char data[200]={0};
        zmq::message_t msg(128);
        memset(msg.data(),0,128);

       //bm->select_price("XBTUSDSell","asc",10);
       //bm->select_price("XBTUSDBuy","desc",10);
       //bm->select_minsellprice("XBTUSDSell");

        bm->select_askamount("XBTUSDSell");
        bm->select_bidamount("XBTUSDBuy");
        double a_b = bm->m_ask/bm->m_bid;
        double b_a = bm->m_bid/bm->m_ask;

        int t = unix_timestamp();
        cout<<"------BOOK----------- "<<getCurrentSystemTime()<<" ---------------"<<endl;
        //cout<<"ask book amt:"<<bm->m_ask<<endl;
        //cout<<"bid book amt:"<<bm->m_bid<<endl;
        cout<<"sell/buy----"<<a_b<<endl;
        cout<<"buy/sell----"<<b_a<<endl;

        sprintf(data,"bookdata %.1f %.1f %.2f %.2f %d end",bm->m_ask,bm->m_bid,a_b,b_a,t);  //must %.1f
        memcpy(msg.data(),data,strlen(data));
        socket.send(msg);


        if(a_b>6.0){
            cout<<"####### signal----->sell-----sell/buy ####### "<<a_b<<endl;
        }
        if(b_a>6.0){
            cout<<"####### signal----->buy------buy/sell ####### "<<b_a<<endl;
        }

    }
    //bm->selectprice("XBTUSDSell");
    //bm->selectprice("XBTUSDBuy");
}

int main(int argc, char *argv[])
{
    //serveEventSource();
    //serveHttp();
    //serveBenchmark();

#ifdef UWS_THREADSAFE
  //  testThreadSafety();
#endif
    //need thread start

    /*
    testWSS();
    thread t(testWSS);
    t.join(); */

    //zmqservice::start();
    //bitmexbook();
    //bitmextrade();

    bitfinex *bf = new bitfinex();
    bf->start_stream();
    bf->subscribe_depth("BTC_BTS");
    //bf->subscribe_trade("");

    this_thread::sleep_until(chrono::system_clock::now() + chrono::hours(numeric_limits<int>::max()));
    while(true){
        // cout<<"sleep 60 sec"<<endl;
        cout<<"-----60s-----"<<endl;
        chrono::milliseconds dura(1000*60);
        this_thread::sleep_for(dura);
        cout<<getCurrentSystemTime()<<endl;
        cout<<"ask amount    "<<"bid amount"<<endl;
        cout<<bf->ask_amount<<"         "<<bf->bid_amount<<endl;
        cout<<"ask sum       "<<"bid sum"<<endl;
        cout<<bf->ask_sum<<"        "<<bf->bid_sum<<endl;
        cout<<"ask/bid   "<<bf->ask_sum/bf->bid_sum<<endl;
        cout<<"bid/ask   "<<bf->bid_sum/bf->ask_sum<<endl;
        bf->ask_amount=0;
        bf->ask_sum=0;
        bf->bid_amount=0;
        bf->bid_sum=0;
    }
/*
    zmq::socket_t socket (zmqservice::context, ZMQ_DEALER);

    chrono::milliseconds dura(5*1000);
    this_thread::sleep_for(dura);

    cout<<"after 5s,connect to myinprctunnel..."<<endl;
    socket.connect("inproc:///tmp/xxxxx11111");

    cout<<"send msg..."<<endl;
    zmq::message_t msg(10);
    memcpy(msg.data(),"xxxxx",10);
    socket.send(msg);
    //zmq_send(msg);  */

    this_thread::sleep_until(chrono::system_clock::now() + chrono::hours(numeric_limits<int>::max()));
    /*
    string uri = genuri_byparm();
    string domain = "https://api.huobi.pro";
    string url = domain + "/v1/account/accounts?" + uri;
    string header = "Content-Type:application/x-www-form-urlencoded";

    https_get(url,header);
    */

/*
    huobipro *hb = new huobipro();
    //string hbstr = hb->get_balance();
    //cout <<hbstr<<endl;
    hb->start_stream();
    //hb->subscribe_depth("btcusdt");
    hb->subscribe_trade_detail("btcusdt");
//    this_thread::sleep_until(chrono::system_clock::now() + chrono::hours(numeric_limits<int>::max()));

    while(true){
        // cout<<"sleep 60 sec"<<endl;
        cout<<"-----60s-----"<<endl;
        chrono::milliseconds dura(300000);
        this_thread::sleep_for(dura);
        cout<<getCurrentSystemTime()<<endl;
        cout<<"ask amount    "<<"bid amount"<<endl;
        cout<<hb->ask_amount<<"         "<<hb->bid_amount<<endl;
        cout<<"ask sum       "<<"bid sum"<<endl;
        cout<<hb->ask_sum<<"        "<<hb->bid_sum<<endl;
        cout<<"ask/bid   "<<hb->ask_sum/hb->bid_sum<<endl;
        cout<<"bid/ask   "<<hb->bid_sum/hb->ask_sum<<endl;
        hb->ask_amount=0;
        hb->ask_sum=0;
        hb->bid_amount=0;
        hb->bid_sum=0;
    }  */



    /*

    bitmex * bm = new bitmex();
    bm->start_stream();
    bm->subscribe_depth("xx");  //sub string not correct

    string exg58smb = "btc_usdt";
    string type   = "SWAPS_ORDER"; //ORDER_BOOK
    exg58coin *exg58 =  new exg58coin();
    exg58->start_stream();
    exg58->subscribe_depth(exg58smb,type);

    string exg58smb = "btc_usdt";
    string type   = "SWAPS_MARKET";
    exg58coin *exg58 =  new exg58coin();
    exg58->start_stream();
    exg58->subscribe_depth(exg58smb,type);



    bitfinex *bf = new bitfinex();
    bf->start_stream();


    poloniex *pln = new poloniex();
    pln->start_stream();
    pln->subscribe_depth("xx");



    gateio *gate = new gateio();
    gate->start_stream();
    gate->server_sign();
    gate->subscribe_depth("xx");

    deribit *db = new deribit();
    db->start_stream();
    db->subscribe_depth("d");


    zb *exgzb =new zb();
    exgzb->start_stream();
    exgzb->subscribe_depth("btcusdt_depth");



    huobipro *hb = new huobipro();
    //string hbstr = hb->get_balance();
    //cout <<hbstr<<endl;
    hb->start_stream();
    hb->subscribe_depth("market.btcusdt.depth.step5");


  */

    //string hb    = hb.create_limit_buyorder();
    //string hbb   = hb.create_limit_sellorder();
    //huobipro *hb = new huobipro();
    //string result =  hb->get_balance();
    //string result =  hb->create_limit_sell_order("xrpusdt",1.58,0.6);
    //cout<<result<<endl;

    /*
    string bnsym = "xlmbtc";
    binance *bn = new binance();
    //string result = bn->create_limit_sell_order("ethusdt",0.17910,480);
    //cout<< result <<endl;
    //bn->start_stream();  //don't run this useless code
    bn->subscribe_depth(bnsym);   */
    //string result = bn->get_snapshot_depth("EOSUSDT",10);
    //bn->parse_snapshot_to_map(result,"eosusdt");
    //cout<<"sleep 10 sec"<<endl;
    //chrono::milliseconds dura(10000);

    //cout<<"subscribe btcusdt"<<endl;
    //bn->subscribe_depth("btcusdt");

  /*  string oksym = "ok_sub_spot_xrp_usdt_depth";
    okex                        *ok     = new okex();
    ok->start_stream();
    ok->subscribe_depth(oksym);  */
    //cout<<"all wss is ok ....."<<endl;
    //string result = ok->create_limit_sell_order("eos_usdt",2.1965,8.0);
    //cout <<result<<endl;

  /*  string oksym = "ok_sub_spot_xlm_btc_depth";
    okex                        *ok     = new okex();
    ok->start_stream();
    ok->subscribe_depth(oksym);  */

  /*   while(true){
       // cout<<"sleep 0.5 sec"<<endl;
        chrono::milliseconds dura(500);
        this_thread::sleep_for(dura);

     map<double,double>::reverse_iterator  rit;
        for(rit=bm->symbol_askbid_table["XBTUSD"].ask_table.rbegin();rit!=bm->symbol_askbid_table["XBTUSD"].ask_table.rend();rit++){
            cout<<"ask-----"<<rit->first<<"     "<<rit->second<<endl;
        }

        int cnt = 0;
        map<double,double>::reverse_iterator  rit;
        for(rit=exg58->symbol_askbid_table[exg58smb].bid_table.rbegin();rit!=exg58->symbol_askbid_table[exg58smb].bid_table.rend();rit++){
            cout<<"ask-----"<<rit->first<<"     "<<rit->second<<endl;
            if(cnt>10){
                break;
            }
            cnt++;
        }
    } */
    //this_thread::sleep_until(chrono::system_clock::now() + chrono::hours(numeric_limits<int>::max()));

 /*   while(true){
        cout<<"sleep 1 sec------------"<<endl;
        chrono::milliseconds dura(1000);
        this_thread::sleep_for(dura);

        map<double,double>::reverse_iterator  rit;
        for(rit=ok->symbol_askbid_table[oksym].ask_table.rbegin();rit!=ok->symbol_askbid_table[oksym].ask_table.rend();rit++){
            cout<<"ask-----"<<rit->first<<"     "<<rit->second<<endl;
        }

        int cnt = 0;
        map<double,double>::reverse_iterator  rit;
        for(rit=ok->symbol_askbid_table[oksym].bid_table.rbegin();rit!=ok->symbol_askbid_table[oksym].bid_table.rend();rit++){
            cout<<"bid-----"<<rit->first<<"     "<<rit->second<<endl;
            if(cnt>10){
                break;
            }
            cnt++;
        }
    }  */

    //this_thread::sleep_for(dura);
    cout<<"loop..."<<endl;
    //need to check two exchange wss state
    while(true){
            //cout<<"sleep 0.5 sec"<<endl;
            chrono::milliseconds dura(500);
            this_thread::sleep_for(dura);

     /*       double bnprice1    = bn->symbol_askbid_table[bnsym].ask_table.begin()->first;
            double bnquantity1 = bn->symbol_askbid_table[bnsym].ask_table.begin()->second;
            double okprice1    = ok->symbol_askbid_table[oksym].bid_table.rbegin()->first;
            double okquantity1 = ok->symbol_askbid_table[oksym].bid_table.rbegin()->second;

            double profit1     = profitcalc(bnprice1,bnquantity1,0.001,okprice1,okquantity1,0.002); */

            //need to put  into a iterator
    /*        double bnprice2    = bn->symbol_askbid_table[bnsym].bid_table.rbegin()->first;
            double bnquantity2 = bn->symbol_askbid_table[bnsym].bid_table.rbegin()->second;
            double okprice2    = ok->symbol_askbid_table[oksym].ask_table.begin()->first;
            double okquantity2 = ok->symbol_askbid_table[oksym].ask_table.begin()->second;

            double profit2     = profitcalc(okprice2,okquantity2,0.002,bnprice2,bnquantity2,0.001);   */


        /*    double tradeqtt =  (bnquantity>okquantity)?okquantity:bnquantity;

            string sy = "eosusdt";
            thread tbuy(buy,bn,sy,tradeqtt,bnprice);
            string sellby = "ok_sub_spot_eos_usdt_depth";
            thread tsell(sell,ok,sellby,tradeqtt,okprice);
            tbuy.join();
            tsell.join();
            cout<<"buy and sell is complete.."<<endl;  */
      //      this_thread::sleep_until(chrono::system_clock::now() + chrono::hours(numeric_limits<int>::max()));

    }

/*
     while(true){
            //cout<<"sleep 1 sec"<<endl;
            chrono::milliseconds dura(500);
            this_thread::sleep_for(dura);
            double bnprice    = bn->symbol_askbid_table["eosusdt"].bid_table.begin()->first;
            double bnquantity = bn->symbol_askbid_table["eosusdt"].bid_table.begin()->second;
            double okprice    = ok->symbol_askbid_table["ok_sub_spot_eos_usdt_depth"].ask_table.rbegin()->first;
            double okquantity = ok->symbol_askbid_table["ok_sub_spot_eos_usdt_depth"].ask_table.rbegin()->second;
            profitcalc(okprice,okquantity,0.002,bnprice,bnquantity,0.001);

    }*/

    //start the zeroMQ pub-sub server

    // json parse


}
