#include "zmqservice.h"

#include <zmq.hpp>
#include <thread>
#include <chrono>
#include <iostream>
using namespace std;

zmqservice::zmqservice()
{
    //ctor
}

zmqservice::~zmqservice()
{
    //dtor
}

void   zmqservice::threadfunc(){
    //  Prepare our context and socket
    zmq::context_t context_ipc (1);
    zmq::socket_t socket_ipc (context_ipc, ZMQ_DEALER);
    zmqservice::psocket_ipc = &socket_ipc;
    //psocket_ipc->bind ("ipc:///tmp/myipctunneltrade");
    psocket_ipc->bind ("ipc:///tmp/myipctunnelbook");


    zmq::socket_t socket_inprc (zmqservice::context, ZMQ_DEALER);
    //socket_inprc.bind ("inproc:///tmp/tunneltrade");
    socket_inprc.bind ("inproc:///tmp/tunnelbook");

    while (true) {
        zmq::message_t msg;

        //  Wait for next request from client
        socket_inprc.recv (&msg);
        //std::cout << "Received : "<<msg << std::endl;
        psocket_ipc->send(msg);
    }
}



bool zmqservice::start()
{
    thread t(&zmqservice::threadfunc);
    t.detach();
    return true;
}

void zmqservice::sendmsg_topy(zmq::message_t msg)
{
    //zmq::message_t message(msg.length());
    //memcpy(message.data(),msg.c_str(),msg.length());
    zmqservice::psocket_ipc->send(msg);

    /*    zmq::message_t reply (5);
        memcpy (reply.data (), "World", 5);
        socket.send (reply); */
}


bool zmqservice::stop()
{
    return true;
}

zmq::socket_t *zmqservice::psocket_inprc=nullptr;
zmq::socket_t *zmqservice::psocket_ipc=nullptr;
zmq::context_t zmqservice::context = zmq::context_t(1);;
