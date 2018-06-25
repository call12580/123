#include "global.h"
#include "net_march.h"

#ifndef ROUTE_DIS_H
#define ROUTE_DIS_H



class route_dis
{
public:
    route_dis(net_march * init_n_m,unsigned char * init_node_p);
    ~route_dis();

    net_march * n_m;

    unsigned char node_index[7];

    //接收数据并转发
    int loop_recv_msg();

};

#endif // ROUTE_DIS_H
