/**
* Copyright (c) 2024 chenly. All rights reserved.
* Created by chenly on 1/2/24.
* Description
**/


#ifndef LOLOG_MSG_H
#define LOLOG_MSG_H

#include <iostream>
#include <list>
#include <string.h>

#include "CircleBuffer.h"

namespace lolog {

    using namespace std;
#pragma pack(4)
    struct MsgDef {
        unsigned int flag;     // msg head flag 0xFFFFFFFF
        int type;              // msg type
        int len;               // msg len
        char data[0];
    };
#pragma pack()

    class LyMsg {
    public:
        LyMsg() {
            m_using_tail = false;
            m_check_msg_version = true;
        }


        ~LyMsg() {}

        bool read(CircleBuffer *circle_buffer, MsgDef *msg, size_t max_expected_len) {

            size_t nDataLen = 0;
            const int head_len = sizeof(MsgDef);

            nDataLen = circle_buffer->get_used_buff_len();
            if (nDataLen <= head_len) {
                msg->flag = 0;
                msg->type = 0;
                msg->len = 0;

                return false;
            }

            if (0 == circle_buffer->memcmp(&MSG_TAG, sizeof(int))) {

                circle_buffer->memcpy((char *) msg, head_len);
                if (msg->len <= 0 || msg->len > max_expected_len - head_len) {
                    circle_buffer->dec_data_len(1);
                    return false;
                }

                if (circle_buffer->get_used_buff_len() < head_len + msg->len) {
                    return false;
                }

                if (m_check_msg_version) {
                    m_using_tail = is_advanced_msg(circle_buffer, msg);
                    m_check_msg_version = false;
                }

                if (m_using_tail && !is_advanced_msg(circle_buffer, msg)) {
                    msg->flag = MSG_TAG;
                    msg->type = 0;
                    msg->len = abandon_error_data(circle_buffer);
                    return false;
                }

                circle_buffer->read((char *) msg, head_len + msg->len);

                if (m_using_tail)
                    msg->len -= sizeof(int);

                return true;
            } else {
                msg->flag = MSG_TAG;
                msg->type = 0;
                msg->len = abandon_error_data(circle_buffer);
            }

            return false;
        }

        bool write(const char *data, int type, size_t count, CircleBuffer *circle_buffer) {
            MsgDef msg_head = {MSG_TAG, type, int(count + sizeof(int))};

            if (circle_buffer->get_available_buff_len() < msg_head.len + sizeof(MsgDef))
                return false;

            circle_buffer->write((char *) &msg_head, sizeof(MsgDef));
            circle_buffer->write(data, msg_head.len - sizeof(int));
            int nTail = get_tail_value(msg_head.len);
            circle_buffer->write((char *) &nTail, sizeof(int));

            return true;
        }

        size_t abandon_error_data(CircleBuffer *circle_buffer) {

            char data[1024] = {0};
            char *p = data;
            size_t error_len = 0;

            size_t all_data_len = circle_buffer->get_used_buff_len();
            if (all_data_len > 1024)
                all_data_len = 1024;

            size_t data_len = 0;
            do {
                if (error_len < 1000) {
                    circle_buffer->memcpy(p, 1);
                    ++p;
                    ++error_len;
                } else {
                    break;
                }

                circle_buffer->dec_data_len(1);
                data_len = circle_buffer->get_used_buff_len();
            } while (data_len > 3 && 0 != circle_buffer->memcmp(&MSG_TAG, sizeof(int)));

            return error_len;
        }

    protected:
        inline static int get_tail_value(int msg_len) {
            return msg_len ^ 0xaaaaaaaa;        // ff get oppset value
        }

    private:
        static bool is_advanced_msg(MsgDef *pmsg) {
            return *(int *) (pmsg->data + pmsg->len - sizeof(int)) == get_tail_value(pmsg->len);
        }

        static bool is_advanced_msg(CircleBuffer *circle_buffer, MsgDef *pmsg) {
            int tail = get_tail_value(pmsg->len);
            int tail_pos = sizeof(MsgDef) + pmsg->len - sizeof(int);
            return (circle_buffer->memcmp((char *) &tail, sizeof(int), tail_pos) == 0);
        }

        int discard_error_data(CircleBuffer *circle_buffer) {

            char data[1024] = {0};
            char *p = data;
            int error_len = 0;

            size_t all_data_len = circle_buffer->get_used_buff_len();
            if (all_data_len > 1024)
                all_data_len = 1024;

            size_t data_len = 0;
            do {
                if (error_len < 1000) {
                    circle_buffer->memcpy(p, 1);
                    ++p;
                    ++error_len;
                } else {
                    break;
                }

                circle_buffer->dec_data_len(1);
                data_len = circle_buffer->get_used_buff_len();
            } while (data_len > 3 && 0 != circle_buffer->memcmp(&MSG_TAG, sizeof(int)));

            return error_len;
        }

    public:
        unsigned int MSG_TAG = 0xFFFFFFFF;


    protected:
        bool m_using_tail;
        bool m_check_msg_version;
    };
}
#endif //LOLOG_MSG_H
