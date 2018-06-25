#include "net_march.h"



net_march::net_march(const char * init_ip,const char * init_port):ip_addr(init_ip),ip_port(init_port)
{
    this->pack_count = 0;
    this->fd_net = -1;
    this->net_state = NET_OFFLINE;

    printf("%s  %s\r\n",ip_addr, ip_port);
}




int net_march :: make_connect(void)
{
    int ret;
    struct linger so_linger;
    int optval;
    socklen_t optlen = sizeof(optval);
    //创建套接字,即创建socket
    int clt_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(clt_sock < 0)
    {
      cout << "socket error" << endl;
      return -1;
    }

    //绑定信息，即命名socket
    struct sockaddr_in addr;

    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(atoi(ip_port));
    addr.sin_addr.s_addr=inet_addr(ip_addr);
    //addr.sin_port=htons(atoi(SERVER_PORT));
    //addr.sin_addr.s_addr=inet_addr(SERVER_ADDR);

    so_linger.l_linger = 0;
    so_linger.l_onoff = 1;
    setsockopt(clt_sock, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));

    /*
    optval = 1;//打开keepalive
    setsockopt(clt_sock, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);

    optval = 3;// 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
    setsockopt(clt_sock, SOL_TCP, TCP_KEEPCNT, &optval, optlen);

    optval = 10;//如该连接在10秒内没有任何数据往来,则进行探测
    setsockopt(clt_sock, SOL_TCP, TCP_KEEPIDLE, &optval, optlen);

    optval = 1;// 探测时发包的时间间隔为3秒
    setsockopt(clt_sock, SOL_TCP, TCP_KEEPINTVL, &optval, optlen);
    */


    socklen_t addr_len = sizeof(addr);
    ret = connect(clt_sock, (struct sockaddr*)&addr, addr_len);
    if(ret < 0)
    {
        cout << "connect error" << endl;
        return -2;
    }

    cout << "connect success" << endl;
    this->fd_net = clt_sock;
    this->net_state = NET_READY;
    this->ack_fail_count = 0;

    return 0;
}


int net_march :: repair_connect(void)
{
    int ret;

    close(this->fd_net);

    while(1)
    {
        ret = this->make_connect();
        if(0 ==  ret ) break;
        usleep(5000*1000);
    }


    return 0;
}


