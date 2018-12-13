#ifndef ZMQSERVICE_H
#define ZMQSERVICE_H

#include <zmq.hpp>
#include <string>
using namespace std;

class zmqservice
{
    public:
        zmqservice();
        virtual ~zmqservice();

        static void threadfunc();
        static bool start();
        static void sendmsg_topy(zmq::message_t msg);
        static bool stop();
        static zmq::socket_t *psocket_inprc;
        static zmq::socket_t *psocket_ipc;
        static zmq::context_t context;
    protected:

    private:


};

#endif // ZMQSERVICE_H
