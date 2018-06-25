#include "park_lk.h"




park_lk :: park_lk(net_march * init_n_m,lock_ptl * init_l_p,
                   unsigned char * init_joint_index,int init_485_add,int init_queue_id)
                  :n_m(init_n_m),l_p(init_l_p)
{
    memcpy(this->joint_3960_index,init_joint_index,7);
    this->lock_index_addr = 0x01 * init_485_add;
    this->cmd_ready = 0;
    this->lock_power = L_OFFLINE;
    this->last_lock_power = L_OFFLINE;
    this->self_init_flag = 0;

    this->q_c = new queue_com(init_queue_id);
}




int park_lk::lock_recv_cmd(unsigned char * r_buf)
{
    int i;
    struct msgstru temp_msg;
    struct PACK_MSG * pack_head = (struct PACK_MSG *) r_buf;


    //准备进行临时的转储，在合适的时间点进行存储

    //查看系统事件

    //转存至一个大数据格式空间

    //这个大数据空间，在恰当的时间点可以存储到一个文件

    //在存储之间要查看文件的大小，若太大则明显是异常了，要注意







    //将server被动发出的响应包和主动发出的指令包分开
    if(0x02 == pack_head->frame_msg.msg_type[1])
    {
        //printf("1\r\n");
        //memcpy(&this->msg_responce,&pack_head->frame_msg,26);
        memcpy(&this->event_msg_responce,&pack_head->frame_msg,sizeof(struct CMD_MSG));

        //printf("park_lk.cpp lock_recv_cmd \r\n");
        //for(i = 0;i<32;i++)
        //{
        //    printf("0x%x ",r_buf[i]);
        //}
        //printf("\r\n");
    }
    else  //剩余0x05 0x07
    {
        //构造数据格式、塞入队列
        temp_msg.msgtype = L_CMD_MSG;
        memcpy(temp_msg.msgtext,&pack_head->frame_msg,sizeof(struct CMD_MSG));

        this->q_c->send_msg(L_CMD_MSG,&temp_msg);

        /*
        if(CMD_IDLE == this->cmd_ready)
        {
            //printf("2\r\n");
            memcpy(&this->msg_run,&pack_head->frame_msg,26);
            this->cmd_ready = CMD_BUSY;
        }
        else
        {
            //printf("3\r\n");
            memcpy(&this->msg_run,&pack_head->frame_msg,26);
            this->make_responce_busy_pack();
        }
        */

        //printf("park_lk.cpp lock_recv_cmd \r\n");
        //for(i = 0;i<32;i++)
        //{
        //    printf("0x%x ",r_buf[i]);
        //}
        //printf("\r\n");
    }
    return 0;
}


