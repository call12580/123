#include "global.h"

#include "park_lk.h"
#include "route_dis.h"
#include "joint_3960.h"
#include "lock_485.h"
#include "lock_ptl.h"
#include "net_march.h"
#include "queue_com.h"


//not makefile ,simple compile cmd
//arm-none-linux-gnueabi-g++ main.cpp joint_3960.cpp lock_485.cpp lock_ptl.cpp net_march.cpp park_lk.cpp route_dis.cpp queue_com.cpp -o park -lpthread

//2018.05.10
//实际进行了和车锁的485通讯对接，通过在park_lk::lock_run_loop()中插入模拟tcp数据，目前完成控制命令的发送和响应
//接收，数据的读取，还进行了控制命令中繁忙响应的触发，快速不间断读取每个读取周期约为150ms，
//并记录1000ms周期10次读取2001，接着升锁，再立刻1000ms周期10次读取2001，接着降锁，再立刻1000ms周期10次读取2001

//还需要确认的部分：
//某个地址的机器丢失后的恢复上线，上线后丢失后的剔除。
//进一步整理了一下 execute_ctl_cmd 返回值的含义和从机的丢机恢复等问题。
//待测，关键认为：等待485响应时，多次超时即认为从机掉线

//2018.05.15
//测试和服务段的连接，短线，重连等机制，目前之测试了上电启动事件及其响应和周期心跳


//2018.05.17
//增加节点id的识别，调整两个锁loop循环现查看谁有控制命令就先执行.
//锁设备脱机状态时，锁不接受tcp任何数据控制。
//锁设备脱机后恢复连接，清空可能意外残存的cmd_ready。

//2018.05.25
//心跳包也增加了响应，用于应对客户端无法快速识别服务端异常关闭的情况。
//通讯协议由轻微更改
//并对通过心跳识别到服务器关闭时（需要心跳周期*2来识别到），对当前连接的关闭，设置为清空发送缓冲，
//省的服务端只是拔掉网线后，程序没关，客户端识别到短线后，插回网线，重连时，把原来的缓存接着发送。
//不过目前服务端拔网线，而客户断还未 心跳周期*2来识别到时，再次连接，这个时候怎么说呢，我好像有时看到也被抛弃了，
//有时看到缓冲里的又被发出了。

//2018.05.30
//重测了一下485的单个寄存器响应速度，约6000次/240s，25ms单次响应。

//2018.05.30
//锁这部分的命令接受和处理做了调整，加入了队列，不再进行一个命令限制，
//锁的事件的发出相响应识别不变，由此命令的执行响应和事件的发出接收响应
//完全隔离开来了，待测。。。


park_lk * lock_team[24];
route_dis * r_d;
joint_3960 * j_3;
lock_485 * l_4P1;
lock_485 * l_4P2;
lock_ptl * l_p1;
lock_ptl * l_p2;
net_march * tcp_n_m;

//数据的接受后分发线程
void * msg_loop(void * arg)
{
    pthread_detach(pthread_self());

    while(1)
    {
        //printf("msg_loop\r\n");
        r_d->loop_recv_msg();
        //sleep(1);
        //usleep(30*1000);
    }
}


//节点设备线程
void * dev_loop_node(void * arg)
{
    pthread_detach(pthread_self());

    while(1)
    {
        //printf("dev_loop_node\r\n");
        //printf("++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
        //轮询节点是否存在心跳或异常数据
        j_3->joint_run_loop();

        //节点没这么多事情，让出来给中置锁线程
        usleep(50*1000);
    }
}

//设备0~11,轮询线程
void * dev_loop_port_1(void * arg)
{
    pthread_detach(pthread_self());

    int i,j,msg_ret;


    for(i = 0;i < 12;i++)
    {
        lock_team[i]->lock_init_check();
    }


    while(1)
    {
        //printf("dev_loop_port_1\r\n");
        //printf("--------------------------------------------------\r\n");
        for(i = 0;i<12;i++)
        {
            lock_team[i]->lock_run_loop();

            for(j = 0;j<12;j++)
            {
                if(L_ONLINE == lock_team[j]->lock_power)
                {
                    msg_ret = lock_team[j]->q_c->read_msg_qnum();
                    if(msg_ret != 0)
                    {
                        lock_team[j]->lock_run_loop();
                        //这个地方的break将会导致控制命令只对一个执行一次后就会
                        //跳回原来的轮转过此，这可以使得其他有事件的锁不会耽搁的太久
                        //先这样看看，不行再把break删掉。
                        break;
                    }
                }
            }
        }
        /*
        //轮询12台设备，
        //是否存在控制信息，是否存在查询信息，是否存在异常信息，是否存在心跳信息，
        for(i = 0;i < 12;i++)
        {
            lock_team[i]->lock_run_loop();
        }
        */
        //usleep(30*1000);
    }
}

