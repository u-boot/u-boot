/**
 *
 * @file tftp.c
 *
 * @author   Logan Gunthorpe <logang@deltatee.com>
 *           Dirk Ziegelmeier <dziegel@gmx.de>
 *
 * @brief    Trivial File Transfer Protocol (RFC 1350)
 *
 * Copyright (c) Deltatee Enterprises Ltd. 2013
 * All rights reserved.
 *
 */

/*
 * Redistribution and use in source and binary forms, with or without
 * modification,are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Logan Gunthorpe <logang@deltatee.com>
 *         Dirk Ziegelmeier <dziegel@gmx.de>
 *
 */

/**
 * @defgroup tftp TFTP client/server
 * @ingroup apps
 *
 * This is simple TFTP client/server for the lwIP raw API.
 * You need to increase MEMP_NUM_SYS_TIMEOUT by one if you use TFTP!
 */

#include "lwip/apps/tftp_client.h"
#include "lwip/apps/tftp_server.h"

#if LWIP_UDP

#include "lwip/udp.h"
#include "lwip/timeouts.h"
#include "lwip/debug.h"

#define TFTP_DEFAULT_BLOCK_SIZE 512
#define TFTP_HEADER_LENGTH    4

#define TFTP_RRQ   1
#define TFTP_WRQ   2
#define TFTP_DATA  3
#define TFTP_ACK   4
#define TFTP_ERROR 5
#define TFTP_OACK  6

enum tftp_error {
  TFTP_ERROR_FILE_NOT_FOUND    = 1,
  TFTP_ERROR_ACCESS_VIOLATION  = 2,
  TFTP_ERROR_DISK_FULL         = 3,
  TFTP_ERROR_ILLEGAL_OPERATION = 4,
  TFTP_ERROR_UNKNOWN_TRFR_ID   = 5,
  TFTP_ERROR_FILE_EXISTS       = 6,
  TFTP_ERROR_NO_SUCH_USER       = 7,
  TFTP_ERROR_OPTION_NEGOTIATION = 8
};

#include <string.h>

struct tftp_req {
  ip_addr_t addr;
  u16_t port;
  u16_t opcode;
  enum tftp_transfer_mode mode;
  char* fname;
};

struct tftp_state {
  const struct tftp_context *ctx;
  void *handle;
  struct pbuf *last_data;
  struct udp_pcb *upcb;
  ip_addr_t addr;
  u16_t port;
  int timer;
  int last_pkt;
  u16_t blknum;
  u16_t blksize;
  u32_t tsize;
  u32_t put_tsize;
  u8_t retries;
  u8_t mode_write;
  u8_t tftp_mode;
  bool wait_oack;
  bool request_tsize;
  bool put_tsize_set;
};

static struct tftp_state tftp_state;
static struct tftp_req tftp_req;

static void tftp_tmr(void *arg);
static void tftp_req_tmr(void *arg);
static const char *mode_to_string(enum tftp_transfer_mode mode);
static u16_t payload_size(void);

static void
clear_req(void)
{
  ip_addr_set_any(0, &tftp_req.addr);
  tftp_req.port = 0;
  tftp_req.opcode = 0;
  free(tftp_req.fname);
  tftp_req.fname = NULL;
  tftp_req.mode = 0;

  sys_untimeout(tftp_req_tmr, NULL);
}

static void
close_handle(void)
{
  clear_req();

  tftp_state.port = 0;
  ip_addr_set_any(0, &tftp_state.addr);
  tftp_state.retries = 0;

  if (tftp_state.last_data != NULL) {
    pbuf_free(tftp_state.last_data);
    tftp_state.last_data = NULL;
  }

  sys_untimeout(tftp_tmr, NULL);

  if (tftp_state.handle) {
    tftp_state.ctx->close(tftp_state.handle);
    tftp_state.handle = NULL;
    LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("tftp: closing\n"));
  }
}

static struct pbuf*
init_packet(u16_t opcode, u16_t extra, size_t size)
{
  struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)(TFTP_HEADER_LENGTH + size), PBUF_RAM);
  u16_t* payload;

  if (p != NULL) {
    payload = (u16_t*) p->payload;
    payload[0] = PP_HTONS(opcode);
    payload[1] = lwip_htons(extra);
  }

  return p;
}

