#include "lock_ptl.h"










int lock_ptl :: make_ctl_cmd(
                             unsigned char device_add,      //目标设备地址
                             unsigned char * msg_info_p,    //tcp业务编码，对应485数据内容中的 业务代码\地址
                             unsigned char * msg_parm_p)    //tcp业务参数，对应485数据内容中的 业务参数，即待写入的具体数值，
                                                            //tcp中该部分数据为8字节的，其实都只用了一个字节，因为485都是单个字节写入的，
                                                            //但tcp目前为确定是msg_parm_p[0]\msg_parm_p[7]
{


    if(msg_info_p[0] == 0x10&& msg_info_p[1] == 0x01)           //升锁
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = msg_parm_p[7];

        this->cmd_pool_depth = 1;

    }
    else if(msg_info_p[0] == 0x10&& msg_info_p[1] == 0x02)          //降锁
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = msg_parm_p[7];

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x10&& msg_info_p[1] == 0x03)          //车位锁翻板停止运行
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = 0x00;

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x10&& msg_info_p[1] == 0x09)          //重启车位锁=》重启检车器
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = 0x00;

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x10&& msg_info_p[1] == 0x0A)          //重启检车器=》重启车位锁
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = 0x00;

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x10&& msg_info_p[1] == 0x05)      //电机故障清除复位
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = 0x00;

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x10&& msg_info_p[1] == 0x07)          //夜间警示灯控制
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = msg_parm_p[7];

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x30&& msg_info_p[1] == 0x05)          //设置电流阈值
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = msg_parm_p[7];

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x30&& msg_info_p[1] == 0x01)          //设置延时升锁时间
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = msg_parm_p[7];

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x30&& msg_info_p[1] == 0x03)          //设置降锁离场时间
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = msg_parm_p[7];

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x30&& msg_info_p[1] == 0x09)          //设置感应方式
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = msg_parm_p[7];

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x30&& msg_info_p[1] == 0x07)          //设置翻板高度阈值
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = msg_parm_p[7];

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x30&& msg_info_p[1] == 0x0B)          //设置压力检测阈值
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = msg_parm_p[7];

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x30&& msg_info_p[1] == 0x0C)          //设置压力校准
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = msg_parm_p[7];

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x30&& msg_info_p[1] == 0x0E)          //设置上锁后翻板调整间隔时间
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x06;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = msg_parm_p[7];

        this->cmd_pool_depth = 1;
    }
    else
    {
        cout << "impossiable ..." << endl;
        this->cmd_pool_depth = 0;
    }


    return 0;
}


//查询数据类 tcp转485
//该函数将tcp协议内容转为485协议内容，并将485返回数据转为tcp协议，并将485返回数据的属性转为tcp返回数据的属性（success outtime。。。）
int lock_ptl :: make_res_cmd(
                             unsigned char device_add,    //目标设备地址
                             unsigned char * msg_info_p,  //tcp业务编码，对应485数据内容中的 业务代码\地址
                             unsigned char * msg_parm_p)  //tcp业务参数，对应485数据内容中的 业务参数。此函数为读取，该参数无意义

{
    //以下为：tcp指令为一个，对应一个485指令
    //////////////////////////////////////////////////
    if(msg_info_p[0] == 0x20&& msg_info_p[1] == 0xC7)  //查询电流阈值
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x03;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = 0x01;

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x20&& msg_info_p[1] == 0xC5) //查询延时升锁时间
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x03;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = 0x01;

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x20&& msg_info_p[1] == 0xC6) //查询降锁离场时间
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x03;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = 0x01;

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x20&& msg_info_p[1] == 0xC9) //查询感应方式
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x03;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = 0x01;

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x20&& msg_info_p[1] == 0xC8) //查询翻板高度阈值
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x03;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = 0x01;

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0x20&& msg_info_p[1] == 0xCA) //查询压力检测阈值
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x03;
        this->modbus_cmd_pool[0][2] = msg_info_p[0];
        this->modbus_cmd_pool[0][3] = msg_info_p[1];
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = 0x01;

        this->cmd_pool_depth = 1;
    }
    else if(msg_info_p[0] == 0xF0&& msg_info_p[1] == 0x1A) //查询单个车位锁状态信息
    {
        this->modbus_cmd_pool[0][0] = device_add;
        this->modbus_cmd_pool[0][1] = 0x03;
        this->modbus_cmd_pool[0][2] = 0x20;
        this->modbus_cmd_pool[0][3] = 0x01;
        this->modbus_cmd_pool[0][4] = 0x00;
        this->modbus_cmd_pool[0][5] = 0x01;

        this->cmd_pool_depth = 1;
    }
    else
    {
        //以下为：tcp指令为一个，对应多个485指令
        ///////////////////////////////////////////////////////////////////////////////
        if(msg_info_p[0] == 0xF0
        && msg_info_p[1] == 0x19) //查询车位锁基本信息
        {
            //查询软件版本号
            this->modbus_cmd_pool[0][0] = device_add;
            this->modbus_cmd_pool[0][1] = 0x03;
            this->modbus_cmd_pool[0][2] = 0x20;
            this->modbus_cmd_pool[0][3] = 0xC3;
            this->modbus_cmd_pool[0][4] = 0x00;
            this->modbus_cmd_pool[0][5] = 0x01;

            this->cmd_pool_depth = 1;


            //查询设备版本号
            this->modbus_cmd_pool[1][0] = device_add;
            this->modbus_cmd_pool[1][1] = 0x03;
            this->modbus_cmd_pool[1][2] = 0x20;
            this->modbus_cmd_pool[1][3] = 0xC0;
            this->modbus_cmd_pool[1][4] = 0x00;
            this->modbus_cmd_pool[1][5] = 0x01;

            this->cmd_pool_depth = 2;
        }
        else
        {

        }

    }
    return 0;

}

