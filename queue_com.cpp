#include "queue_com.h"

int queue_com::create_queue(void)
{
    if(0 > msgget(this->msg_id,IPC_CREAT|0666))
    {
         printf("failed to create msq\r\n");
         return -1;
    }
    else
    {
         printf("create new msg success\r\n");
         return 0;
    }

}


int queue_com::is_queue_exist(void)
{
    if(0 > msgget(this->msg_id,IPC_EXCL))
    {
       printf("msq not existed!\r\n");
       return -1;
    }
    else
    {
        printf("msq existed!\r\n");
        return 0;
    }
}


int queue_com::open_queue(void)
{
    int msgid;
    msgid = msgget(this->msg_id,IPC_EXCL);
    if(msgid < 0)
    {
       printf("msq not existed! errno=[%s]\n",strerror(errno));
       return -1;
    }
    else
    {
        this->fd = msgid;
    }
    return 0;
}


int queue_com::read_msg(int msg_type,struct msgstru * msgs)
{
    int ret_value;

    ret_value = msgrcv(this->fd,msgs,sizeof(struct msgstru),msg_type,IPC_NOWAIT);

    if(ret_value > 0)
    {
        return 0;
    }
    else
    {
        if(0 == strncmp("No message of desired",strerror(errno),21))
        {
            return -2;
        }
        else
        {
            printf("msgrcv error %s",strerror(errno));
            return -1;
        }

    }
}


int queue_com::send_msg(int msg_type,struct msgstru * msgs)
{
    int ret_value;
    //发给队列
    ret_value = msgsnd(this->fd,msgs,sizeof(struct msgstru),IPC_NOWAIT);
    if ( ret_value < 0 )
    {
       printf("msgsnd() write msg failed[%s]\n",strerror(errno));
       return -1;
    }
    return 0;
}


int queue_com::read_msg_qnum(void)
{
    this->msg_state();

    return this->msg_info.msg_qnum;
}


int queue_com::clear_msg(int msg_type)
{
    struct msgstru msgs;
    int ret_value;
    //查询命令缓冲队列，看看有没有发我的，有的话就抠出来
    while(1)
    {
        ret_value = msgrcv(this->fd,&msgs,sizeof(struct msgstru),msg_type,IPC_NOWAIT);

        if(ret_value < 0)
        {
            printf("finish clear!\r\n");
            return 0;
        }
    }
}


int queue_com::msg_state(void)
{
    if(0 == msgctl(this->fd,IPC_STAT,&this->msg_info))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}


int queue_com::rm_queue(void)
{
    if(0 == msgctl(this->fd,IPC_RMID,&this->msg_info))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}



int queue_com::q_p(void)
{
    printf("i am que init fd=%d\r\n",this->fd);
    return 0;
}