static err_t
send_request(const ip_addr_t *addr, u16_t port, u16_t opcode, const char* fname, const char* mode)
{
  size_t fname_length = strlen(fname)+1;
  size_t mode_length = strlen(mode)+1;
  size_t tsize_length = 0;
  size_t blksize_length = 0;
  char tsize[11];
  int blksize = tftp_state.blksize;
  struct pbuf* p;
  char* payload;
  err_t ret;

  if (tftp_state.request_tsize) {
    sprintf(tsize, "%u", tftp_state.tsize);
    tsize_length = strlen("tsize") + 1 + strlen(tsize) + 1;
  }

  if (blksize) {
    /* 'blksize\0'.\0" with . = 1 digit */
    blksize_length = strlen("blksize") + 1 + 1 + 1;
    while (blksize >= 10) {
      blksize /= 10;
      blksize_length++;
    }
  }

  p = init_packet(opcode, 0, fname_length + mode_length + blksize_length + tsize_length - 2);
  if (p == NULL) {
    return ERR_MEM;
  }

  payload = (char*) p->payload;
  MEMCPY(payload+2,              fname, fname_length);
  MEMCPY(payload+2+fname_length, mode,  mode_length);
  if (tftp_state.request_tsize)
    sprintf(payload+2+fname_length+mode_length, "tsize%c%s", 0, tsize);
  if (tftp_state.blksize)
    sprintf(payload+2+fname_length+mode_length+tsize_length, "blksize%c%d", 0, tftp_state.blksize);

  tftp_state.wait_oack = tsize_length || blksize_length;
  ret = udp_sendto(tftp_state.upcb, p, addr, port);
  pbuf_free(p);
  return ret;
}

static err_t
send_error(const ip_addr_t *addr, u16_t port, enum tftp_error code, const char *str)
{
  int str_length = strlen(str);
  struct pbuf *p;
  u16_t *payload;
  err_t ret;

  p = init_packet(TFTP_ERROR, code, str_length + 1);
  if (p == NULL) {
    return ERR_MEM;
  }

  payload = (u16_t *) p->payload;
  MEMCPY(&payload[2], str, str_length + 1);

  ret = udp_sendto(tftp_state.upcb, p, addr, port);
  pbuf_free(p);
  return ret;
}

static void
send_error_and_close(const ip_addr_t *addr, u16_t port,
                     enum tftp_error code, const char *str)
{
  send_error(addr, port, code, str);
  if (tftp_state.handle != NULL)
    tftp_state.ctx->error(tftp_state.handle, code, str, strlen(str));
  close_handle();
}

static err_t
send_ack(const ip_addr_t *addr, u16_t port, u16_t blknum)
{
  struct pbuf *p;
  err_t ret;

  p = init_packet(TFTP_ACK, blknum, 0);
  if (p == NULL) {
    return ERR_MEM;
  }

  ret = udp_sendto(tftp_state.upcb, p, addr, port);
  pbuf_free(p);
  return ret;
}

static err_t
resend_request(void)
{
  return send_request(&tftp_req.addr, tftp_req.port, tftp_req.opcode, tftp_req.fname, mode_to_string(tftp_req.mode));
}

static err_t
resend_data(const ip_addr_t *addr, u16_t port)
{
  err_t ret;
  struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, tftp_state.last_data->len, PBUF_RAM);
  if (p == NULL) {
    return ERR_MEM;
  }

  ret = pbuf_copy(p, tftp_state.last_data);
  if (ret != ERR_OK) {
    pbuf_free(p);
    return ret;
  }

  ret = udp_sendto(tftp_state.upcb, p, addr, port);
  pbuf_free(p);
  return ret;
}

static void
send_data(const ip_addr_t *addr, u16_t port)
{
  u16_t size = payload_size();
  u16_t *payload;
  int ret;

  if (tftp_state.last_data != NULL) {
    pbuf_free(tftp_state.last_data);
  }

  tftp_state.last_data = init_packet(TFTP_DATA, tftp_state.blknum, size);
  if (tftp_state.last_data == NULL) {
    return;
  }

  payload = (u16_t *) tftp_state.last_data->payload;

  ret = tftp_state.ctx->read(tftp_state.handle, &payload[2], size);
  if (ret < 0) {
    send_error_and_close(addr, port, TFTP_ERROR_ACCESS_VIOLATION,
                         "Error occurred while reading the file.");
    return;
  }

  pbuf_realloc(tftp_state.last_data, (u16_t)(TFTP_HEADER_LENGTH + ret));
  resend_data(addr, port);
}