//执行查询命令队列
int lock_ptl :: execute_ctl_cmd(void)
{
    unsigned char responce_cmd[32];
    int responce_cmd_len;
    int ret;
    int i,j;

    for(i = 0;i<this->cmd_pool_depth;i++)
    {
        for(j = 0;j<RETRY_485_COUNT;j++)
        {
            ret = this->l_4_p->rtu_write(this->modbus_cmd_pool[i],6,responce_cmd,&responce_cmd_len,1,50);

            //break;

            if(COMM_FAIL == ret)
            {
                //返回COMM_FAIL是出现了非业务层面的错误,同样也对外报失败，无再次尝试必要
                return COMM_FAIL;
            }

            if(COMM_TIMEOUT == ret)
            {
                //超时，没有响应，正常情况下，就是锁没有响应了，continue，n次尝试
                cout<< "execute_ctl_cmd recv timeout try " << j << endl;
                continue; //超时没有响应返回，则再尝试一下
            }

            if(COMM_SUCCESS == ret) //有返回数据了，则实际解析
            {
                if(0x06 == responce_cmd[1])// && 0x00 == responce_cmd[4] && 0x00 == responce_cmd[5]) //表示执行成功
                {
                    cout << "finish one cmd" << endl;
                    break; //实际解析成功，去往可能的下一个执行命令
                }
                else if(0x86 == responce_cmd[1])
                //&& (0x00 != responce_cmd[4] || 0x00 != responce_cmd[5])) //执行失败，但原因是485设备在忙，可以考虑再次尝试
                {
                    return COMM_BUSY;
                    //continue;
                }
                else  //接受到的数据是异常的一些响应，这个的确有这可能，想想如何处理
                {
                    cout<< "execute_ctl_cmd recv stange responce" <<endl;

                    for(i = 0;i<8;i++)
                    {
                        printf("responce_cmd[%d] = 0x%2x\r\n",i,responce_cmd[i]);
                    }

                    this->l_4_p->serial_485_flush_read_buf();
                    //这里其实应该退出了，毕竟接受到的是些异常的信息了
                    //要么做进一步分析，要么做什么动作，否则直接退出
                    return COMM_UNKNOWN;
                }

            }
        }
        if(j >= RETRY_485_COUNT && COMM_TIMEOUT == ret)
        {
            return COMM_TIMEOUT;
        }

    }

    return COMM_SUCCESS;
}


