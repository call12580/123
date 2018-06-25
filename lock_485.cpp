#include "lock_485.h"






lock_485::lock_485(int init_bps,const char * init_port_name):device_band_rate(init_bps),port_name(init_port_name)
{

    this->fd = -1;

}


int lock_485 :: serial_485_write_test(void)
{
    unsigned char code_485[8];
    int head_len = 6;
    unsigned short crc_temp;


    code_485[0] = 0x01;
    code_485[1] = 0x06;
    code_485[2] = 0x10;
    code_485[3] = 0x02;
    code_485[4] = 0x00;
    code_485[5] = 0x00;
    //code_485[6] = 0xCA;
    //code_485[7] = 0xDC;
    crc_temp = this->crc16(code_485,6);
    //memcpy(&code_485[6],&crctemp,2);
    code_485[6] = ((unsigned char *)(&crc_temp))[1];
    code_485[7] = ((unsigned char *)(&crc_temp))[0];

    for(int a = 0;a<8;a++)
    {
        printf("code_485[%d] = 0x%2x\r\n",a,code_485[a]);
    }

    //return COMM_SUCCESS;

    write(this->fd,code_485,8);


    return 0;
}


int lock_485 :: rtu_write(unsigned char * head_p,
                          int head_len,
                          unsigned char * responce_back,
                          int * responce_len,
                          int back_responce_flag,
                          int time_out_ms)
{
    unsigned short crc_temp;
    int ret,nread;
    int desire_get_len = 8;

    //如果需要响应，则存放在这里
    unsigned char read_temp[32];
    int send_len = head_len + 2;

    //用于存放完整的485帧数据，由于传入的头固定时6，那么加上CRC，也就固定为8字节
    unsigned char code_485[8];

    int nfds;
    fd_set readfds;
    struct timeval tv;

    //crc_temp = this->crc16(head_p,head_len);

    memcpy(code_485,head_p,head_len);

    crc_temp = this->crc16(code_485,head_len);

    code_485[6] = ((unsigned char *)(&crc_temp))[1];
    code_485[7] = ((unsigned char *)(&crc_temp))[0];



    //read_temp[1] = 0x06;
    //read_temp[4] = 0x00;
    //read_temp[5] = 0x00;
    //memcpy(responce_back,read_temp,8);
    //return COMM_SUCCESS;

    //tcflush(this->fd, TCIOFLUSH);

    //usleep(500*1000);

    printf("lock_485.cpp rtu_write write is \r\n");
    printf("code_485[0~7]:");
    for(int i = 0;i<8;i++)
    {
        printf("0x%2x  ",code_485[i]);
    }
    printf("\r\n");

    if(0 == back_responce_flag)    //不需要等待响应，只要发送成功即可
    {
        ret = write(this->fd,code_485,send_len);
        if(ret == send_len)
        {
            cout << "485 send ok"  << endl;
            return COMM_SUCCESS;
        }
        else
        {
            cout << "485 send fail"  << endl;
            return COMM_TIMEOUT;
        }
    }
    else                        //需要（必须）超时等待响应的发送
    {
        ret = write(this->fd,code_485,send_len);
        if(ret == send_len)
        {
            tv.tv_sec = time_out_ms/1000;
            tv.tv_usec = time_out_ms*1000;
            FD_ZERO(&readfds);
            FD_SET(this->fd,&readfds);
            bzero(read_temp,sizeof(read_temp));
            nfds = select(this->fd+1,&readfds,NULL,NULL,&tv);

            cout << "485 send ok"  << endl;

            if(nfds == 0){
                cout << "485 recv responce time out"  << endl;
                return COMM_TIMEOUT;    //等待响应时超时
            }
            else{
                nread = read(this->fd,read_temp,desire_get_len);

                cout << "485 recv responce something " << nread << endl;

                if(0 < nread && nread <= 8){
                    //准确接受到响应信息，至于接受到的信息代表什么意思，则由于485协议的拟定当时，建议在上一层完整解析
                    printf("recevie responce is \r\n");
                    for(int a = 0;a<8;a++)
                    {
                        printf("read_temp[%d] = 0x%2x\r\n",a,read_temp[a]);
                    }

                    memcpy(responce_back,read_temp,nread);
                    *responce_len = nread;
                    return COMM_SUCCESS;

                }
                else
                {
                    printf("lock_485.cpp rtu_write recevie responce len is \r\n",nread);
                    return COMM_TIMEOUT;    //等待响应时出错，一般为fd被关闭，一般不会出现
                }
            }
        }
        else
        {
            return COMM_TIMEOUT;
        }
    }


    return 0;
}