int net_march :: send_unusual_msg(struct CMD_MSG * p_data,int timeout_ms,struct CMD_MSG * p_recv)
{
    int ret;
    int i;
    struct timeval timeout_start;
    struct timeval timenow;
    int time_fly;


    unsigned short crc_temp;


    this->pack_msg.frame_head[0] = 0x7E;
    this->pack_msg.frame_head[1] = 0x7E;
    memcpy(&this->pack_msg.frame_msg, p_data,sizeof(struct CMD_MSG));
    this->pack_msg.frame_end[0] = 0xFC;
    this->pack_msg.frame_end[1] = 0xFC;


    __sync_fetch_and_add (&this->pack_count,1);
    this->pack_msg.frame_msg.msg_num[0] = ((unsigned char *)(&this->pack_count))[3];
    this->pack_msg.frame_msg.msg_num[1] = ((unsigned char *)(&this->pack_count))[2];
    this->pack_msg.frame_msg.msg_num[2] = ((unsigned char *)(&this->pack_count))[1];
    this->pack_msg.frame_msg.msg_num[3] = ((unsigned char *)(&this->pack_count))[0];



    crc_temp = this->crc16((unsigned char *)(&this->pack_msg),sizeof(struct CMD_MSG)+4);

    this->pack_msg.crc[0] = ((unsigned char *)(&crc_temp))[1];
    this->pack_msg.crc[1] = ((unsigned char *)(&crc_temp))[0];


    if(NET_READY ==  this->net_state)
    {
        for(i = 0;i<RETRY_TCP_COUNT;i++)
        {
            //printf("before send\r\n");
            //printf("p_data  %x, %x, %x, %x\r\n",p_data->msg_num[0],p_data->msg_num[1],p_data->msg_num[2],p_data->msg_num[3]);
            ret = send(this->fd_net,&this->pack_msg,sizeof(struct PACK_MSG),0);

            if(ret == sizeof(struct PACK_MSG))
            {
                //记录启动时间
                gettimeofday(&timeout_start,NULL);

                //手动超时等待
                while(1)
                {
                    //printf("123\r\n");

                    usleep(50*1000);

                    //printf("p_data  %x, %x, %x, %x\r\n",p_data->msg_num[0],p_data->msg_num[1],p_data->msg_num[2],p_data->msg_num[3]);
                    //printf("p_recv  %x, %x, %x, %x\r\n",p_recv->msg_num[0],p_recv->msg_num[1],p_recv->msg_num[2],p_recv->msg_num[3]);
                    //只在主动发送数据包中需要等待返回响应，并根据包编号判断是否是针对的响应
                    if((this->pack_msg.frame_msg.msg_num[0] == p_recv->msg_num[0])
                     &&(this->pack_msg.frame_msg.msg_num[1] == p_recv->msg_num[1])
                     &&(this->pack_msg.frame_msg.msg_num[2] == p_recv->msg_num[2])
                     &&(this->pack_msg.frame_msg.msg_num[3] == p_recv->msg_num[3]))
                    {
                        //接受到返回响应
                        cout << "ack unusual msg responce ok " << endl;
                        //this->ack_fail_count = 0;
                        return COMM_SUCCESS;
                    }
                    else
                    {
                        //检查是否超时
                        gettimeofday(&timenow,NULL);
                        time_fly = this->cal_time_duration(&timenow,&timeout_start);
                        if(time_fly >= timeout_ms)
                        {
                            //this->ack_fail_count++;
                            //if(this->ack_fail_count >= ACK_BEAT_COUNT )
                            //{
                            //    cout << "ack responce fail count " << ACK_BEAT_COUNT <<  endl;
                            //    this->net_state = NET_OFFLINE;
                            //}
                            break; //退出while循环
                        }
                    }

                }

                //退出自while循环，并判断是第几次退出
                if(i >=  (RETRY_TCP_COUNT-1))
                {
                    return COMM_TIMEOUT;
                }

            }
            else  //连发送都是失败的。。。
            {
                return COMM_FAIL;
            }

        }
    }
    else
    {
        printf("NET OFFLINE\r\n");
        return COMM_FAIL;
    }
}