//初始检测车锁状态
int park_lk::lock_init_check()
{
    struct sysinfo s_t;
    unsigned char respone_from_485[8];
    unsigned char reg_2001,reg_2002;
    int cmd_ret;

    //模拟tcp报文
    unsigned char msg_info_row[2];
    unsigned char msg_parm_row[2];

    msg_info_row[0] = 0xF0;
    msg_info_row[1] = 0x1A;

    this->l_p->make_res_cmd(this->lock_index_addr,msg_info_row,msg_parm_row);
    cmd_ret = this->l_p->execute_res_cmd(msg_info_row,this->lock_index_addr,respone_from_485);

    //printf("cmd_ret = %d\r\n",cmd_ret);
    if(COMM_SUCCESS == cmd_ret)  //查询成功，重新认为机器恢复了
    {
        //printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
        //启动时，还要把异常标志位处理好

        reg_2001 = respone_from_485[7];
        this->R2001BIT0 = reg_2001 & 0x01;
        this->R2001BIT6 = (reg_2001 >> 6) & 0x01;
        this->R2001BIT5 = (reg_2001 >> 5) & 0x01;
        this->R2001BIT4 = (reg_2001 >> 4) & 0x01;

        reg_2002 = respone_from_485[6];
        this->R2002BIT0 = (reg_2002) & 0x01;
        this->R2002BIT1 = (reg_2002 >> 1) & 0x01;
        this->R2002BIT2 = (reg_2002 >> 2) & 0x01;
        this->R2002BIT3 = (reg_2002 >> 3) & 0x01;
        this->R2002BIT4 = (reg_2002 >> 4) & 0x01;
        this->R2002BIT5 = (reg_2002 >> 5) & 0x01;
        this->R2002BIT6 = (reg_2002 >> 6) & 0x01;


        this->lock_power = L_ONLINE;
        this->last_lock_power = L_ONLINE;
    }
    else if(COMM_TIMEOUT == cmd_ret)    //仍然处于无响应状态
    {
        //更新掉电时间
        sysinfo(&s_t);
        this->off_time_second = s_t.uptime;
        this->lock_power = L_OFFLINE;
        this->last_lock_power = L_OFFLINE;
    }
    else
    {
        //可能接受到的是COMM_FAIL或COMM_UNKNOWN
        sysinfo(&s_t);
        this->off_time_second = s_t.uptime;
        this->lock_power = L_OFFLINE;
        this->last_lock_power = L_OFFLINE;
    }

    //提示本锁的上电初检完成了
    if(0 == this->self_init_flag) this->self_init_flag = 1;


    return 0;
}



