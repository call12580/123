#include "global.h"
#include "net_march.h"
#include "lock_ptl.h"
#include "queue_com.h"

#ifndef PARK_LK_H
#define PARK_LK_H





class park_lk
{
public:
    park_lk(net_march * init_n_m,lock_ptl * init_l_p,unsigned char * init_joint_index,int init_485_add,int init_queue_id);
    ~park_lk();

    //tcp通讯功能对象
    net_march * n_m;

    //地锁485协议解释对象
    lock_ptl * l_p;



    //自检标志
    int self_init_flag;

    //锁的前7位节点编号
    unsigned char joint_3960_index[7];

    //使用中置锁的485地址直接作为，唯一码的部分
    unsigned char lock_index_addr;


    //通讯接受数据塞进队列,包括控制指令存储的地方
    queue_com * q_c;

    //对于响应控制类指令，待填充空间，
    struct CMD_MSG cmd_msg_back;

    //从队列中拷贝一条待执行的控制命令
    struct CMD_MSG msg_run;


    //待填充发送的事件类信息数据空间
    struct CMD_MSG event_msg_send;

    //专门用于存放异常事件发出后，返回的响应信息，由route_dis对象提供
    struct CMD_MSG event_msg_responce;






    //专门用于存放异常事件发出后，返回的响应信息，由route_dis对象提供
    struct CMD_MSG msg_responce;

    //待填充返回数据空间
    struct CMD_MSG msg_back;

    //待填充返回数据空间,busy响应专用
    struct CMD_MSG msg_busy_back;

    //标识该中置锁当前存在待执行的命令
    int cmd_ready;


    //中置锁是否在线 L_ONLINE L_OFFLINE
    int lock_power;

    //检测最近一次开始断线的时间，用于计算平时啥时候在进行一次是否恢复的检测
    long off_time_second;


    //以下四个位存储上一次数据，并和这次数据组合判断出7中车辆进出场事件
    bool R2001BIT0;
    bool R2001BIT6;
    bool R2001BIT5;
    bool R2001BIT4;

    //车锁故障事件是状态数据，应该要注意有跳变才发送一次
    bool R2002BIT0;
    bool R2002BIT1;
    bool R2002BIT2;
    bool R2002BIT3;
    bool R2002BIT4;
    bool R2002BIT5;
    bool R2002BIT6;

    int last_lock_power;


    //心跳包中需要带的数据，让锁自己填充，节点读取即可
    //计费状态
    int lock_charging_state;
    //车位状态
    int park_lot_state;



    //用于接受命令信息的数据空间单条即可，若超出一条，直接返回设备忙
    int lock_recv_cmd(unsigned char * r_buf);



    //构建针对服务器发出命令的几种响应数据包内容的构建
    //返回成功执行配置、控制。
    int make_success_ctl_pack(void);

    //返回成功执行读取
    int make_success_read_pack(unsigned char * data_from_485);


    //指令执行失败，包括配置、控制、查询类型指令
    int make_fail_responce_pack(void);


    //指令执行遇繁忙，配置、控制、查询类型指令
    int make_responce_busy_pack(void);


    //锁状态初始化
    int lock_init_check();


    //单个中置锁每次轮询到后需要进行的一个完整循环
    int lock_run_loop(void);


    //锁异常信息变动检测,并作最近的数据更新。
    int unusual_register_state_check(unsigned char reg_2001,unsigned char reg_2002);

    //联机、脱机事件检测,并作最近的数据更新。
    int unusual_power_state_check(void);






};

#endif // PARK_LK_H
