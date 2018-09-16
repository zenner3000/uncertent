#include "trade.h"

#include <thread>
#include <iostream>
#include "exchange.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iomanip>

using namespace rapidjson;
using namespace std;

void buy(exchange *exg, string symbol,double buyquantity, double buyprice){
    string result = exg->create_limit_buy_order(symbol,buyquantity,buyprice);
    cout<<"buy---"<<result<<endl;
    //print the result
}

void sell(exchange *exg, string symbol,double sellquantity, double sellprice){
    string result = exg->create_limit_sell_order(symbol,sellquantity,sellprice);
    cout<<"sell---"<<result<<endl;
    //print the result
}

//need to know symbol
void buy_sell(exchange *exgbuy, string symbol_buy, exchange *exgsell, string symbol_sell, double quantity,double buyprice, double sellprice){

    thread tbuy(buy,exgbuy,symbol_buy,quantity,buyprice);
    thread tsell(sell,exgsell,symbol_sell,quantity,sellprice);
    tbuy.join();
    tsell.join();
}



double profitcalc(double sellprice, double sellquantity, double sell_feerate, double buyprice, double buyquantity, double buy_feerate){
    //cout<< fixed << setprecision(10) << sellprice << endl;
    //cout<< fixed << setprecision(10) << buyprice << endl;
    double totalprofit;
    double tradeqtt =  (sellquantity>buyquantity)?buyquantity:sellquantity;   //should trade quantity
    double fee_sellpercoin  =  sellprice * sell_feerate;
    double fee_buypercoin   =  buyprice  * buy_feerate;
    double profit_percoin = (buyprice - fee_buypercoin) - (sellprice - fee_sellpercoin);

   if(profit_percoin>0){
        totalprofit =  profit_percoin * tradeqtt;

        cout<<fixed<<setprecision(13)<<"sell price--"<<sellprice<<"  quantity--"<<sellquantity<<"   fee--"<<sell_feerate<<"     fee_sell--"<<fee_sellpercoin<<endl;
        cout<<fixed<<setprecision(13)<<"buy price--"<<buyprice<<"  quantity--"<<buyquantity<<"   fee--"<<buy_feerate<<"     fee_buy--"<<fee_buypercoin<<endl;
        cout<<fixed<<setprecision(13)<<"each coin profit--"<<profit_percoin<<endl;
        cout<<fixed<<setprecision(13)<<"totalprofit--"<<totalprofit<<"   tradeqtt--"<<tradeqtt<<endl;
        cout<<"--------"<<endl;

        return profit_percoin;
    }
    return 0;
}


