#include "global.h"

#ifndef NET_MARCH_H
#define NET_MARCH_H



class net_march
{
public:
    net_march(const char * init_ip,const char * init_port);

    const char * ip_addr;
    const char * ip_port;


    int fd_net;

    //全句在数据发送时，若为自主发送的，就需要增加1
    //如：心跳包，事件包
    unsigned int pack_count;

    //网络的状态标志，所有内嵌该网络对象的类都按照这个状体来决定是否使用
    //该标志是对各个装饰对象而言的，不对应具体的tcp链接的意义。
    int net_state;

    int ack_fail_count;

    //待填充的返回、发往server的数据包
    struct PACK_MSG pack_msg;

    //创建链接
    int make_connect(void);

    //修复节点，内部为周期net_march::make_connect()
    int repair_connect(void);

    //tcp返回响应信息，且是无响应式发送，发完不管
    int send_tcp_responce(struct CMD_MSG * p_data);

    //tcp主动发出异常信息，并等待响应后确认，未响应则重发3次
    int send_unusual_msg(struct CMD_MSG * p_data,int timeout_ms,struct CMD_MSG * p_recv);

    //发送心跳包，包编号也要自增，但不需要等待响应
    //int send_tcp_heart_beat(struct CMD_MSG * p_data);
    int send_beat_msg(struct CMD_MSG * p_data,int timeout_ms,struct CMD_MSG * p_recv);

    //crc
    unsigned short crc16(unsigned char * dest, unsigned int wDataLen);
    //void InvertUint8(unsigned char *dBuf, unsigned char *srcBuf);
    //void InvertUint16(unsigned char *dBuf, unsigned char *srcBuf);


    int cal_time_duration( struct timeval * fresh_time,struct timeval * old_time);
};

#endif // NET_MARCH_H