int lock_ptl :: execute_res_cmd(unsigned char * msg_info_p,unsigned char lock_addr,unsigned char * responce_tcp_pack_formate)
{
    unsigned char to_tcp[8];
    unsigned char responce_cmd[5][32];
    unsigned char responce_cmd_temp[32];
    int responce_cmd_len;
    int ret;
    int i,j,k;

    //printf("first ret brefore this->cmd_pool_depth = %d\r\n",this->cmd_pool_depth);
    for(i = 0;i<this->cmd_pool_depth;i++)
    {
        //printf("first ret brefore\r\n");

        for(j = 0;j<RETRY_485_COUNT;j++)
        {
            ret = this->l_4_p->rtu_read(this->modbus_cmd_pool[i],6,responce_cmd_temp,&responce_cmd_len,50);

            //printf("first ret = %d\r\n",ret);

            if(COMM_FAIL == ret)
            {
                return COMM_FAIL;  //返回COMM_FAIL是出现了非业务层面的错误,同样也对外报失败，无再次尝试必要
            }

            if(COMM_TIMEOUT == ret)
            {
                //超时，没有响应，正常情况下，就是锁没有响应了，continue，n次尝试
                cout<< "this try is count " << j+1 << " but execute_res_cmd recv timeout "<< endl;
                continue; //超时没有响应返回，则再尝试一下
            }

            if(COMM_SUCCESS == ret) //有返回数据了，则实际解析
            {
                if(0x03 == responce_cmd_temp[1]) //表示执行成功
                {
                    memset(responce_cmd[i],0,sizeof(responce_cmd[i]));

                    //将大端的485数据部分转移给方向tcp的数据区域
                    //for(k = 0;k<responce_cmd_len-5;k++)
                    //{
                    //    responce_cmd[i][7-k] = responce_cmd_temp[responce_cmd_len-3-k];
                    //}
                    memcpy(responce_cmd[i],responce_cmd_temp,responce_cmd_len);
                    break; //实际解析成功，去往可能的下一个执行命令
                }
                else if(0x83 == responce_cmd_temp[1])
                    //&& (0x00 != responce_cmd_temp[4] || 0x00 != responce_cmd_temp[5])) //执行失败，但原因是485设备在忙，可以考虑再次尝试
                {
                    return COMM_BUSY;
                    //continue;
                }
                else
                {
                    cout<< "execute_res_cmd recv stange responce" <<endl;

                    for(i = 0;i<8;i++)
                    {
                        printf("responce_cmd_len[%d] = 0x%2x\r\n",i,responce_cmd_temp[i]);
                    }

                    this->l_4_p->serial_485_flush_read_buf();

                    return COMM_UNKNOWN;
                }

            }
        }
        if(j >= RETRY_485_COUNT && COMM_TIMEOUT == ret)
        {
            cout<< "final get 485 timeout" <<endl;
            return COMM_TIMEOUT;
        }
    }


    //这里其实可以直接把tcp要的数据格式赋给responce_tcp_pack_formate
    memset(to_tcp,0,sizeof(to_tcp));

    if(0xF0 ==  msg_info_p[0] && 0x1A == msg_info_p[1])  //查询的寄存器是2byte的
    {
        //存放在返回tcp包的高低字节未定

        to_tcp[6] = responce_cmd[0][3];  //中置锁2002H寄存器的高字节数据
        to_tcp[7] = responce_cmd[0][4];  //中置锁2001H寄存器的低字节数据
    }
    else if(0xF0 == msg_info_p[0] && 0x19 == msg_info_p[1])  //查询的寄存器是2byte的，且有两个命令
    {
        //软件版本号
        to_tcp[6] = responce_cmd[0][3];  //中置锁20C3H寄存器的高字节数据
        to_tcp[7] = responce_cmd[0][4];  //中置锁20C3H寄存器的低字节数据

        //设备版本号
        to_tcp[4] = responce_cmd[1][3];  //中置锁20C0H寄存器的高字节数据
        to_tcp[5] = responce_cmd[1][4];  //中置锁20C0H寄存器的低字节数据

        //设备类型
        to_tcp[2] = 0x00;
        to_tcp[3] = 0x01;

        //波特率
        to_tcp[1] = 0x03; //固定9600

        //车位锁通讯地址
        to_tcp[0] = lock_addr;

    }
    else
    {
        to_tcp[7] = responce_cmd[0][4];
        //this->msg_485_to_tcp[8] = responce_cmd[3];
    }

    memcpy(responce_tcp_pack_formate,to_tcp,8);

    return COMM_SUCCESS;
}




/*

//查询过  2001H
int lock_ptl:: make_state_check(unsigned char * responce_tcp)
{
    unsigned char tcp_to_485[5];
    unsigned char res_tata_team[2];
    int ret;

    memset(tcp_to_485,0,sizeof(tcp_to_485));


    tcp_to_485[0] = 0x03;

    tcp_to_485[1] = 0x20;
    tcp_to_485[2] = 0x01;


    tcp_to_485[3] = 0x00;
    tcp_to_485[4] = 0x00;


    ret = this->send_485_res_16(tcp_to_485,res_data_team,1);
    if(1 == ret)
    {
        //查询顺利
    }


    return 0;
}





int lock_ptl:: check_live(void)
{

    return 0;
}


int lock_ptl:: make_state_check(unsigned char * responce_tcp)
{


    return 0;
}
*/