int park_lk::lock_run_loop()
{
    //printf("lock_run_loop1\r\n");

    struct sysinfo s_t;
    //long off_time_second = 0;
    unsigned char respone_from_485[8];
    unsigned char reg_2001,reg_2002;

    int i;
    static unsigned int ct = 0;

    //模拟tcp报文
    unsigned char msg_info_row[2];
    unsigned char msg_parm_row[2];

    int msg_ret;
    struct msgstru temp_msgs;

    int cmd_ret;

    //printf("lock_run_loop\r\n");

    //每周期状态检测
    //1：电源，脱机、联机事件（寄存器数据无直接相关）
    this->unusual_power_state_check();


    //每周期设备恢复联机检测
    if(L_OFFLINE == this->lock_power)
    {
        //当设备运行时脱机状态后，将会定时约30s周期,进行一次查看，是否恢复了
        sysinfo(&s_t);
        if(s_t.uptime - this->off_time_second >= 45)
        {
            msg_info_row[0] = 0xF0;
            msg_info_row[1] = 0x1A;

            this->l_p->make_res_cmd(this->lock_index_addr,msg_info_row,msg_parm_row);
            cmd_ret = this->l_p->execute_res_cmd(msg_info_row,this->lock_index_addr,respone_from_485);

            //printf("cmd_ret = %d\r\n",cmd_ret);
            if(COMM_SUCCESS == cmd_ret)  //查询成功，重新认为机器恢复了
            {
                //printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
                reg_2001 = respone_from_485[7];
                this->R2001BIT0 = reg_2001 & 0x01;
                this->R2001BIT6 = (reg_2001 >> 6) & 0x01;
                this->R2001BIT5 = (reg_2001 >> 5) & 0x01;
                this->R2001BIT4 = (reg_2001 >> 4) & 0x01;

                reg_2002 = respone_from_485[6];
                this->R2002BIT0 = (reg_2002) & 0x01;
                this->R2002BIT1 = (reg_2002 >> 1) & 0x01;
                this->R2002BIT2 = (reg_2002 >> 2) & 0x01;
                this->R2002BIT3 = (reg_2002 >> 3) & 0x01;
                this->R2002BIT4 = (reg_2002 >> 4) & 0x01;
                this->R2002BIT5 = (reg_2002 >> 5) & 0x01;
                this->R2002BIT6 = (reg_2002 >> 6) & 0x01;

                this->cmd_ready = CMD_IDLE;

                this->q_c->clear_msg(L_CMD_MSG);

                this->lock_power = L_ONLINE;


            }
            else if(COMM_TIMEOUT == cmd_ret)    //仍然处于无响应状态
            {
                //更新掉电时间
                sysinfo(&s_t);
                this->off_time_second = s_t.uptime;
                this->lock_power = L_OFFLINE;
            }
            else
            {
                //可能接受到的是COMM_FAIL或COMM_UNKNOWN
                this->lock_power = L_OFFLINE;
            }
        }


        //无论是否恢复，检测这次时不会进入正常模式中的，要等下一次了，
        //但至少在线的状态已经恢复了
        return 0;
    }


    //printf("...\r\n");

    //每周期检查命令数据中，是否存在待执行的命令，0x05，0x07
    //查看队列有无指令，有则拷贝后执行
    msg_ret = this->q_c->read_msg_qnum();
    if(msg_ret != 0)
    {
        this->q_c->read_msg(L_CMD_MSG,&temp_msgs);
        memcpy(&this->msg_run,temp_msgs.msgtext,sizeof(struct CMD_MSG));
        if(0x05 == this->msg_run.msg_type[1])   //指令配置类
        {
            this->l_p->make_ctl_cmd(this->lock_index_addr,this->msg_run.msg_info,this->msg_run.msg_parm);
            cmd_ret = this->l_p->execute_ctl_cmd();

            if(COMM_SUCCESS == cmd_ret)
            {
                //执行成功，告知tcp返回执行成功数据包
                this->make_success_ctl_pack();
            }
            else if(COMM_TIMEOUT == cmd_ret)
            {
                //锁没有响应，且满三次，认为是掉线了。
                //并直接跳出该函数，不再继续进行下去。
                this->make_fail_responce_pack();
                sysinfo(&s_t);
                this->off_time_second = s_t.uptime;
                this->lock_power = L_OFFLINE;
                //return 0;
            }
            else{
                //执行失败,返回的是COMM_UNKNOWN或者COMM_FAIL
                //告知tcp返回执行失败数据包
                this->make_fail_responce_pack();
            }
        }
        else if(0x07 == this->msg_run.msg_type[1]) //查询类
        {
            this->l_p->make_res_cmd(this->lock_index_addr,this->msg_run.msg_info,this->msg_run.msg_parm);
            cmd_ret = this->l_p->execute_res_cmd(this->msg_run.msg_info,this->lock_index_addr,respone_from_485);

            if(COMM_SUCCESS == cmd_ret)  //查询成功，并tcp返回数据
            {
                //告知tcp返 具体数据数据包，传入数据在该函数内部直接拷贝到this->msg_back.msg_parm段，所以
                //传入前就要格式正确了
                this->make_success_read_pack(respone_from_485);
            }
            else if(COMM_TIMEOUT == cmd_ret)
            {
                //锁没有响应，且满三次，认为是掉线了，并想tcp返回失败信息？
                //并直接跳出该函数，不再继续进行下去。
                this->make_fail_responce_pack();
                sysinfo(&s_t);
                this->off_time_second = s_t.uptime;
                this->lock_power = L_OFFLINE;
                //return 0;
            }
            else{
                //执行失败,返回的是COMM_UNKNOWN或者COMM_FAIL
                //告知tcp返回执行失败数据包
                this->make_fail_responce_pack();
            }
        }
        else
        {
            cout << "strange this->msg_run.msg_type in lock_run_loop " << endl;
        }

        //this->cmd_ready = CMD_IDLE;
    }


    //只在设备在线时进行，即假如是在执行上面的指令中发生设备短线，这里也不会进行周期状态检测了
    if(L_ONLINE == this->lock_power)
    {
        //1：485寄存器获得事件，模拟tcp查询寄存器 2001H
        msg_info_row[0] = 0xF0;
        msg_info_row[1] = 0x1A;

        this->l_p->make_res_cmd(this->lock_index_addr,msg_info_row,msg_parm_row);
        cmd_ret = this->l_p->execute_res_cmd(msg_info_row,this->lock_index_addr,respone_from_485);

        if(COMM_SUCCESS == cmd_ret)  //查询成功，并tcp返回数据
        {
            //检测中置锁事件，并更新参考位的数据
            this->unusual_register_state_check(respone_from_485[7],respone_from_485[6]);

            //待填充的心跳包数据，每查询均进行填充
            this->lock_charging_state = ((respone_from_485[7]>>1) & 0x01);
            this->park_lot_state = (respone_from_485[7] & 0x01);

        }
        else if(COMM_TIMEOUT == cmd_ret)
        {
            //锁超时没有响应，且满n次，认为是掉线了
            sysinfo(&s_t);
            off_time_second = s_t.uptime;
            this->lock_power = L_OFFLINE;
        }
        else{

        }
    }


    ///////////////////////////////////////////////////

    return 0;
}

