#include "joint_3960.h"
#include "park_lk.h"



extern park_lk * lock_team[24];


joint_3960 :: joint_3960(net_march * init_n_m,unsigned char * init_node_index):n_m(init_n_m)
{
    struct sysinfo s_t;

    this->joint_ver[0] = 0x01;
    this->joint_ver[1] = 0x01;

    memcpy(this->joint_id,init_node_index,7);

    /*
    this->joint_id[0] = 0x01;
    this->joint_id[1] = 0x02;
    this->joint_id[2] = 0x03;
    this->joint_id[3] = 0x04;
    this->joint_id[4] = 0x05;
    this->joint_id[5] = 0x06;
    this->joint_id[6] = 0x07;
    */

    sysinfo(&s_t);
    this->last_beat_tick = s_t.uptime;

    this->cmd_ready = 0;

    this->beat_loop = 30;

    this->need_beat_flag = 0;

    this->boot_up_event_flag = 1;

}


int joint_3960::joint_recv_cmd(unsigned char * r_buf)
{
    int i;

    struct PACK_MSG * pack_head = (struct PACK_MSG *) r_buf;


    //准备进行临时的转储，在合适的时间点进行存储

    //查看系统事件

    //转存至一个大数据格式空间

    //这个大数据空间，在恰当的时间点可以存储到一个文件

    //在存储之间要查看文件的大小，若太大则明显是异常了，要注意


    //这些文件的创建是由本程序创建的，



    //将server被动发出的响应包和主动发出的指令包分开
    if(0x02 == pack_head->frame_msg.msg_type[1] || 0x04 == pack_head->frame_msg.msg_type[1])
    {
        memcpy(&this->msg_responce,&pack_head->frame_msg,26);


        printf("joint_3960.cpp joint_recv_cmd \r\n");
        for(i = 0;i<32;i++)
        {
            printf("0x%x ",r_buf[i]);
        }
        printf("\r\n");

    }
    else //剩余0x05 0x07
    {

        printf("joint_3960.cpp joint_recv_cmd \r\n");
        for(i = 0;i<32;i++)
        {
            printf("0x%x ",r_buf[i]);
        }
        printf("\r\n");

        if(CMD_IDLE ==  this->cmd_ready)
        {
            memcpy(&this->msg_run,&pack_head->frame_msg,26);
            this->cmd_ready = CMD_BUSY;
        }
        else
        {
            memcpy(&this->msg_run,&pack_head->frame_msg,26);
            this->responce_busy_back();
        }
    }

    return 0;
}


int joint_3960::joint_run_loop()
{
    int ret,i;
    struct sysinfo s_t;


    //每周期检查命令数据中，是否存在待执行的命令，0x05，0x07
    if(CMD_BUSY == this->cmd_ready)
    {
        if(0x05 == this->msg_run.msg_type[1])   //执行命令
        {
            ret = this->make_execute_ctl_cmd(this->msg_run.msg_info,this->msg_run.msg_parm);

            //这里存在一个矛盾，节点控制类命令都比较特殊，一个需要响应，另一个不需要响应，
            //且两种操作都是必然成功的，所以直接把响应动作放到协议解析处

            if(COMM_SUCCESS == ret)
            {
                //执行成功

                //this->responce_success_back();
            }
            else{
                //执行失败,返回的是COMM_UNKNOWN或者COMM_FAIL
                //告知tcp返回执行失败数据包
                //this->responce_fail_back();
            }
        }
        this->cmd_ready = CMD_IDLE;
    }


    //更新检测时间，如存在手动标志触发，或者正常周期到达，则发出一个心跳
    sysinfo(&s_t);
    if((1 == this->need_beat_flag) ||
     (((s_t.uptime - this->last_beat_tick) >= this->beat_loop)&&(0 == this->boot_up_event_flag)))
    {
        cout << "heat beat " << endl;
        ret = this->responce_heart_beat();

        printf("ret =  %d\r\n",ret);

        this->last_beat_tick = s_t.uptime;
        this->need_beat_flag = 0;
    }


    //boot up
    if(1 == this->boot_up_event_flag && NET_READY == this->n_m->net_state)
    {
        for(i = 0;i<LOCK_DEVICE_NUM;i++)
        {
            //所有设备地址完成检测
            if(0 == lock_team[i]->self_init_flag) break;
        }

        //所有锁已经经过初检，可发出上电事件
        if(i >= LOCK_DEVICE_NUM)
        {
            ret = this->responce_boot_up();

            if(COMM_SUCCESS == ret)
            {
                this->boot_up_event_flag = 0;
            }

            //完成 上电事件 后，即可开始心跳周期
            sysinfo(&s_t);
            this->last_beat_tick = s_t.uptime;
        }
        else  //还未完成开机初检
        {

        }
    }
    return 0;
}


