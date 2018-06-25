#include "global.h"
#ifndef LOCK_485_H
#define LOCK_485_H





class lock_485
{
public:

    lock_485(int init_bps,const char * init_port_name);
    ~lock_485();
    //底层485 端口打开和维护

    //unsigned char device_add;

    //波特率
    unsigned char device_band_rate_hex;
    unsigned int  device_band_rate;

    //端口名字
    const char * port_name;

    //fd编号
    int fd;


    int serial_485_open(void);
    int serial_485_close(void);
    int serial_485_flush_read_buf(void);


    int serial_485_write_test(void);


    unsigned short crc16(unsigned char * dest, unsigned int wDataLen);


    //应包含整个动作过程，并包含重发机制和返回数据的485层面协议的解析，最终返回结果
    int rtu_write(unsigned char * head_p,        //待传送协议的字节头
                  int head_len,                  //头的长度，并非都是6字节,只是因为该项目的写都是写一个字节的数据，所以这里固定为6
                  unsigned char * responce_back, //有些协议的指定者，对于返回数据的数据段还进行填充，若这部分涉及上层协议，则需要返回。
                  int * responce_len,              //返回字节的长度
                  int back_responce_flag,        //是否需要等待响应
                  int time_out_ms);                  //响应是否需要等待超时


    //其实和rtu_write，一样，且反而简单，刨除了无需响应的分支
    int rtu_read(unsigned char * head_p,
                 int head_len,
                 unsigned char * responce_back,
                 int * responce_len,
                 int time_out_ms);

    //假设485发送命令如下
    //int send_msg_rtu(unsigned char * msg);

    //int recv_msg_8(unsigned char * responce_485,int time_out_ms);

    //int recv_msg_16(unsigned char * responce_485,int time_out_ms);


};

#endif // LOCK_485_H