int net_march :: send_beat_msg(struct CMD_MSG * p_data,int timeout_ms,struct CMD_MSG * p_recv)
{
    int ret;
    int i;
    struct timeval timeout_start;
    struct timeval timenow;
    int time_fly;


    unsigned short crc_temp;


    this->pack_msg.frame_head[0] = 0x7E;
    this->pack_msg.frame_head[1] = 0x7E;
    memcpy(&this->pack_msg.frame_msg, p_data,sizeof(struct CMD_MSG));
    this->pack_msg.frame_end[0] = 0xFC;
    this->pack_msg.frame_end[1] = 0xFC;


    __sync_fetch_and_add (&this->pack_count,1);
    this->pack_msg.frame_msg.msg_num[0] = ((unsigned char *)(&this->pack_count))[3];
    this->pack_msg.frame_msg.msg_num[1] = ((unsigned char *)(&this->pack_count))[2];
    this->pack_msg.frame_msg.msg_num[2] = ((unsigned char *)(&this->pack_count))[1];
    this->pack_msg.frame_msg.msg_num[3] = ((unsigned char *)(&this->pack_count))[0];


    crc_temp = this->crc16((unsigned char *)(&this->pack_msg),sizeof(struct CMD_MSG)+4);

    this->pack_msg.crc[0] = ((unsigned char *)(&crc_temp))[1];
    this->pack_msg.crc[1] = ((unsigned char *)(&crc_temp))[0];


    if(NET_READY ==  this->net_state )
    {
        //printf("before send\r\n");
        //printf("p_data  %x, %x, %x, %x\r\n",p_data->msg_num[0],p_data->msg_num[1],p_data->msg_num[2],p_data->msg_num[3]);
        ret = send(this->fd_net,&this->pack_msg,sizeof(struct PACK_MSG),0);

        if(ret == sizeof(struct PACK_MSG))
        {
            //记录启动时间
            gettimeofday(&timeout_start,NULL);

            //手动超时等待
            while(1)
            {
                //printf("123\r\n");
                usleep(50*1000);

                //printf("p_data  %x, %x, %x, %x\r\n",p_data->msg_num[0],p_data->msg_num[1],p_data->msg_num[2],p_data->msg_num[3]);
                //printf("p_recv  %x, %x, %x, %x\r\n",p_recv->msg_num[0],p_recv->msg_num[1],p_recv->msg_num[2],p_recv->msg_num[3]);
                //只在主动发送数据包中需要等待返回响应，并根据包编号判断是否是针对的响应
                if((this->pack_msg.frame_msg.msg_num[0] == p_recv->msg_num[0])
                 &&(this->pack_msg.frame_msg.msg_num[1] == p_recv->msg_num[1])
                 &&(this->pack_msg.frame_msg.msg_num[2] == p_recv->msg_num[2])
                 &&(this->pack_msg.frame_msg.msg_num[3] == p_recv->msg_num[3]))
                {
                    //接受到返回响应
                    cout << "ack beat responce ok " << endl;
                    this->ack_fail_count = 0;
                    return COMM_SUCCESS;
                }
                else
                {
                    //检查是否超时
                    gettimeofday(&timenow,NULL);
                    time_fly = this->cal_time_duration(&timenow,&timeout_start);
                    if(time_fly >= timeout_ms)
                    {
                        this->ack_fail_count++;
                        if(this->ack_fail_count >= ACK_BEAT_COUNT )
                        {
                            cout << "ack beat responce fail count " << ACK_BEAT_COUNT <<  endl;
                            this->net_state = NET_OFFLINE;
                        }
                        break; //退出while循环
                    }
                }

            }

            //退出自while循环，并判断是第几次退出
            if(i >= (RETRY_TCP_COUNT-1))
            {
                return COMM_TIMEOUT;
            }

        }
        else  //连发送都是失败的。。。
        {
            return COMM_FAIL;
        }


    }
    else
    {
        printf("NET OFFLINE\r\n");
        return COMM_FAIL;
    }
}
/*
//类似net_march :: send_tcp_responce，同样为无响应式的，只是有个编号要自增
int net_march :: send_tcp_heart_beat(struct CMD_MSG * p_data)
{
    int ret;
    unsigned short crc_temp;
    unsigned int s_buf_len;

    this->pack_msg.frame_head[0] = 0x7E;
    this->pack_msg.frame_head[1] = 0x7E;
    memcpy(&this->pack_msg.frame_msg, p_data,sizeof(struct CMD_MSG));
    this->pack_msg.frame_end[0] = 0xFC;
    this->pack_msg.frame_end[1] = 0xFC;

    __sync_fetch_and_add (&this->pack_count,1);
    this->pack_msg.frame_msg.msg_num[0] = ((unsigned char *)(&this->pack_count))[3];
    this->pack_msg.frame_msg.msg_num[1] = ((unsigned char *)(&this->pack_count))[2];
    this->pack_msg.frame_msg.msg_num[2] = ((unsigned char *)(&this->pack_count))[1];
    this->pack_msg.frame_msg.msg_num[3] = ((unsigned char *)(&this->pack_count))[0];

    crc_temp = this->crc16((unsigned char *)(&this->pack_msg),sizeof(struct CMD_MSG)+4);

    this->pack_msg.crc[0] = ((unsigned char *)(&crc_temp))[1];
    this->pack_msg.crc[1] = ((unsigned char *)(&crc_temp))[0];


    if(NET_READY ==  this->net_state )
    {
        printf("b fd_net=%d\r\n",this->fd_net);

        ret = send(this->fd_net,&this->pack_msg,sizeof(struct PACK_MSG),0);

        printf("fd_net=%d\r\n",this->fd_net);

        printf("send ret=%d\r\n",ret);

        //ioctl(this->fd_net, SIOCOUTQ, &s_buf_len);

        //printf("this .........s_buf_len=%d\r\n",s_buf_len);

        if(ret == sizeof(struct PACK_MSG))
        {
            return COMM_SUCCESS;
        }
        else
        {
            return COMM_FAIL;
        }
    }
    else
    {
        printf("NET OFFLINE\r\n");
        return COMM_FAIL;
    }

}
*/