int park_lk :: unusual_power_state_check(void)
{
    int ret;

    memset(&this->msg_back,0,sizeof(struct CMD_MSG));

    //返回数据包都需要填充的共同部分
    this->msg_back.device_type[0] = 0x00;
    this->msg_back.device_type[1] = 0x01;

    memcpy(this->msg_back.device_id,this->joint_3960_index,7);
    this->msg_back.device_id[7] = this->lock_index_addr;


    if(this->lock_power != last_lock_power && L_ONLINE == this->lock_power)
    {
        //0ah
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x02;

        this->msg_back.msg_parm[7] = 0x0B;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }


    if(this->lock_power != last_lock_power && L_OFFLINE == this->lock_power)
    {
        //0bh
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x02;

        this->msg_back.msg_parm[7] = 0x0A;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    last_lock_power = this->lock_power;

    return 0;

}


int park_lk :: unusual_register_state_check(unsigned char reg_2001,unsigned char reg_2002)
{
    int i;
    int ret;
	
    //使用2001H的 2byte更新这几个bool值，
    // 然后和last的比较
    bool temp_R2001BIT0;
    bool temp_R2001BIT6;
    bool temp_R2001BIT5;
    bool temp_R2001BIT4;


    //车锁故障事件是状态数据，应该要注意有跳变才发送一次
    bool temp_R2002BIT0;
    bool temp_R2002BIT1;
    bool temp_R2002BIT2;
    bool temp_R2002BIT3;
    bool temp_R2002BIT4;
    bool temp_R2002BIT5;
    bool temp_R2002BIT6;


    //返回数据包都需要填充的共同部分
    this->msg_back.device_type[0] = 0x00;
    this->msg_back.device_type[1] = 0x01;

    memcpy(this->msg_back.device_id,this->joint_3960_index,7);
    this->msg_back.device_id[7] = this->lock_index_addr;

    memset(this->msg_back.msg_parm,0,8);
	
    ////////////////////////////////////////////////////////////////////////////////
    //使用2001H的,low byte
    temp_R2001BIT0 = reg_2001 & 0x01;
    temp_R2001BIT6 = (reg_2001 >> 6) & 0x01;
    temp_R2001BIT5 = (reg_2001 >> 5) & 0x01;
    temp_R2001BIT4 = (reg_2001 >> 4) & 0x01;

    //printf("R2001 = 0x%x\r\n",reg_2001);
    //printf("R2001BIT6=%d R2001BIT5=%d  R2001BIT4=%d R2001BIT0=%d\r\n",R2001BIT6,R2001BIT5,R2001BIT4,R2001BIT0);
    //printf("tR2001BIT6=%d tR2001BIT5=%d  tR2001BIT4=%d tR2001BIT0=%d\r\n",temp_R2001BIT6,temp_R2001BIT5,temp_R2001BIT4,temp_R2001BIT0);

    //车辆到位//01H
    if(temp_R2001BIT0 != this->R2001BIT0 && 1 == temp_R2001BIT0)
    {
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x01;

        this->msg_back.msg_parm[7] = 0x01;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    //车辆驶离//02H
    if(temp_R2001BIT0 != this->R2001BIT0 && 0 == temp_R2001BIT0)
    {
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x01;

        this->msg_back.msg_parm[7] = 0x02;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    //开始降锁//03H
    if(temp_R2001BIT6 != this->R2001BIT6 && 1 == temp_R2001BIT6)
    {
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x01;

        this->msg_back.msg_parm[7] = 0x03;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);

    }

    //降锁结束//04H
    if(temp_R2001BIT6 != this->R2001BIT6 && 0 == temp_R2001BIT6)
    {
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x01;

        this->msg_back.msg_parm[7] = 0x04;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    //开始升锁//05H
    if(temp_R2001BIT5 != this->R2001BIT5 && 1 == temp_R2001BIT5)
    {
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x01;

        this->msg_back.msg_parm[7] = 0x05;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    //升锁结束//06H
    if(temp_R2001BIT5 != this->R2001BIT5 && 0 == temp_R2001BIT5)
    {
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x01;

        this->msg_back.msg_parm[7] = 0x06;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    //车辆离位超时//07H
    if(temp_R2001BIT4 != this->R2001BIT4 && 1 == temp_R2001BIT4)
    {
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x01;

        this->msg_back.msg_parm[7] = 0x07;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    //数据更新
    this->R2001BIT0 = temp_R2001BIT0;
    this->R2001BIT6 = temp_R2001BIT6;
    this->R2001BIT5 = temp_R2001BIT5;
    this->R2001BIT4 = temp_R2001BIT4;
    ////////////////////////////////////////////////////////////////////////////////

	
    ///////////////////////////////////////////////////////////////////////////////
    temp_R2002BIT0 = (reg_2002) & 0x01;
    temp_R2002BIT1 = (reg_2002 >> 1) & 0x01;
    temp_R2002BIT2 = (reg_2002 >> 2) & 0x01;
    temp_R2002BIT3 = (reg_2002 >> 3) & 0x01;
    temp_R2002BIT4 = (reg_2002 >> 4) & 0x01;
    temp_R2002BIT5 = (reg_2002 >> 5) & 0x01;
    temp_R2002BIT6 = (reg_2002 >> 6) & 0x01;


    if(temp_R2002BIT2 != this->R2002BIT2 && 1 == temp_R2002BIT2)
    {
        //01H
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x02;

        this->msg_back.msg_parm[7] = 0x01;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    if(temp_R2002BIT1 != this->R2002BIT1 && 1 == temp_R2002BIT1)
    {
        //02H
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x02;

        this->msg_back.msg_parm[7] = 0x02;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    if(temp_R2002BIT0 != this->R2002BIT0 && 1 == temp_R2002BIT0)
    {
        //03H
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x02;

        this->msg_back.msg_parm[7] = 0x03;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    if((0 == temp_R2002BIT0 && 0 == temp_R2002BIT1 && 0 == temp_R2002BIT2)
     &&(1 == this->R2002BIT0 || 1 == this->R2002BIT1 || 1 == this->R2002BIT2))
    {
        //04H
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x02;

        this->msg_back.msg_parm[7] = 0x04;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    if(temp_R2002BIT3 != this->R2002BIT3 && 1 == temp_R2002BIT3)
    {
        //05H
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x02;

        this->msg_back.msg_parm[7] = 0x05;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    if(temp_R2002BIT4 != this->R2002BIT4 && 1 == temp_R2002BIT4)
    {
        //06H
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x02;

        this->msg_back.msg_parm[7] = 0x06;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    if((0 == temp_R2002BIT3 && 0 == temp_R2002BIT4)
    &&(1 == this->R2002BIT3 || 1 == this->R2002BIT4))
    {
        //07H
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x02;

        this->msg_back.msg_parm[7] = 0x07;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    if(temp_R2002BIT5 != this->R2002BIT5 && 1 == temp_R2002BIT5)
    {
        //08H
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x02;

        this->msg_back.msg_parm[7] = 0x08;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }

    if(temp_R2002BIT5 != this->R2002BIT5 && 0 == temp_R2002BIT5)
    {
        //09H
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x02;

        this->msg_back.msg_parm[7] = 0x09;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }



    if(temp_R2002BIT6 != this->R2002BIT6 && 1 == temp_R2002BIT6)
    {
        //0cH
        this->msg_back.msg_type[0] = 0x00;
        this->msg_back.msg_type[1] = 0x01;

        //this->msg_back.msg_num[] = 0x00;

        this->msg_back.msg_info[0] = 0x20;
        this->msg_back.msg_info[1] = 0x02;

        this->msg_back.msg_parm[7] = 0x0C;

        ret = this->n_m->send_unusual_msg(&this->msg_back,500,&this->event_msg_responce);
    }



    this->R2002BIT0 = temp_R2002BIT0;
    this->R2002BIT1 = temp_R2002BIT1;
    this->R2002BIT2 = temp_R2002BIT2;
    this->R2002BIT3 = temp_R2002BIT3;
    this->R2002BIT4 = temp_R2002BIT4;
    this->R2002BIT5 = temp_R2002BIT5;
    this->R2002BIT6 = temp_R2002BIT6;


    return 0;
}




int park_lk ::make_success_ctl_pack(void)
{
    int ret;
    //协议构建，并使用n_m直接返回


    //接收到的直接全部贴回去先
    memcpy(&this->msg_back,&this->msg_run,sizeof(struct CMD_MSG));

    memset(this->msg_back.msg_parm,0,8);

    this->msg_back.msg_type[1] = this->msg_run.msg_type[1] + 0x01 ;
    this->msg_back.msg_parm[0] = 0xF6;

    ret = this->n_m->send_tcp_responce(&this->msg_back);

    return ret;
}


//
int park_lk::make_success_read_pack(unsigned char * data_from_485)
{
    int ret;
    //协议构建，并使用n_m直接返回

    //接收到的直接全部贴回去先
    memcpy(&this->msg_back,&this->msg_run,sizeof(struct CMD_MSG));

    memset(this->msg_back.msg_parm,0,8);

    this->msg_back.msg_type[1] = this->msg_run.msg_type[1] + 0x01 ;
    memcpy(this->msg_back.msg_parm,data_from_485,8);

    ret = this->n_m->send_tcp_responce(&this->msg_back);

    return ret;
}


int park_lk ::make_fail_responce_pack(void)
{
    int ret;
    //协议构建，并使用n_m直接返回

    //接收到的直接全部贴回去先
    memcpy(&this->msg_back,&this->msg_run,sizeof(struct CMD_MSG));

    memset(this->msg_back.msg_parm,0,8);

    this->msg_back.msg_type[1] = this->msg_run.msg_type[1] + 0x01 ;
    this->msg_back.msg_parm[0] = 0xF7;

    ret = this->n_m->send_tcp_responce(&this->msg_back);

    return ret;
}


/*
int park_lk ::make_responce_busy_pack(void)
{
    int ret;
    //协议构建，并使用n_m直接返回

    //接收到的直接全部贴回去先
    memcpy(&this->msg_back,&this->msg_run,sizeof(struct CMD_MSG));

    this->msg_back.msg_type[1] = this->msg_run.msg_type[1] + 0x01 ;
    this->msg_back.msg_parm[0] = 0xF8;

    ret = this->n_m->send_tcp_responce(&this->msg_back);

    return ret;
}
*/


int park_lk ::make_responce_busy_pack(void)
{
    int ret;
    //协议构建，并使用n_m直接返回

    //接收到的直接全部贴回去先
    memcpy(&this->msg_busy_back,&this->msg_run,sizeof(struct CMD_MSG));
    memset(this->msg_busy_back.msg_parm,0,8);

    this->msg_busy_back.msg_type[1] = this->msg_run.msg_type[1] + 0x01 ;
    this->msg_busy_back.msg_parm[0] = 0xF8;

    ret = this->n_m->send_tcp_responce(&this->msg_busy_back);

    return ret;
}