int lock_485 :: rtu_read(unsigned char * head_p,
                         int head_len,
                         unsigned char * responce_back,
                         int * responce_len,
                         int time_out_ms)
{
    unsigned short crc_temp;
    int ret,nread;
    int i;

    //如果需要响应，则存放在这里
    unsigned char read_temp[32];
    int send_len = head_len + 2;

    //用于存放完整的485帧数据，由于传入的头固定时6，那么加上CRC，也就固定为8字节
    unsigned char code_485[8];

    int nfds;
    fd_set readfds;
    struct timeval tv;


    memcpy(code_485,head_p,head_len);

    crc_temp = this->crc16(code_485,head_len);

    code_485[6] = ((unsigned char *)(&crc_temp))[1];
    code_485[7] = ((unsigned char *)(&crc_temp))[0];


    //tcflush(this->fd,TCIFLUSH);
    //tcflush(this->fd, TCIOFLUSH);

    //usleep(500*1000);

    ret = write(this->fd,code_485,send_len);


    if(ret == send_len)
    {
        printf("lock_485.cpp rtu_read write is \r\n");
        printf("code_485[0~7]:");
        for(int i = 0;i<8;i++)
        {
            printf("0x%2x  ",code_485[i]);
        }
        printf("\r\n");

        cout << "485 send ok"  << endl;

        tv.tv_sec = time_out_ms/1000;
        tv.tv_usec = time_out_ms*1000;
        FD_ZERO(&readfds);
        FD_SET(this->fd,&readfds);
        bzero(read_temp,sizeof(read_temp));
        nfds = select(this->fd+1,&readfds,NULL,NULL,&tv);

        if(nfds == 0){
            cout << "485 recv responce time out"  << endl;
            return COMM_TIMEOUT;    //等待响应时超时
        }
        else{
            nread = read(this->fd,read_temp,3);  //线读取返回头

            cout << "485 recv responce head something " << nread << endl;

            //printf("recevie responce head is %d\r\n",nread);
            //for(int a = 0;a<3;a++)
            //{
            //    printf("code_485[%d] = 0x%2x\r\n",a,code_485[a]);
            //}
            /*
            if(nread<=0){
                return COMM_FAIL;    //等待响应时出错，一般为fd被关闭，一般不会出现
            }
            else        //准确接受到响应信息，至于接受到的信息代表什么意思，则由于485协议的拟定当时，建议在上一层完整解析
            {
                //这里假设一旦读取到头三个就能正确接受了后面的量了，实际这么做可能有bug，并会导致死在这个read
                printf("485 recv responce something left is  data len + crc len is(bytes) = %d \r\n",read_temp[2] + 2);
                nread = read(this->fd,&read_temp[3],read_temp[2]+ 0x02);



                printf("recevie responce is \r\n");
                for(int a = 0;a< read_temp[2]+5;a++)
                {
                    printf("read_temp[%d] = 0x%2x\r\n",a,read_temp[a]);
                }

                memcpy(responce_back,read_temp,nread+3);
                *responce_len = nread+3;


                return COMM_SUCCESS;
            }
            */

            if(3 == nread)
            {
                //这里假设一旦读取到头三个就能正确接受了后面的量了，实际这么做可能有bug，并会导致死在这个read
                printf("485 recv responce something left is  data len + crc len is(bytes) = %d \r\n",read_temp[2] + 2);

                if(read_temp[2] <= RECV485_DTU_DATA_MAX)
                {
                    nread = read(this->fd,&read_temp[3],read_temp[2]+ 0x02);

                    printf("recevie sense responce is \r\n");
                    for(int a = 0;a< read_temp[2]+5;a++)
                    {
                        printf("read_temp[%d] = 0x%2x\r\n",a,read_temp[a]);
                    }

                    memcpy(responce_back,read_temp,nread+3);
                    *responce_len = nread+3;

                    return COMM_SUCCESS;
                }
                else
                {
                    //只要响应失败，即可一认为某种程度上
                    return COMM_TIMEOUT;
                }
            }
            else
            {
                return COMM_TIMEOUT;    //等待响应时超时
            }

        }
    }
    else
    {
        return COMM_TIMEOUT;
    }

}

