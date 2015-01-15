/**
 * $Id: snowleo_sw.h 2014-5
 *
 * @brief SNOWleo SDR sw Module.
 *
 * @Author Eric <gaocc@v3best.com>
 *
 * (c) V3best  http://www.zingsoc.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */
#ifndef __SW_SET__
#define __SW_SET__
struct msg_st
{
	long int msg_type;
	unsigned int param;
};
enum {
	E_GNURADIO_MODE = 1,
	E_MATLAB_MODE,
};
enum {
	E_STATE_GNURADIO,
	E_STATE_MATLAB,
	E_STATE_UNINIT,

};
enum {
	E_CLIENT_DISCONNECT = 0,
	E_CLIENT_CONNECT,
};


#define  XDMA_MAGIC              'D'
#define  XDMA_IOCTL_RESET                _IO(XDMA_MAGIC, 1)
#define  XDMA_START_WAIT_COMPLETE        _IO(XDMA_MAGIC, 2)
#define  XDMA_IOCTL_CONFIG                _IO(XDMA_MAGIC, 3)


#define  LENGTH_MIN           8
#define  LENGTH_MAX           4088
#define  PACKET_SIZE          1024


struct global_param {
	int sockfd;
	int clifd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_len;
	pthread_t thread_fd;
	pthread_mutex_t mutex_lock;
	pthread_cond_t cond;
};

struct axi_dma_config_info {
	int channel;
	unsigned long mem_addr;
	unsigned long length;
	unsigned int cycle;
	unsigned int reset_type;
};

#define MaxSize 2048
#define BUFFER_MAX_SIZE (PACKET_SIZE*8)
struct ring_buf
{
	unsigned char *data[MaxSize];
	int front,rear;
};
#endif/*__SW_SET__*/