//处理节点的控制类指令的：解析和响应
int joint_3960 :: make_execute_ctl_cmd(unsigned char * msg_info_p,unsigned char * msg_parm_p)  //tcp业务参数
{
    int ret;
    unsigned short time_interval_sec;

    if(msg_info_p[0] == 0xF0 && msg_info_p[1] == 0x11)  //设置心跳间隔
    {
        ((unsigned char *)(&time_interval_sec))[0] = msg_parm_p[7];
        ((unsigned char *)(&time_interval_sec))[1] = msg_parm_p[6];
        this->beat_loop = time_interval_sec;
        cout << "new time interval is " << this->beat_loop << endl;
        ret = this->responce_success_back();
    }
    else if(msg_info_p[0] == 0xF0&& msg_info_p[1] == 0x1B)  //转化为server 提前触发心跳
    {
        cout << "make heart beat right " << endl;
        this->need_beat_flag = 1;
    }
    else
    {
        printf("joint_3960.cpp make_execute_ctl_cmd get strnsg cmd code:\r\n");
        printf("msg_info_p = 0x%x 0x%x   ",msg_info_p[0],msg_info_p[1]);
        printf("msg_parm_p = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x  \r\n",msg_parm_p[0],msg_parm_p[1]
                                                                           ,msg_parm_p[2],msg_parm_p[3]
                                                                           ,msg_parm_p[3],msg_parm_p[5]                                                                           ,msg_parm_p[4],msg_parm_p[7]);
    }

    return 0;
}


//主动产生一个启动事件
int joint_3960:: responce_boot_up(void)
{
    int i,ret;

    memset(&this->msg_back,0,sizeof(struct CMD_MSG));

    this->msg_back.device_type[0] = 0x00;
    this->msg_back.device_type[1] = 0x02;

    memcpy(this->msg_back.device_id,this->joint_id,7);

    this->msg_back.msg_type[0] = 0x00;
    this->msg_back.msg_type[1] = 0x01;

    //this->msg_back.msg_num[] = 0x00;

    this->msg_back.msg_info[0] = 0xF0;
    this->msg_back.msg_info[1] = 0x01;

    for(i = 0;i<8;i++)
    {
        if(L_ONLINE == lock_team[i]->lock_power)
        {
            this->msg_back.msg_parm[5] = this->msg_back.msg_parm[5] | (0x01 << (i));
        }
    }
    for(i = 8;i<16;i++)
    {
        if(L_ONLINE == lock_team[i]->lock_power)
        {
            this->msg_back.msg_parm[4] = this->msg_back.msg_parm[4] | (0x01 << (i-8));
        }
    }
    for(i = 16;i<LOCK_DEVICE_NUM;i++)
    {
        if(L_ONLINE == lock_team[i]->lock_power)
        {
            this->msg_back.msg_parm[3] = this->msg_back.msg_parm[3] | (0x01 << (i-16));
        }
    }

    this->msg_back.msg_parm[6] = this->joint_ver[1];
    this->msg_back.msg_parm[7] = this->joint_ver[0];


    ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->msg_responce);

    printf("joint_3960.cpp responce is ret = %d\r\n",ret);

    if(ret == COMM_SUCCESS)
    {
        return COMM_SUCCESS;
    }
    else
    {
        return COMM_FAIL;
    }
}

