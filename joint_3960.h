#include "global.h"
#include "net_march.h"

#ifndef JOINT_3960_H
#define JOINT_3960_H





class joint_3960
{
public:
    joint_3960(net_march * init_n_m,unsigned char * init_node_index);

    ~joint_3960();

    //tcp通讯功能对象
    net_march * n_m;

    //待填充命令空间
    struct CMD_MSG msg_run;

    //专门用于存放异常事件发出后，返回的响应信息，由route_dis对象填充提供
    struct CMD_MSG msg_responce;

    //待填充返回数据空间
    struct CMD_MSG msg_back;

    //最近的一次心跳触发的时间(系统开机时间算起)
    unsigned int last_beat_tick;

    //心跳触发的周期长度,单位sec
    unsigned int beat_loop;

    //是否进行心跳标志
    int need_beat_flag;

    //系统启动标志
    int boot_up_event_flag;

    //节点id,调整为大段在前的数据类型，方便直接拷贝给数据包
    unsigned char joint_id[7];

    //节点软件版本号
    unsigned char joint_ver[2];

    //标识该中置锁当前是否存在待执行的命令
    int cmd_ready;

    //route_dis中用于向节点对象分发数据包的方法
    int joint_recv_cmd(unsigned char * r_buf);

    //接受server主动指令后返回的响应，即只对 0x05 和 0x07类型响应
    int responce_success_back(void);
    int responce_fail_back(void);
    int responce_busy_back(void);

    //单个节点每次轮询到后需要进行的一个完整循环
    int joint_run_loop();

    //节点接受的server控制指令比较简单，无需先转为485命令再进行函数安排执行，直接解析加执行。
    int make_execute_ctl_cmd(unsigned char * msg_info_p,unsigned char * msg_parm_p);

    //产生一次节点启动事件，并需要等待响应
    int responce_boot_up(void);

    //产生一次心跳事件，无需响应
    int responce_heart_beat(void);

};

#endif // JOINT_3960_H