static u16_t payload_size(void)
{
  if (tftp_state.blksize)
    return tftp_state.blksize;
  return TFTP_DEFAULT_BLOCK_SIZE;
}

static int
parse_u32_option(const char *str, u32_t *value)
{
  u32_t result = 0;

  if (*str == '\0')
    return 0;

  while (*str != '\0') {
    u8_t digit;

    if ((*str < '0') || (*str > '9'))
      return 0;
    digit = (u8_t)(*str - '0');
    if (result > (LWIP_UINT32_MAX - digit) / 10)
      return 0;
    result = result * 10 + digit;
    str++;
  }

  *value = result;
  return 1;
}

/* Read a complete NUL-terminated field, including across chained pbufs. */
static int
read_option_field(struct pbuf *p, u16_t *offset, char *field, size_t size)
{
  const char nul = 0;
  u16_t end = pbuf_memfind(p, &nul, 1, *offset);
  u16_t len;

  if ((end == 0xffff) || (end <= *offset))
    return 0;
  len = (u16_t)(end - *offset + 1);
  if (len > size)
    return 0;
  pbuf_copy_partial(p, field, len, *offset);
  *offset = (u16_t)(end + 1);
  return 1;
}

static int
parse_oack(struct pbuf *p, u32_t *blksize, u32_t *tsize)
{
  u16_t offset = 2;
  unsigned int seen = 0;

  *blksize = 0;
  *tsize = 0;
  while (offset < p->tot_len) {
    char option[16], value[11];
    u32_t number;

    if (!read_option_field(p, &offset, option, sizeof(option)) ||
        !read_option_field(p, &offset, value, sizeof(value)) ||
        !parse_u32_option(value, &number))
      return 0;

    if (!lwip_stricmp(option, "blksize")) {
      if ((seen & 1) || !tftp_state.blksize || (number < 8) ||
          (number > 65464) || (number > tftp_state.blksize))
        return 0;
      seen |= 1;
      *blksize = number;
    } else if (!lwip_stricmp(option, "tsize")) {
      if ((seen & 2) || !tftp_state.request_tsize ||
          ((tftp_state.mode_write == 0) && (number != tftp_state.tsize)))
        return 0;
      seen |= 2;
      *tsize = number;
    } else {
      /* RFC 2347 forbids acknowledging an option we did not request. */
      return 0;
    }
  }

  return seen != 0;
}