/*
int lock_485 :: send_msg_rtu(unsigned char * msg)
{
    int ret;
    int send_len = 8;

    ret = write(this->fd,msg,send_len);
    if(ret == send_len)
    {
        return COMM_SUCCESS;
    }
    else
    {
        return COMM_FAIL;
    }

}

//
int lock_485 :: recv_msg(unsigned char * responce_485,int time_out_ms)
{
    int nfds;
    int nread = 0 ;
    char read_temp[32];
    fd_set readfds;
    struct timeval tv;

    int desire_get_len = 8;

    tv.tv_sec = time_out_ms/1000;
    tv.tv_usec = time_out_ms%1000;
    FD_ZERO(&readfds);
    FD_SET(this->fd,&readfds);
    bzero(read_temp,sizeof(read_temp));
    nfds = select(this->fd+1,&readfds,NULL,NULL,&tv);

    if(nfds == 0){
        return COMM_TIMEOUT;
    }
    else{
        if(0 == desire_get_len){
            nread = read(this->fd,read_temp,32);
        }
        else{
           nread = read(this->fd,read_temp,desire_get_len);
        }

        if(nread<=0){
            return COMM_FAIL;
        }
        else if(8 == nread){
            memcpy(responce_485,read_temp,8);
            return COMM_SUCCESS;
        }
        else{
            return COMM_UNKNOWN;
        }

    }
}



//recv
int lock_485 :: recv_msg_1byte(unsigned char * responce_485,int time_out_ms)
{
    int nfds;
    int nread = 0 ;
    char read_temp[32];
    fd_set readfds;
    struct timeval tv;

    int desire_get_len = 8;

    tv.tv_sec = time_out_ms/1000;
    tv.tv_usec = time_out_ms%1000;
    FD_ZERO(&readfds);
    FD_SET(this->fd,&readfds);
    bzero(read_temp,sizeof(read_temp));
    nfds = select(this->fd+1,&readfds,NULL,NULL,&tv);

    if(nfds == 0){
        return COMM_TIMEOUT;
    }
    else{
        if(0 == desire_get_len){
            nread = read(this->fd,read_temp,32);
        }
        else{
           nread = read(this->fd,read_temp,desire_get_len);
        }

        if(nread<=0){
            return COMM_FAIL;
        }
        else if(8 == nread){
            memcpy(responce_485,read_temp,8);
            return COMM_SUCCESS;
        }
        else{
            return COMM_UNKNOWN;
        }

    }
}


int lock_485 :: recv_msg_2byte(unsigned char * responce_485,int time_out_ms)
{

    int nfds;
    int nread = 0 ;
    char read_temp[32];
    fd_set readfds;
    struct timeval tv;

    int desire_get_len = 16;

    tv.tv_sec = time_out_ms/1000;
    tv.tv_usec = time_out_ms%1000;
    FD_ZERO(&readfds);
    FD_SET(this->fd,&readfds);
    bzero(read_temp,sizeof(read_temp));
    nfds = select(this->fd+1,&readfds,NULL,NULL,&tv);

    if(nfds == 0){
        return COMM_TIMEOUT;
    }
    else
    {
        if(0 == desire_get_len){
            nread = read(this->fd,read_temp,32);
        }
        else{
           nread = read(this->fd,read_temp,desire_get_len);
        }

        if(nread<=0){
            return COMM_FAIL;
        }
        else if(16 == nread){
            memcpy(responce_485,read_temp,16);
            return COMM_SUCCESS;
        }
        else{
            return COMM_UNKNOWN;
        }
    }
}
*/



