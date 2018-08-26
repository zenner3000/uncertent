#include "huobipro.h"
#include "../utils/httprequest.h"
#include <map>
#include "../utils/codec.h"
#include "../utils/misc.h"
#include <iostream>


using namespace std;



huobipro::huobipro()
{
    //ctor
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