static void
tftp_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
  u16_t *sbuf = (u16_t *) p->payload;
  int opcode;

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(upcb);

  if (((tftp_state.port != 0) && (port != tftp_state.port)) ||
      (!ip_addr_isany_val(tftp_state.addr) && !ip_addr_eq(&tftp_state.addr, addr))) {
    send_error(addr, port, TFTP_ERROR_ACCESS_VIOLATION, "Only one connection at a time is supported");
    pbuf_free(p);
    return;
  }

  opcode = sbuf[0];

  tftp_state.last_pkt = tftp_state.timer;
  tftp_state.retries = 0;

  if (tftp_req.fname)
    clear_req();

  switch (opcode) {
    case PP_HTONS(TFTP_RRQ): /* fall through */
    case PP_HTONS(TFTP_WRQ): {
      const char tftp_null = 0;
      char filename[TFTP_MAX_FILENAME_LEN + 1];
      char mode[TFTP_MAX_MODE_LEN + 1];
      u16_t filename_end_offset;
      u16_t mode_end_offset;

      if (tftp_state.handle != NULL) {
        send_error(addr, port, TFTP_ERROR_ACCESS_VIOLATION, "Only one connection at a time is supported");
        break;
      }

      if ((tftp_state.tftp_mode & LWIP_TFTP_MODE_SERVER) == 0) {
        send_error(addr, port, TFTP_ERROR_ACCESS_VIOLATION, "TFTP server not enabled");
        break;
      }

      sys_timeout(TFTP_TIMER_MSECS, tftp_tmr, NULL);

      /* find \0 in pbuf -> end of filename string */
      filename_end_offset = pbuf_memfind(p, &tftp_null, sizeof(tftp_null), 2);
      if ((u16_t)(filename_end_offset - 1) > sizeof(filename)) {
        send_error(addr, port, TFTP_ERROR_ACCESS_VIOLATION, "Filename too long/not NULL terminated");
        break;
      }
      pbuf_copy_partial(p, filename, filename_end_offset - 1, 2);

      /* find \0 in pbuf -> end of mode string */
      mode_end_offset = pbuf_memfind(p, &tftp_null, sizeof(tftp_null), filename_end_offset + 1);
      if ((u16_t)(mode_end_offset - filename_end_offset) > sizeof(mode)) {
        send_error(addr, port, TFTP_ERROR_ACCESS_VIOLATION, "Mode too long/not NULL terminated");
        break;
      }
      pbuf_copy_partial(p, mode, mode_end_offset - filename_end_offset, filename_end_offset + 1);

      tftp_state.handle = tftp_state.ctx->open(filename, mode, opcode == PP_HTONS(TFTP_WRQ));
      tftp_state.blknum = 1;

      if (!tftp_state.handle) {
        send_error(addr, port, TFTP_ERROR_FILE_NOT_FOUND, "Unable to open requested file.");
        break;
      }

      LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("tftp: %s request from ", (opcode == PP_HTONS(TFTP_WRQ)) ? "write" : "read"));
      ip_addr_debug_print(TFTP_DEBUG | LWIP_DBG_STATE, addr);
      LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, (" for '%s' mode '%s'\n", filename, mode));

      ip_addr_copy(tftp_state.addr, *addr);
      tftp_state.port = port;

      if (opcode == PP_HTONS(TFTP_WRQ)) {
        tftp_state.mode_write = 1;
        send_ack(addr, port, 0);
      } else {
        tftp_state.mode_write = 0;
        send_data(addr, port);
      }

      break;
    }

    case PP_HTONS(TFTP_DATA): {
      int ret;
      u16_t blknum;

      if (tftp_state.handle == NULL) {
        send_error(addr, port, TFTP_ERROR_ACCESS_VIOLATION, "No connection");
        break;
      }

      if (tftp_state.mode_write != 1) {
        send_error(addr, port, TFTP_ERROR_ACCESS_VIOLATION, "Not a write connection");
        break;
      }

      blknum = lwip_ntohs(sbuf[1]);
      if (tftp_state.wait_oack) {
        /*
         * Data received while we are expecting an OACK for our tsize option.
         * This means the server doesn't support it, let's switch back to the
         * default block size.
         */
        tftp_state.tsize = 0;
        tftp_state.blksize = 0;
        tftp_state.wait_oack = false;
      }
      if (blknum == tftp_state.blknum) {
        pbuf_remove_header(p, TFTP_HEADER_LENGTH);

        ret = tftp_state.ctx->write(tftp_state.handle, p);
        if (ret < 0) {
          send_error_and_close(addr, port, TFTP_ERROR_ACCESS_VIOLATION,
                               "error writing file");
        } else {
          send_ack(addr, port, blknum);
        }

        if (p->tot_len < payload_size()) {
          close_handle();
        } else {
          tftp_state.blknum++;
        }
      } else if ((u16_t)(blknum + 1) == tftp_state.blknum) {
        /* retransmit of previous block, ack again (casting to u16_t to care for overflow) */
        send_ack(addr, port, blknum);
      } else {
        send_error(addr, port, TFTP_ERROR_UNKNOWN_TRFR_ID, "Wrong block number");
      }
      break;
    }

    case PP_HTONS(TFTP_ACK): {
      u16_t blknum;
      int lastpkt;

      if (tftp_state.handle == NULL) {
        send_error(addr, port, TFTP_ERROR_ACCESS_VIOLATION, "No connection");
        break;
      }

      if (tftp_state.mode_write != 0) {
        send_error(addr, port, TFTP_ERROR_ACCESS_VIOLATION, "Not a read connection");
        break;
      }

      blknum = lwip_ntohs(sbuf[1]);
      if ((blknum == 0) && (tftp_state.blknum == 0) && tftp_state.wait_oack) {
        /* The server ignored the options in our write request. */
        tftp_state.blksize = 0;
        tftp_state.wait_oack = false;
      }
      if (blknum != tftp_state.blknum) {
        send_error(addr, port, TFTP_ERROR_UNKNOWN_TRFR_ID, "Wrong block number");
        break;
      }

      lastpkt = 0;

      if (tftp_state.last_data != NULL) {
        lastpkt = tftp_state.last_data->tot_len != (payload_size() + TFTP_HEADER_LENGTH);
      }

      if (!lastpkt) {
        tftp_state.blknum++;
        send_data(addr, port);
      } else {
        close_handle();
      }

      break;
    }
    case PP_HTONS(TFTP_ERROR):
      if (tftp_state.handle != NULL) {
        pbuf_remove_header(p, TFTP_HEADER_LENGTH);
        tftp_state.ctx->error(tftp_state.handle, sbuf[1], (const char*)p->payload, p->len);
        close_handle();
      }
      break;
    case PP_HTONS(TFTP_OACK): {
      u32_t srv_blksize = 0;
      u32_t srv_tsize = 0;

      if (tftp_state.handle == NULL) {
        send_error(addr, port, TFTP_ERROR_ACCESS_VIOLATION, "No connection");
        break;
      }
      if (!parse_oack(p, &srv_blksize, &srv_tsize)) {
        send_error_and_close(addr, port, TFTP_ERROR_OPTION_NEGOTIATION,
                             "Invalid OACK");
        break;
      }
      tftp_state.wait_oack = false;
      /* Publish only after validating the entire option list. */
      tftp_state.blksize = (u16_t)srv_blksize;
      tftp_state.tsize = srv_tsize;
      if (tftp_state.mode_write == 0) {
        tftp_state.blknum = 1;
        send_data(addr, port);
      } else {
        send_ack(addr, port, 0);
      }
      break;
    }
    default:
      send_error(addr, port, TFTP_ERROR_ILLEGAL_OPERATION, "Unknown operation");
      break;
  }

  pbuf_free(p);
}

