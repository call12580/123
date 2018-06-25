#include "global.h"
#include "lock_485.h"

#ifndef LOCK_PTL_H
#define LOCK_PTL_H




class lock_ptl
{
public:
    lock_ptl(lock_485 * init_l_4_p):l_4_p(init_l_4_p){
        device_type = 0x01;
    }
    ~lock_ptl();

    lock_485 * l_4_p;

    unsigned char device_type;

    //将返回的485数据转为待返回的8字节的tcp数据
    unsigned char msg_485_to_tcp[8];


    //待执行的由tcp映射成的485指令
    unsigned char modbus_cmd_pool[5][8];
    //待执行指令数量，部分tcp映射成的485指令不止一个
    int cmd_pool_depth;


    //构造 控制/设置 命令(可认为就是tcp协议转485协议)
    int make_ctl_cmd(unsigned char device_add,
                     unsigned char * msg_info_p ,
                     unsigned char * msg_parm_p);

    //构造 查询 命令
    //传入业务编码和业务参数，查询成功，并将1字节实际的485返回的数据，
    int make_res_cmd(unsigned char device_add,
                     unsigned char * msg_info_p,
                     unsigned char * msg_parm_p);


    //执行485控制命令，并返回执行结果,无tcp数据返回，只返回结果提供给park_lk使用
    //返回COMM_SUCCESS：执行成功，且接受到正确的响应
    //返回COMM_BUSY：执行成功，但接受到从机繁忙的响应（在延迟上锁时立即解锁等情况出现）
    //返回COMM_UNKNOWN;执行成功，但接受到了奇怪的信息
    //返回COMM_TIMEOUT;n次执行，从机无响应
    //返回COMM_FAIL：串口设备的fd层面的写入或select的fd层面的失败
    int execute_ctl_cmd(void);


    //执行485查询命令，并返回执行结果和8字节的tcp数据
    //返回值含义同上
    int execute_res_cmd(unsigned char * msg_info_p,unsigned char lock_addr,unsigned char * responce_tcp_pack_formate);


    //检查自有事件
    //int make_state_check(unsigned char * responce_tcp);


    //检测是否存活
    //int check_live(void);




};

#endif // LOCK_PTL_H
