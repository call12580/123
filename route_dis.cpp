#include "route_dis.h"
#include "park_lk.h"
#include "joint_3960.h"





extern park_lk * lock_team[24];
extern joint_3960 * j_3;


route_dis::route_dis(net_march * init_n_m,unsigned char * init_node_p):n_m(init_n_m)
{
    memcpy(this->node_index,init_node_p,7);
}

int route_dis::loop_recv_msg()
{
    //printf("loop_recv_msg1\r\n");

    static unsigned int tcp_tfly_count = 0;
    fd_set net_connect_rfd;
    unsigned char recv_buf[32];
    int i,nfds,ret;
    struct timeval tv;


    while(NET_READY == n_m->net_state)
    {
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        FD_ZERO(&net_connect_rfd);
        FD_SET(n_m->fd_net,&net_connect_rfd);
        nfds = select(n_m->fd_net+1,&net_connect_rfd,NULL,NULL,&tv);
        if(nfds > 0)
        {
            memset(recv_buf,0,sizeof(recv_buf));
            ret = recv(n_m->fd_net,recv_buf,sizeof(recv_buf),0);
            cout << "route_dis.cpp recv data len = "  << ret << endl;
            if(ret <= 0)
            {
                cout << "route_dis.cpp tcp error " << endl;
                n_m->net_state = NET_OFFLINE;
                break;
            }
            else
            {
                //正常接收到有效数据
                if(32 == ret
                 && this->node_index[0] == recv_buf[4]
                 && this->node_index[1] == recv_buf[5]
                 && this->node_index[2] == recv_buf[6]
                 && this->node_index[3] == recv_buf[7]
                 && this->node_index[4] == recv_buf[8]
                 && this->node_index[5] == recv_buf[9]
                 && this->node_index[6] == recv_buf[10])

                {
                    if(0x02 == recv_buf[3])
                    {
                        //数据发向节点
                        j_3->joint_recv_cmd(recv_buf);
                    }
                    else if(0x01 == recv_buf[3])
                    {
                        //数据发向中置锁
                        for(i = 0;i< LOCK_DEVICE_NUM;i++)
                        {
                            if(lock_team[i]->lock_index_addr == recv_buf[11]
                            && L_ONLINE == lock_team[i]->lock_power)
                            {
                                //发往对应的中置锁对象
                                printf("route_dis.cpp get 05/07 msg for lock_team[%d]\r\n",i);
                                lock_team[i]->lock_recv_cmd(recv_buf);
                                break;
                            }
                        }

                        //if(i>=LOCK_DEVICE_NUM)
                        //{
                        //    printf("add problem 0x%x\r\n",recv_buf[11]);
                        //}

                    }
                    else
                    {
                        cout << "no such device!"<< recv_buf[3] << endl;
                    }
                }
                else
                {
                    cout << "recv some strange msg, discard" << endl;
                }
            }
        }
        else if(nfds == 0)
        {
            cout << "tcp/ip live idle message  " << tcp_tfly_count++ << endl;
            cout << "n_m.ack_fail_count  " << n_m->ack_fail_count << endl;
            /////////////////////////////////
            //ret = recv(n_m->fd_net,recv_buf,sizeof(recv_buf),MSG_PEEK);
            //if(ret <= 0)
            //{
            //    cout << "route_dis.cpp tcp error " << endl;
            //    n_m->net_state = NET_OFFLINE;
            //    break;
            //}
        }
        else
        {
            //内层的select出问题了，直接关闭现有链接后等待重连
            cout <<  "tcp/ip recv select error\r\n"<<endl;
            this->n_m->net_state = NET_OFFLINE;
            break;
        }
    }


    /*
    while(NET_READY == n_m->net_state)
    {
        memset(recv_buf,0,sizeof(recv_buf));
        cout << "route_dis.cpp prepare recv data " << tcp_tfly_count++ << endl;
        ret = recv(n_m->fd_net,recv_buf,sizeof(recv_buf),0);
        cout << "route_dis.cpp recv data len = "  << ret << endl;
        if(ret <= 0)
        {
            cout << "route_dis.cpp tcp error " << endl;
            n_m->net_state = NET_OFFLINE;
            break;
        }
        else
        {
            //正常接收到有效数据
            if(32 == ret
             && this->node_index[0] == recv_buf[4]
             && this->node_index[1] == recv_buf[5]
             && this->node_index[2] == recv_buf[6]
             && this->node_index[3] == recv_buf[7]
             && this->node_index[4] == recv_buf[8]
             && this->node_index[5] == recv_buf[9]
             && this->node_index[6] == recv_buf[10])
            {
                if(0x02 == recv_buf[3])
                {
                    //数据发向节点
                    j_3->joint_recv_cmd(recv_buf);
                }
                else if(0x01 == recv_buf[3])
                {
                    //数据发向中置锁
                    for(i = 0;i< LOCK_DEVICE_NUM;i++)
                    {
                        if(lock_team[i]->lock_index_addr == recv_buf[11]
                        && L_ONLINE == lock_team[i]->lock_power)
                        {
                            //发往对应的中置锁对象
                            printf("route_dis.cpp get 05/07 msg for lock_team[%d]\r\n",i);
                            lock_team[i]->lock_recv_cmd(recv_buf);
                            break;
                        }
                    }

                    //if(i>=LOCK_DEVICE_NUM)
                    //{
                    //    printf("add problem 0x%x\r\n",recv_buf[11]);
                    //}

                }
                else
                {
                    cout << "no such device!"<< recv_buf[3] << endl;
                }
            }
            else
            {
                cout << "recv some strange msg, discard" << endl;
            }
        }

    }
    */


    if(NET_READY != n_m->net_state)
    {
        usleep(500*1000);
    }

    return 0;
}