//设备12~23,轮询线程
void * dev_loop_port_2(void * arg)
{
    pthread_detach(pthread_self());

    int i,j,msg_ret;

    for(i = 12;i < 24;i++)
    {
        lock_team[i]->lock_init_check();
    }


    while(1)
    {
        for(i = 12;i<24;i++)
        {
            lock_team[i]->lock_run_loop();

            for(j = 12;j<24;j++)
            {
                if(L_ONLINE == lock_team[j]->lock_power)
                {
                    msg_ret = lock_team[j]->q_c->read_msg_qnum();
                    if(msg_ret != 0)
                    {
                        lock_team[j]->lock_run_loop();
                        //这个地方的break将会导致控制命令只对一个执行一次后就会
                        //跳回原来的轮转过此，这可以使得其他有事件的锁不会耽搁的太久
                        //先这样看看，不行再把break删掉。
                        break;
                    }
                }

                /*
                if(CMD_BUSY == lock_team[j]->cmd_ready
                && L_ONLINE == lock_team[j]->lock_power)
                {
                    lock_team[j]->lock_run_loop();
                }
                */
            }
        }
        /*
        //轮询12台设备，
        //是否存在控制信息，是否存在查询信息，是否存在异常信息，是否存在心跳信息，
        for(i = 12;i < 24;i++)
        {
            lock_team[i]->lock_run_loop();
        }
        */
        //usleep(30*1000);
    }
}



