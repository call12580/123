#include "global.h"

#ifndef QUEUE_COM_H
#define QUEUE_COM_H

struct msgstru
{
   long msgtype;
   unsigned char msgtext[26];
};

class queue_com
{
public:
    queue_com(int id_flag):msg_id(id_flag){}
    //~queue_com();

    int create_queue(void);
    int is_queue_exist(void);
    int open_queue(void);
    int rm_queue(void);
    int send_msg(int msg_type,struct msgstru * msgs);
    int read_msg(int msg_type,struct msgstru * msgs);
    int read_msg_qnum(void);
    int clear_msg(int msg_type);
    int msg_state(void);


    int q_p(void);

    struct msqid_ds  msg_info;
    int msg_id;


private:
    int fd;


};

#endif // QUEUE_COM_H