//返回一个心跳包，无需返回任何东西
int joint_3960:: responce_heart_beat(void)
{
    int i,ret;
    int device_num;

    memset(&this->msg_back,0,sizeof(struct CMD_MSG));

    this->msg_back.device_type[0] = 0x00;
    this->msg_back.device_type[1] = 0x02;

    memcpy(this->msg_back.device_id,this->joint_id,7);

    this->msg_back.msg_type[0] = 0x00;
    this->msg_back.msg_type[1] = 0x03;

    //this->msg_back.msg_num[] = 0x00;

    this->msg_back.msg_info[0] = 0xFF;
    this->msg_back.msg_info[1] = 0xFF;

    device_num = 0;

    for(i = 0;i<8;i++)
    {
        if(L_ONLINE == lock_team[i]->lock_power)
        {
            device_num++;
            if(1 == lock_team[i]->lock_charging_state)
            {
                this->msg_back.msg_parm[7] = this->msg_back.msg_parm[7] | (0x01 << i);
            }

            if(1 == lock_team[i]->park_lot_state)
            {
                this->msg_back.msg_parm[3] = this->msg_back.msg_parm[3] | (0x01 << i);
            }
        }
        else
        {
            //该车位脱机
            //维持全零
        }
    }

    for(i = 8;i<16;i++)
    {
        if(L_ONLINE == lock_team[i]->lock_power)
        {
            device_num++;
            if(1 == lock_team[i]->lock_charging_state)
            {
                this->msg_back.msg_parm[6] = this->msg_back.msg_parm[6] | (0x01 << (i-8));
            }

            if(1 == lock_team[i]->park_lot_state)
            {
                this->msg_back.msg_parm[2] = this->msg_back.msg_parm[2] | (0x01 << (i-8));
            }
        }
        else
        {
            //该车位脱机
            //维持全零
        }
    }

    for(i = 16;i<LOCK_DEVICE_NUM;i++)
    {
        if(L_ONLINE == lock_team[i]->lock_power)
        {
            device_num++;
            if(1 == lock_team[i]->lock_charging_state)
            {
                this->msg_back.msg_parm[5] = this->msg_back.msg_parm[5] | (0x01 << (i-16));
            }

            if(1 == lock_team[i]->park_lot_state)
            {
                this->msg_back.msg_parm[1] = this->msg_back.msg_parm[1] | (0x01 << (i-16));
            }
        }
        else
        {
            //该车位脱机
            //维持全零
        }
    }

    //printf("this->msg_back.msg_parm[4] = 0x%x\r\n",this->msg_back.msg_parm[4]);

    this->msg_back.msg_parm[4] = 0x01 * device_num;

    //printf("this->msg_back.msg_parm[4] = 0x%x\r\n",this->msg_back.msg_parm[4]);

    //ret = this->n_m->send_tcp_heart_beat(&this->msg_back);
    //心跳需要更正为等待响应的
    //ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->msg_responce);
    ret = this->n_m->send_beat_msg(&this->msg_back,500,&this->msg_responce);

    if(ret == COMM_SUCCESS)
    {
        return COMM_SUCCESS;
    }
    else
    {

        return COMM_FAIL;
    }

}

int joint_3960::responce_success_back(void)
{
    int ret;

    memcpy(&this->msg_back,&this->msg_run,sizeof(struct CMD_MSG));

    memset(this->msg_back.msg_parm,0,8);
    this->msg_back.msg_type[1] = this->msg_run.msg_type[1] + 0x01;
    this->msg_back.msg_parm[0] = 0xF6;

    ret = this->n_m->send_tcp_responce(&this->msg_back);

    if(ret == COMM_SUCCESS)
    {
        return COMM_SUCCESS;
    }
    else
    {
        return COMM_FAIL;
    }
}

int joint_3960::responce_fail_back(void)
{
    int ret;

    memcpy(&this->msg_back,&this->msg_run,sizeof(struct CMD_MSG));

    this->msg_back.msg_type[1] = this->msg_run.msg_type[1] + 0x01;
    this->msg_back.msg_parm[0] = 0xF7;

    ret = this->n_m->send_tcp_responce(&this->msg_back);

    if(ret == COMM_SUCCESS)
    {
        return COMM_SUCCESS;
    }
    else
    {
        return COMM_FAIL;
    }
}


int joint_3960 ::responce_busy_back(void)
{
    int ret;

    //接收到的直接全部贴回去先
    memcpy(&this->msg_back,&this->msg_run,sizeof(struct CMD_MSG));

    this->msg_back.msg_type[1] = this->msg_run.msg_type[1] + 0x01;
    this->msg_back.msg_parm[0] = 0xF8;

    ret = this->n_m->send_tcp_responce(&this->msg_back);

    if(ret == COMM_SUCCESS)
    {
        return COMM_SUCCESS;
    }
    else
    {
        return COMM_FAIL;
    }
}