//无响应式发送
int net_march :: send_tcp_responce(struct CMD_MSG * p_data)
{
    int ret;
    unsigned short crc_temp;

    this->pack_msg.frame_head[0] = 0x7E;
    this->pack_msg.frame_head[1] = 0x7E;
    memcpy(&this->pack_msg.frame_msg, p_data,sizeof(struct CMD_MSG));
    this->pack_msg.frame_end[0] = 0xFC;
    this->pack_msg.frame_end[1] = 0xFC;

    crc_temp = this->crc16((unsigned char *)(&this->pack_msg),sizeof(struct CMD_MSG)+4);

    this->pack_msg.crc[0] = ((unsigned char *)(&crc_temp))[1];
    this->pack_msg.crc[1] = ((unsigned char *)(&crc_temp))[0];

    if(NET_READY ==  this->net_state )
    {
        ret = send(this->fd_net,&this->pack_msg,sizeof(struct PACK_MSG),0);

        if(ret == sizeof(struct PACK_MSG))
        {
            return COMM_SUCCESS;
        }
        else
        {
            return COMM_FAIL;
        }
    }
    else
    {
        printf("NET OFFLINE\r\n");
        return COMM_FAIL;
    }
}




unsigned short net_march :: crc16(unsigned char * dest, unsigned int wDataLen)
{
        unsigned char chCRCHi = 0xFF; // 高CRC字节初始化
        unsigned char chCRCLo = 0xFF; // 低CRC字节初始化
        unsigned int wIndex;            // CRC循环中的索引
        unsigned char * pchMsg;
        pchMsg = dest;
        while (wDataLen--)
        {
                // 计算CRC
                wIndex = chCRCLo ^ *pchMsg++ ;
                chCRCLo = chCRCHi ^ chCRCHTalbe[wIndex];
                chCRCHi = chCRCLTalbe[wIndex] ;
        }
        return ((chCRCHi << 8) | chCRCLo) ;
}




/*
void net_march :: InvertUint8(unsigned char *dBuf, unsigned char *srcBuf)
{
    int i;
    unsigned char tmp[4];
    tmp[0] = 0;
    for (i = 0;i< 8;i++)
    {
        if (srcBuf[0] & (1 << i))
            tmp[0] |= 1 << (7 - i);
    }
    dBuf[0] = tmp[0];

}

void net_march :: InvertUint16(unsigned short *dBuf, unsigned short *srcBuf)
{
    int i;
    unsigned short tmp[4];
    tmp[0] = 0;
    for (i = 0;i< 16;i++)
    {
        if (srcBuf[0] & (1 << i))
            tmp[0] |= 1 << (15 - i);
    }
    dBuf[0] = tmp[0];
}

unsigned short net_march ::  crc16(unsigned char *puchMsg, unsigned int usDataLen)
{
    unsigned short wCRCin = 0xFFFF;
    unsigned short wCPoly = 0x8005;
    unsigned char wChar = 0;
    int i;

    while (usDataLen--)
    {
        wChar = *(puchMsg++);
        this->InvertUint8(&wChar, &wChar);
        wCRCin ^= (wChar << 8);
        for (i = 0;i < 8;i++)
        {
            if (wCRCin & 0x8000)
                wCRCin = (wCRCin << 1) ^ wCPoly;
            else
                wCRCin = wCRCin << 1;
        }
    }
    this->InvertUint16(&wCRCin, &wCRCin);
    return (wCRCin);
}
*/

int net_march :: cal_time_duration( struct timeval * fresh_time,struct timeval * old_time)
{
    int sec,usec;
    int duration_msec;

    sec = fresh_time->tv_sec - old_time->tv_sec;
    usec = fresh_time->tv_usec - old_time->tv_usec;


    duration_msec = (sec*1000 + usec/1000);


    if(duration_msec < 0)  //时钟被向前改动了,强制返回86400ms
    {
        printf("time chage\r\n");
        return 86400;
    }

    return duration_msec;
}