static void
tftp_tmr(void *arg)
{
  LWIP_UNUSED_ARG(arg);

  tftp_state.timer++;

  if (tftp_state.handle == NULL) {
    return;
  }

  sys_timeout(TFTP_TIMER_MSECS, tftp_tmr, NULL);

  if ((tftp_state.timer - tftp_state.last_pkt) > (TFTP_TIMEOUT_MSECS / TFTP_TIMER_MSECS)) {
    if ((tftp_state.last_data != NULL) && (tftp_state.retries < TFTP_MAX_RETRIES)) {
      LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("tftp: timeout, retrying\n"));
      resend_data(&tftp_state.addr, tftp_state.port);
      tftp_state.retries++;
    } else {
      LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("tftp: timeout\n"));
      close_handle();
    }
  }
}

static void
tftp_req_tmr(void *arg)
{
  if (tftp_state.handle == NULL) {
    return;
  }

  sys_timeout(TFTP_TIMER_MSECS, tftp_req_tmr, NULL);

  if (tftp_state.retries < TFTP_MAX_RETRIES) {
    LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("tftp: req timeout, retrying\n"));
    resend_request();
    tftp_state.retries++;
  } else {
    LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("tftp: req timeout\n"));
    tftp_state.ctx->error(tftp_state.handle, -1, "Request timeout", strlen("Request timeout"));
    close_handle();
  }
}

/**
 * Initialize TFTP client/server.
 * @param mode TFTP mode (client/server)
 * @param ctx TFTP callback struct
 */
err_t
tftp_init_common(u8_t mode, const struct tftp_context *ctx)
{
  err_t ret;

  /* LWIP_ASSERT_CORE_LOCKED(); is checked by udp_new() */
  struct udp_pcb *pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
  if (pcb == NULL) {
    return ERR_MEM;
  }

  if (mode == LWIP_TFTP_MODE_SERVER) {
    ret = udp_bind(pcb, IP_ANY_TYPE, TFTP_PORT);
    if (ret != ERR_OK) {
      udp_remove(pcb);
      return ret;
    }
  }

  tftp_state.handle    = NULL;
  tftp_state.port      = 0;
  tftp_state.ctx       = ctx;
  tftp_state.timer     = 0;
  tftp_state.last_data = NULL;
  tftp_state.upcb      = pcb;
  tftp_state.tftp_mode = mode;

  udp_recv(pcb, tftp_recv, NULL);

  return ERR_OK;
}

/** @ingroup tftp
 * Initialize TFTP server.
 * @param ctx TFTP callback struct
 */