int lock_485:: serial_485_open(void)
{
    struct termios options;    //usart par struct

    this->fd = open(this->port_name, O_RDWR|O_NOCTTY);

    if(this->fd <= 0)
    {
        printf("debug here !\r\n");
        return -1;
    }

    tcgetattr(this->fd, &options);        //获取opt指针

    switch (this->device_band_rate)
    {
        case 2400:
            cfsetispeed(&options, B2400);
            cfsetospeed(&options, B2400);
            break;
        case 4800:
            cfsetispeed(&options, B4800);
            cfsetospeed(&options, B4800);
            break;
        case 9600:
            cfsetispeed(&options, B9600);
            cfsetospeed(&options, B9600);
            break;
        case 19200:
            cfsetispeed(&options, B19200);
            cfsetospeed(&options, B19200);
            break;
        default:
            cfsetispeed(&options, B9600);
            cfsetospeed(&options, B9600);
            break;
    }


    options.c_cflag |= (CLOCAL | CREAD);    //enable date receiver
    options.c_cflag &= ~PARENB;      //无校验
    options.c_cflag &= ~CRTSCTS;     //没有数据流
    options.c_cflag &= ~CSTOPB;   //关闭两位停止位，就是一位停止位
    options.c_cflag &= ~CSIZE;    //设置数据位宽时打开掩码
    options.c_cflag |= CS8;       //8位数据位

    //关闭ICRNL IXON 防止0x0d 0x11 0x13的过滤
    options.c_iflag &=~(IXON | IXOFF | IXANY);
    options.c_iflag &= ~ (INLCR | ICRNL | IGNCR);
    options.c_oflag &= ~(ONLCR | OCRNL);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    //options.c_oflag &= ~OPOST;term.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    //options.c_oflag  &= ~OPOST;
    options.c_oflag &= ~OPOST;   //使用原始数据

    //只有阻塞时，下面两个才有效，且针对读。
    options.c_cc[VMIN]  = 50;  //最小字节数（读）      4800  50  5
    options.c_cc[VTIME] = 1;     //等待时间，单位百毫秒（读）  115200 50 1

    tcsetattr(this->fd, TCSANOW, &options);	//写入optons

    //tcflush(this->fd, TCIOFLUSH);

    //usleep(500*1000);

    return 0;
}


int lock_485:: serial_485_close(void)
{
    close(this->fd);

    return 0;
}

int lock_485:: serial_485_flush_read_buf(void)
{
    int nfds,nread;
    fd_set readfds;
    struct timeval tv;
    unsigned char read_temp[4];

    if(this->fd <= 0)
    {
        printf("serial_flush_read debug here !\r\n");
        return -1;
    }
    else
    {
        while(1)
        {
            tv.tv_sec = 0;
            tv.tv_usec = 50;
            FD_ZERO(&readfds);
            FD_SET(this->fd,&readfds);
            bzero(read_temp,sizeof(read_temp));
            nfds = select(this->fd+1,&readfds,NULL,NULL,&tv);

            if(nfds == 0){
                cout << "flush read buf ok"  << endl;
                break;   //等待响应时超时
            }
            else{
                nread = read(this->fd,read_temp,1);  //线读取返回头
                if(nread<=0){
                    return -1;
                }
                else
                {

                }
            }
        }
    }

    return 0;
}



unsigned short lock_485 :: crc16(unsigned char * dest, unsigned int wDataLen)
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