int main(int argc,char * argv[])
{
    cout << "Hello World!" << endl;

    pthread_t dev_loop_node_id,msg_loop_id;
    pthread_t dev_loop_port_1_id,dev_loop_port_2_id;
    int i;
    int msg_ret;

    printf("%s  %s\r\n",argv[1], argv[2]);

    //设定板子默认的node_id
    unsigned char node3960[7];
    node3960[0] = 0x01;
    node3960[1] = 0x02;
    node3960[2] = 0x03;
    node3960[3] = 0x04;
    node3960[4] = 0x05;
    node3960[5] = 0x06;
    node3960[6] = 0x07;


    //创建一个tcp通讯对象
    tcp_n_m = new net_march(argv[1],argv[2]);


    //创建两个485通讯对象
    l_4P1 = new lock_485(9600,"/dev/ttySP2");   //fit to RS485_2
    l_4P2 = new lock_485(9600,"/dev/ttySP3");   //fit to RS485_1
    l_4P1->serial_485_open();
    l_4P2->serial_485_open();
    l_4P1->serial_485_flush_read_buf();
    l_4P2->serial_485_flush_read_buf();

    //创建一个地锁控制业务(地锁的控制协议解析)对象
    l_p1 = new lock_ptl(l_4P1);
    l_p2 = new lock_ptl(l_4P2);

    //创建一个分发对象
    r_d = new route_dis(tcp_n_m,node3960);

    //创建一个节点对象
    j_3 = new joint_3960(tcp_n_m,node3960);

    //创建24台中置锁对象
    for(i = 0;i<12;i++)
    {
        lock_team[i] = new park_lk(tcp_n_m,l_p1,j_3->joint_id,i+1,LOCK_CMD_MSGKEY[i]);
        printf("cmd queue %d prepare\r\n",i);
        msg_ret = lock_team[i]->q_c->is_queue_exist();
        if(0 == msg_ret)
        {
            lock_team[i]->q_c->rm_queue();
        }
        lock_team[i]->q_c->create_queue();
        lock_team[i]->q_c->open_queue();
        printf("cmd queue %d finish prepare\r\n",i);
    }


    for(i = 12;i<24;i++)
    {
        lock_team[i] = new park_lk(tcp_n_m,l_p2,j_3->joint_id,i+1,LOCK_CMD_MSGKEY[i]);
        printf("cmd queue %d prepare\r\n",i);
        msg_ret = lock_team[i]->q_c->is_queue_exist();
        if(0 == msg_ret)
        {
            lock_team[i]->q_c->rm_queue();
        }
        lock_team[i]->q_c->create_queue();
        lock_team[i]->q_c->open_queue();
        printf("cmd queue %d finish prepare\r\n",i);
    }

    //从配置文件中获取node_id
    unsigned int node[7];
    FILE * fp;
    if(0 == access("./3960Lid",0))//检查文件是否存在,不存在就创建
    {
        fp = fopen("./3960Lid","r");
        fscanf(fp,"id=%x.%x.%x.%x.%x.%x.%x\r\n",&node[0],&node[1],&node[2],&node[3],&node[4],&node[5],&node[6]);
        fclose(fp);
        r_d->node_index[0]  = (unsigned char) node[0];
        r_d->node_index[1]  = (unsigned char) node[1];
        r_d->node_index[2]  = (unsigned char) node[2];
        r_d->node_index[3]  = (unsigned char) node[3];
        r_d->node_index[4]  = (unsigned char) node[4];
        r_d->node_index[5]  = (unsigned char) node[5];
        r_d->node_index[6]  = (unsigned char) node[6];

        j_3->joint_id[0]  = (unsigned char) node[0];
        j_3->joint_id[1]  = (unsigned char) node[1];
        j_3->joint_id[2]  = (unsigned char) node[2];
        j_3->joint_id[3]  = (unsigned char) node[3];
        j_3->joint_id[4]  = (unsigned char) node[4];
        j_3->joint_id[5]  = (unsigned char) node[5];
        j_3->joint_id[6]  = (unsigned char) node[6];


        printf("%02x.%02x.%02x.%02x.%02x.%02x.%02x\r\n",node[0],node[1],node[2],node[3],node[4],node[5],node[6]);
    }


    printf("%02x.%02x.%02x.%02x.%02x.%02x.%02x\r\n",r_d->node_index[0],
                                                    r_d->node_index[1],
                                                    r_d->node_index[2],
                                                    r_d->node_index[3],
                                                    r_d->node_index[4],
                                                    r_d->node_index[5],
                                                    r_d->node_index[6]);




    for(i = 0;i< LOCK_DEVICE_NUM;i++)
    {
        //printf("add = 0x%x\r\n",lock_team[i]->lock_index_addr);
    }

    while(0)
    {
        usleep(1000*1000);
        l_4P1->serial_485_write_test();

        //l_4P2->serial_485_write_test();

        //l_4P1->rtu_write();

        usleep(1000*1000);

        return 0;

    }

    //printf("main 1\r\n");
    //分四个线程

    //1:分发对象处理 tcp数据分发用
    pthread_create(&msg_loop_id,NULL,&msg_loop,NULL);


    //2:轮询节点
    pthread_create(&dev_loop_node_id,NULL,&dev_loop_node,NULL);


    //3: 485端口1 中置锁用,设备0~11，add 1~12
    pthread_create(&dev_loop_port_1_id,NULL,&dev_loop_port_1,NULL);


    //4: 485端口2 中置锁用,设备12~23，add 13~24
    //pthread_create(&dev_loop_port_2_id,NULL,&dev_loop_port_2,NULL);

    //printf("main 2\r\n");
    //3:主进程负责维护tcp和其他的主循环任务
    //struct sysinfo s_t;
    while(1)
    {
        /*
        sysinfo(&s_t);
        if(s_t.uptime %5 == 0)
        {
            printf("********************************************************************\r\n");
            printf("tcp_n_m->net_state=%d  tcp_n_m->fd_net = %d\r\n",tcp_n_m->net_state,tcp_n_m->fd_net);

            printf("l_4P1 fd = %d\r\n",l_4P1->fd);
            printf("l_4P2 fd = %d\r\n",l_4P2->fd);

            for(i = 0;i<12;i++)
            {
                printf("  lock_power[%d]=%d  cmd_ready[%d]=%d",i,lock_team[i]->lock_power ,i,lock_team[i]->cmd_ready);
            }
            printf("\r\n");

            printf("********************************************************************\r\n");

        }
        */

        //维护tcp
        if(NET_READY != tcp_n_m->net_state)
        {
            tcp_n_m->repair_connect();
        }
        usleep(1000*1000);

    }


    return 0;
}