err_t
tftp_init_server(const struct tftp_context *ctx)
{
  return tftp_init_common(LWIP_TFTP_MODE_SERVER, ctx);
}

/** @ingroup tftp
 * Initialize TFTP client.
 * @param ctx TFTP callback struct
 */
err_t
tftp_init_client(const struct tftp_context *ctx)
{
  return tftp_init_common(LWIP_TFTP_MODE_CLIENT, ctx);
}

/** @ingroup tftp
 * Get the transfer size used by the TFTP client. The server may
 * report zero in case this is unsupported.
 */
u32_t
tftp_client_get_tsize(void)
{
  return tftp_state.tsize;
}

/** @ingroup tftp
 * Set the block size to be used by the TFTP client. The server may choose to
 * accept a lower value.
 * @param blksize Block size in bytes
 */
void
tftp_client_set_blksize(u16_t blksize)
{
  tftp_state.blksize = blksize == TFTP_DEFAULT_BLOCK_SIZE ? 0 : blksize;
}

/** @ingroup tftp
 * Set the transfer size advertised by a TFTP write request.
 * @param tsize Transfer size in bytes
 */
void
tftp_client_set_tsize(u32_t tsize)
{
  tftp_state.put_tsize = tsize;
  tftp_state.put_tsize_set = true;
}

/** @ingroup tftp
 * Deinitialize ("turn off") TFTP client/server.
 */
void tftp_cleanup(void)
{
  LWIP_ASSERT("Cleanup called on non-initialized TFTP", tftp_state.upcb != NULL);
  udp_remove(tftp_state.upcb);
  close_handle();
  memset(&tftp_state, 0, sizeof(tftp_state));
}

static const char *
mode_to_string(enum tftp_transfer_mode mode)
{
  if (mode == TFTP_MODE_OCTET) {
    return "octet";
  }
  if (mode == TFTP_MODE_NETASCII) {
    return "netascii";
  }
  if (mode == TFTP_MODE_BINARY) {
    return "binary";
  }
  return NULL;
}

err_t
start_send_requests(const ip_addr_t *addr, u16_t port, u16_t opcode, const char* fname, enum tftp_transfer_mode mode)
{
  tftp_req.addr = *addr;
  tftp_req.port = port;
  tftp_req.opcode = opcode;
  tftp_req.fname = strdup(fname);
  tftp_req.mode = mode;
  if (!tftp_req.fname)
    return ERR_MEM;
  sys_timeout(TFTP_TIMER_MSECS, tftp_req_tmr, NULL);
  return resend_request();
}

err_t
tftp_get(void* handle, const ip_addr_t *addr, u16_t port, const char* fname, enum tftp_transfer_mode mode)
{
  LWIP_ERROR("TFTP client is not enabled (tftp_init)", (tftp_state.tftp_mode & LWIP_TFTP_MODE_CLIENT) != 0, return ERR_VAL);
  LWIP_ERROR("tftp_get: invalid file name", fname != NULL, return ERR_VAL);
  LWIP_ERROR("tftp_get: invalid mode", mode <= TFTP_MODE_BINARY, return ERR_VAL);

  tftp_state.handle = handle;
  tftp_state.blknum = 1;
  tftp_state.mode_write = 1; /* We want to receive data */
  tftp_state.tsize = 0;
  tftp_state.request_tsize = true;
  return start_send_requests(addr, port, TFTP_RRQ, fname, mode);
}

err_t
tftp_put(void* handle, const ip_addr_t *addr, u16_t port, const char* fname, enum tftp_transfer_mode mode)
{
  LWIP_ERROR("TFTP client is not enabled (tftp_init)", (tftp_state.tftp_mode & LWIP_TFTP_MODE_CLIENT) != 0, return ERR_VAL);
  LWIP_ERROR("tftp_put: invalid file name", fname != NULL, return ERR_VAL);
  LWIP_ERROR("tftp_put: invalid mode", mode <= TFTP_MODE_BINARY, return ERR_VAL);

  tftp_state.handle = handle;
  tftp_state.blknum = 0;
  tftp_state.mode_write = 0; /* We want to send data */
  tftp_state.tsize = tftp_state.put_tsize;
  tftp_state.request_tsize = tftp_state.put_tsize_set;
  return start_send_requests(addr, port, TFTP_WRQ, fname, mode);
}

#endif /* LWIP_UDP */
