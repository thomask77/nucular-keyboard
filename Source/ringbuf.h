/**
 * Lock-free single reader/single writer ring buffer
 * Copyright (C)2000-2015 Thomas Kindler <mail@t-kindler.de>
 *
 * 2014-11-22, tk:  rb_get_pointers for zero-copy operation
 * 2014-11-16, tk:  lock-free read_pos/write_pos implementation
 * 2010-09-12, tk:  block read and write functions
 * 2000-04-24, tk:  initial implementation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef RINGBUF_H
#define RINGBUF_H

#include <malloc.h>
#include <string.h>


struct ringbuf {
    char        *buf;
    size_t      buf_size;
    unsigned    read_pos;
    unsigned    write_pos;
};


#define RINGBUF(size) {     \
    .buf = (char[size]){ }, \
    .buf_size = size        \
};


typedef enum {
    RB_READ,
    RB_WRITE
} rb_ptrmode;


/**
 * Allocate a ring buffer.
 *
 * \note  can later be freed using free()
 *
 * \param   buf_size    requested buffer size
 * \return  pointer to allocated buffer or NULL
 */
static inline struct ringbuf *rb_alloc(size_t buf_size)
{
    struct ringbuf *rb;

    if (!(rb = (struct ringbuf *) malloc(sizeof(*rb) + buf_size) ))
        return NULL;

    rb->buf = (char*)rb + sizeof(*rb);
    rb->buf_size = buf_size;
    rb->read_pos = 0;
    rb->write_pos = 0;

    return rb;
}


/**
 * Get number of bytes available for reading.
 *
 * \param   rb      pointer to ringbuf struct
 * \return  number of bytes available
 */
static inline size_t rb_bytes_used(const struct ringbuf *rb)
{
    int ret = rb->write_pos - rb->read_pos;
    if (ret < 0)
        ret += rb->buf_size;
    
    return ret;
}


/**
 * Get number of bytes available for writing.
 *
 * \param   rb      pointer to ringbuf struct
 * \return  number of bytes available
 */
static inline size_t rb_bytes_free(const struct ringbuf *rb)
{
    // One byte must be left unused to differentiate
    // between full and empty buffers.
    //
    return rb->buf_size - rb_bytes_used(rb) - 1;
}


/**
 * Get pointers for read or write access.
 *
 * \note  ptr2 and len2 may be NULL if wrap-around is not desired.
 *
 * \param   rb      pointer to ringbuf struct
 * \param   mode    RB_READ or RB_WRITE
 * \param   nbyte   maximum number of bytes
 * \param   ptr1    where to store first-chunk pointer
 * \param   len1    where to store first-chunk length
 * \param   ptr2    where to store wrap-around pointer  (will be set equal to rb->buf)
 * \param   len2    where to store wrap-around length   (will receive 0 if not wrapped around)
 * \return  number of bytes available
 */
static inline size_t rb_get_pointers(
    const struct ringbuf *rb,
    rb_ptrmode mode, size_t nbyte,
    void **ptr1, size_t *len1,
    void **ptr2, size_t *len2
)
{
    const unsigned *pos;
	size_t max_bytes;

    if (mode == RB_READ) {
        pos = &rb->read_pos;
        max_bytes = rb_bytes_used(rb);
    }
    else {
        pos = &rb->write_pos;
        max_bytes = rb_bytes_free(rb);
    }

    if (nbyte > max_bytes)
        nbyte = max_bytes;

    *ptr1 = rb->buf + *pos;

    if (nbyte > rb->buf_size - *pos)
        *len1 = rb->buf_size - *pos;
    else
        *len1 = nbyte;

    if (ptr2 != NULL) {
        *ptr2 = rb->buf;
        *len2 = nbyte - *len1;
        return nbyte;
    }
    else {
        return *len1;
    }
}


/**
 * Commit read or write changes to the ring buffer.
 *
 * \note  nbyte is not checked for overflow.
 *        Use values derived from rb_get_pointers().
 * 
 * \param   rb      pointer to ringbuf struct
 * \param   mode    RB_READ or RB_WRITE
 * \param   nbyte   number of bytes to commit
 * \return  number of bytes committed
 */
static inline size_t rb_commit(struct ringbuf *rb, rb_ptrmode mode, size_t nbyte)
{
    unsigned *pos = (mode == RB_READ) ? &rb->read_pos : &rb->write_pos;

    unsigned new_pos = *pos + nbyte;
    if (new_pos >= rb->buf_size)
        new_pos -= rb->buf_size;

    *pos = new_pos;
    return nbyte;
}


/**
 * Read from ring buffer.
 *
 * \param   rb      pointer to ringbuf struct
 * \param   data    pointer to data bytes
 * \param   nbyte   maximum number of bytes to read
 * \return  number of bytes read
 */
static inline size_t rb_read(struct ringbuf *rb, void *data, size_t nbyte)
{
    void *ptr1, *ptr2;
    size_t len1, len2;
    
    nbyte = rb_get_pointers(rb, RB_READ, nbyte, &ptr1, &len1, &ptr2, &len2);

    memcpy(data, ptr1, len1);
    memcpy((char *)data + len1, ptr2, len2);

    return rb_commit(rb, RB_READ, nbyte);
}


/**
 * Write to ring buffer.
 *
 * \param   rb      pointer to ringbuf struct
 * \param   data    pointer to data bytes
 * \param   nbyte   maximum number of bytes to write
 * \return  number of bytes written
 */
static inline size_t rb_write(struct ringbuf *rb, const void *data, size_t nbyte)
{
    void *ptr1, *ptr2;
    size_t len1, len2;
    
    nbyte = rb_get_pointers(rb, RB_WRITE, nbyte, &ptr1, &len1, &ptr2, &len2);

    memcpy(ptr1, data, len1);
    memcpy(ptr2, (const char *)data + len1, len2);

    return rb_commit(rb, RB_WRITE, nbyte);
}


/**
 * Read single byte from ring buffer.
 *
 * \param   rb      pointer to ringbuf struct
 * \return  byte read or -1 if buffer was empty
 */
static inline int rb_getchar(struct ringbuf *rb)
{
    unsigned char *ptr;
    size_t len;

    if (rb_get_pointers(rb, RB_READ, 1, (void**)&ptr, &len, NULL, NULL)) {
        unsigned char uc = *ptr;
        rb_commit(rb, RB_READ, 1);
        return uc;
    }
    else {
        return -1;
    }
}


/**
 * Write single byte to ring buffer.
 *
 * \param   rb      pointer to ringbuf struct
 * \param   data    byte to write
 * \return  byte written or -1 if buffer was full
 */
static inline int rb_putchar(struct ringbuf *rb, int c)
{
    unsigned char *ptr;
    size_t len;

    if (rb_get_pointers(rb, RB_WRITE, 1, (void**)&ptr, &len, NULL, NULL)) {
        *ptr = c;
        rb_commit(rb, RB_WRITE, 1);
        return (unsigned char)c;
    }
    else {
        return -1;
    }
}


#endif
