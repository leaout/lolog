/**
* Created by chenly on 1/2/24.
* Description
**/


#ifndef LOLOG_CIRCLEBUFFER_H
#define LOLOG_CIRCLEBUFFER_H

#include <cstring>

namespace lolog {
    using namespace std;

    class CircleBuffer {
    public:
        CircleBuffer() {
            m_base_address = nullptr;
            m_buffer_size = 0;
            memset(m_buffer_name, 0, sizeof(m_buffer_name));
            free();
        }

        CircleBuffer(size_t size, const char *buff_name) {
            m_base_address = nullptr;
            m_buffer_size = 0;
            alloc(size, buff_name);
        }

        ~CircleBuffer() {
            free();
        }

        bool alloc(size_t size, const char *buff_name) {
            strcpy(m_buffer_name, buff_name);

            if (m_buffer_size != size) {
                free();
                m_buffer_size = size;
                m_base_address = new char[m_buffer_size + 32];
                m_tail_address = m_base_address + m_buffer_size + 1;
            }

            reset();

            return (m_base_address != NULL);
        }

        void free() {
            delete[] m_base_address;

            m_base_address = NULL;
            m_tail_address = NULL;
            m_buffer_size = 0;

            reset();
        }

        void reset() {
            if (m_base_address != NULL) {
                m_head = m_base_address;
                m_tail = m_base_address;
            } else {
                m_head = NULL;
                m_tail = NULL;
            }

            m_is_writing_data = false;
            m_is_reading_data = false;
        }

        size_t get_max_len() {
            return m_buffer_size;
        }

        size_t get_used_buff_len() {
            char *head = (m_head < m_tail_address) ? (char *) m_head : m_base_address;
            char *tail = (m_tail < m_tail_address) ? (char *) m_tail : m_base_address;

            char *vtail = (tail >= head) ? tail : (tail + m_buffer_size + 1);
            return size_t(vtail - head);
        }

        size_t get_available_buff_len() {
            char *head = (m_head < m_tail_address) ? (char *) m_head : m_base_address;
            char *tail = (m_tail < m_tail_address) ? (char *) m_tail : m_base_address;

            if (head == tail)
                return m_buffer_size;

            char *vtail = (tail >= head) ? tail : (tail + m_buffer_size + 1);
            return m_buffer_size - (vtail - head);
        }

        size_t get_continuous_available_buff_len() {
            char *head = (m_head < m_tail_address) ? (char *) m_head : m_base_address;
            char *tail = (m_tail < m_tail_address) ? (char *) m_tail : m_base_address;

            if (head > tail)
                return (head - tail) - 1;

            if ((tail == head) && (head == m_base_address))
                return m_buffer_size;

            return size_t(m_tail_address - tail);
        }

        size_t get_continuous_data_buff_len() {
            char *head = (m_head < m_tail_address) ? (char *) m_head : m_base_address;
            char *tail = (m_tail < m_tail_address) ? (char *) m_tail : m_base_address;

            if (tail >= head)
                return tail - head;

            return size_t(m_tail_address - head);
        }

        bool is_continuous_data_len(size_t count) {
            return m_head + count < m_tail_address;
        }

        size_t read(char *dest, long count) {
            if (get_used_buff_len() < count)
                return 0;

            if (m_is_reading_data || count <= 0)
                return 0;

            char *head = (m_head < m_tail_address) ? (char *) m_head : m_base_address;
            char *tail = (m_tail < m_tail_address) ? (char *) m_tail : m_base_address;

            if (head == tail)
                return 0;

            m_is_reading_data = true;

            size_t copy_len = 0;
            if (head < tail) {
                while (head < tail && count--) {
                    *dest++ = *head++;
                    ++copy_len;
                }
            } else {
                while (head < m_tail_address && count--) {
                    *dest++ = *head++;
                    ++copy_len;
                }

                if (count > 0) {
                    head = m_base_address;
                    while (head < tail && count--) {
                        *dest++ = *head++;
                        ++copy_len;
                    }
                }
            }

            m_head = head;
            m_is_reading_data = false;
            return copy_len;
        }


        bool write(const char *src, long count) {
            if (m_is_writing_data || count <= 0)
                return false;

            if (count > get_available_buff_len())
                return false;

            m_is_writing_data = true;

            char *tail = (m_tail < m_tail_address) ? (char *) m_tail : m_base_address;

            if (tail + count < m_tail_address) {
                while (count--)
                    *tail++ = *src++;
            } else {
                while (tail < m_tail_address && count--)
                    *tail++ = *src++;

                if (count > 0) {
                    tail = m_base_address;
                    while (count--)
                        *tail++ = *src++;
                }
            }

            m_tail = tail;

            m_is_writing_data = false;
            return true;
        }

        char *get_read_position() {
            return (char *) m_head;
        }

        char *get_write_position() {
            return (char *) m_tail;
        }

        bool inc_data_len(long count) {
            if (count <= 0)
                return false;

            m_tail += count;
            if (m_tail >= m_tail_address)
                m_tail -= m_buffer_size + 1;

            return true;
        }

        bool dec_data_len(long count) {
            if (count <= 0)
                return false;

            size_t data_len = get_used_buff_len();
            if (count > data_len)
                count = data_len;

            m_head += count;
            if (m_head >= m_tail_address)
                m_head -= m_buffer_size + 1;

            return true;
        }

        char operator[](size_t index) {
            if ((m_head + index) >= m_tail_address)
                return m_base_address[index - (m_tail_address - m_head)];

            return m_head[index];
        }

        int memcmp(const void *buf, long count, size_t offset = 0) {
            if (!count)
                return (0);

            char *buf1 = (m_head < m_tail_address) ? (char *) m_head : m_base_address;
            char *buf2 = (char *) buf;

            if (offset) {
                buf1 += offset;
                if (buf1 >= m_tail_address)
                    buf1 -= m_buffer_size + 1;
            }

            bool more_cmp = false;
            while ((buf1 < m_tail_address) && --count && (more_cmp = (*buf1 == *buf2))) {
                ++buf1;
                ++buf2;
            }

            if (more_cmp && (count > 0)) {
                buf1 = m_base_address;

                while (--count && (*buf1 == *buf2)) {
                    ++buf1;
                    ++buf2;
                }
            }

            return (*((unsigned char *) buf1) - *((unsigned char *) buf2));
        }

        void *memcpy(void *dst, long count, size_t offset = 0) {
            char *src = (char *) (m_head < m_tail_address) ? (char *) m_head : m_base_address;
            char *dest = (char *) dst;

            if (offset) {
                src += offset;
                if (src >= m_tail_address)
                    src -= m_buffer_size + 1;
            }

            while (src < m_tail_address && count--)
                *dest++ = *src++;

            if (count > 0) {
                src = m_base_address;
                while (src < m_tail_address && count--)
                    *dest++ = *src++;
            }

            return (dst);
        }

        size_t strstr(const char *str) {
            size_t nSearchStrLen = strlen(str);
            size_t nDataStrLen = get_used_buff_len();
            if (nDataStrLen < nSearchStrLen)
                return -1;

            size_t nSearchCount = nDataStrLen - nSearchStrLen;
            for (size_t i = 0; i <= nSearchCount; ++i) {
                if (memcmp(str, nSearchStrLen, i) == 0)
                    return i;
            }

            return -1;
        }


    public:
        char m_buffer_name[64];

    protected:
        char *m_base_address;
        char *m_tail_address;
        size_t m_buffer_size;

        volatile char *m_head;
        volatile char *m_tail;

        volatile bool m_is_writing_data;
        volatile bool m_is_reading_data;
    };
}
#endif //LOLOG_CIRCLEBUFFER_H
