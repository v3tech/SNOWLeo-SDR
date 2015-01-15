/**
 * $Id: snowleosdr_rx_main.c 2015-1
 *
 * @brief SNOWleo SDR RX main Module.
 *
 * @Author Eric <gaocc@v3best.com>
 *
 * (c) V3best  http://www.zingsoc.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */
#define _GNU_SOURCE 1

#include <signal.h>
#include <arpa/inet.h>
#include "snowleo_hw.h"
#include "snowleo_sw.h"
#include <linux/ioctl.h>
#include <sys/msg.h>

struct global_param adc_param;

int client_connect;
unsigned int adc_mode = E_GNURADIO_MODE;
unsigned int adc_state = E_STATE_UNINIT;
unsigned int cmd_buff[2];
int fd;
pthread_t thread_adc, thread_send;
unsigned char *adc_mem_addr ,*dma_reg_addr_wr;
struct ring_buf *cb_adc;
char buffer[16384];
/**
 * Function declaration
 */
void *thread_adc_func(void *arg);
void *thread_sendto_func(void *arg);
int recv_msg(unsigned int *param);
static void sig_pipe(int signo);
static void sig_usr1(int signo);


int main (int argc, char *argv[])
{

	int ret, i;
	int nSendBufLen = 256*1024;

	printf("Build: %s  %s\n", __TIME__, __DATE__);

	if (signal(SIGPIPE, sig_pipe) == SIG_ERR)
	{
		fprintf(stderr, "can't catch SIGPIPE\n");
		exit(1);
	}
	if (signal(SIGUSR1, sig_usr1) == SIG_ERR)
	{
		fprintf(stderr, "can't catch SIGUSR1\n");
		exit(1);
	}
	/*************************************************/
	fd = open ("/dev/v3best-rx", O_RDWR | S_IRUSR | S_IWUSR);
	if (fd < 0) {
		perror("open");
		exit(0);
	}
	printf ("/dev/v3best-rx opened, fd=%d\n", fd);
	/*************************************************/

	/**
	 * Memory Mapping
	 */
	void *page_ptr;
	long page_addr, page_off, page_size = sysconf(_SC_PAGESIZE);

	page_addr = S2MM_DST_ADDR & (~(page_size-1));
	page_off  = S2MM_DST_ADDR - page_addr;

	page_ptr = map_memory(page_addr, S2MM_BUF_SIZE);

	adc_mem_addr = page_ptr + page_off;

	//adc_mem_addr = map_memory(S2MM_DST_ADDR, S2MM_BUF_SIZE);
	memset(adc_mem_addr, 0x0, S2MM_BUF_SIZE);

	dma_reg_addr_wr = map_memory(AXI_DMA_BASE_WR, REG_SIZE);

	/**
	 * Simple ring buffer
	 */

	cb_adc = (struct ring_buf *)malloc(sizeof(struct ring_buf));
	cb_adc->front=cb_adc->rear=0;
	for(i = 0; i < MaxSize; i++)
		cb_adc->data[i] = adc_mem_addr + (i * BUFFER_MAX_SIZE);

	/************************adc tcp socket**********************/
	adc_param.sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(adc_param.sockfd == -1)
	{
		printf("socket error:%s\n",strerror(errno));
		return -1;
	}

	bzero(&adc_param.server_addr, sizeof(adc_param.server_addr));
	adc_param.server_addr.sin_family = AF_INET;
	adc_param.server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	adc_param.server_addr.sin_port = htons(5004);
	ret = bind(adc_param.sockfd, (struct sockaddr *)&(adc_param.server_addr), sizeof(adc_param.server_addr));
	if(ret == -1)
	{
		printf("bind error:%s\n", strerror(errno));
		return -1;
	}

	ret = listen(adc_param.sockfd, 1);
	if(ret == -1)
	{
		printf("listen error\n");
		return -1;
	}
	adc_param.client_len = sizeof(struct sockaddr);

	while(1){
		adc_param.clifd =accept(adc_param.sockfd, (struct sockaddr *)&(adc_param.client_addr),
				&(adc_param.client_len));
		if(adc_param.clifd == -1)
		{
			printf("accept error!\n");
		}else{
			printf("rx client connect!\n");
		}
		client_connect = E_CLIENT_CONNECT;
		setsockopt(adc_param.clifd, SOL_SOCKET, SO_SNDBUF, (const char *)&nSendBufLen, sizeof(int) );

		if(recv_msg(&adc_mode) < 0){
			dma_dbg(MSG_ERROR,"recv msg failed\n");
			close(adc_param.clifd);
			client_connect = E_CLIENT_DISCONNECT;
			printf("connect close!\n");
			continue;
		}
		if(adc_mode == E_GNURADIO_MODE){
			ret = pthread_create(&thread_send, NULL, &thread_sendto_func, NULL);
			if (ret)
			{
				dma_dbg(MSG_ERROR,"ERROR; return code from pthread_create() is %d\n", ret);
				return -1;
			}
			ret = pthread_create(&thread_adc, NULL, &thread_adc_func, &adc_mode);
			if (ret)
			{
				dma_dbg(MSG_ERROR,"ERROR; return code from pthread_create() is %d\n", ret);
				return -1;
			}

			pthread_join(thread_adc,NULL);
			pthread_join(thread_send,NULL);
			cb_adc->front=cb_adc->rear=0;
		}
		if(adc_mode == E_MATLAB_MODE){
			ret = pthread_create(&thread_adc, NULL, &thread_adc_func, &adc_mode);
			if (ret)
			{
				dma_dbg(MSG_ERROR,"ERROR; return code from pthread_create() is %d\n", ret);
				return -1;
			}
			pthread_join(thread_adc, NULL);
			close(adc_param.clifd);
			client_connect = E_CLIENT_DISCONNECT;
			printf("connect close!\n");
		}
	}
	return 0;
}

/**
 * @brief  recv data from axi dma ipcore thread.
 *
 * @param[in]  temp               user parameters
 * @retval                        NULL
 *
 */
void *thread_adc_func(void *arg)
{
	//pthread_detach(pthread_self());

	cpu_set_t mask ;
	CPU_ZERO(&mask);
	CPU_SET(2,&mask);
	int ret = 0;
	unsigned int mode = *((unsigned int *)arg);

	ret =pthread_setaffinity_np(pthread_self(),sizeof(mask),(const cpu_set_t*)&mask );
	if(ret < 0)
		printf("adc:pthread_setaffinity_np error... \n");

	struct axi_dma_config_info config_info;
	config_info.channel = S2MM_CHANNEL;
	config_info.cycle = 3;
	config_info.length = PACKET_SIZE*2;

	if(mode == E_GNURADIO_MODE){
		adc_state = E_STATE_GNURADIO;
		dma_dbg(MSG_DEBUG,"ADC: gnuradio mode is running!\n");
		while(client_connect)
		{
			if((cb_adc->rear+1)%MaxSize==cb_adc->front)
			{
				dma_dbg(MSG_WARNING,"cb_adc full!\n");
				sleep(0);
				continue;
			}
			else
			{
#ifdef DRIVER_IOCTL
				config_info.mem_addr = S2MM_DST_ADDR+(cb_adc->data[cb_adc->rear]-adc_mem_addr);
				if(ioctl(fd, XDMA_IOCTL_CONFIG, &config_info) < 0)
				{
					perror("ioctl-start");
				}
#else
				axi_dma_init(S2MM_CHANNEL, PACKET_SIZE*2, 3, S2MM_DST_ADDR+(cb_adc->data[cb_adc->rear]-adc_mem_addr),(unsigned long)dma_reg_addr_wr);

#endif
				if(ioctl(fd, XDMA_START_WAIT_COMPLETE, &config_info) < 0)
				{
					perror("ioctl-wait");
				}

#ifdef DRIVER_IOCTL
				config_info.reset_type = IP_RESET;
				if(ioctl(fd, XDMA_IOCTL_RESET, &config_info) < 0)
				{
					perror("ioctl-reset");
				}
#else
				axi_dma_reset(S2MM_CHANNEL, (unsigned long)dma_reg_addr_wr, IP_RESET);
#endif
				cb_adc->rear=(cb_adc->rear+1)%MaxSize;
			}
		}
		config_info.reset_type = FIFO_IP_RESET;
		if(ioctl(fd, XDMA_IOCTL_RESET, &config_info) < 0)
		{
			perror("ioctl-reset");
		}
	}
	if(mode == E_MATLAB_MODE){
		int i, nbyte = 0;
		unsigned int length = 0, times = 0, len = 0, rx_len;
		if(recv_msg(&len) < 0){
			dma_dbg(MSG_DEBUG,"ADC:recv msg failed!\n");
			close(adc_param.clifd);
			client_connect = E_CLIENT_DISCONNECT;
			return NULL;
		}
		rx_len = len;
		dma_dbg(MSG_DEBUG,"recv %d bytes!\n", len);
		if(len%8 != 0){
			len = len + (8 - len%8);
		}

		adc_state = E_STATE_MATLAB;
		if(len < LENGTH_MAX){
			for(i = len; i>=8; i-=8) {
				if(len%i==0) {
					length = i;
					times = len/i;
					dma_dbg(MSG_DEBUG,"rx_length = %d, length = %d, times = %d\n", len, length, times);
					break;
				}
			}
		} else {
			for(i = LENGTH_MAX; i>=8; i-=8) {
				if(len%i==0) {
					length = i;
					times = len/i;
					dma_dbg(MSG_DEBUG,"rx_length = %d, length = %d, times = %d\n", len, length, times);
					break;
				}
			}
		}
		do{
			axi_dma_init(S2MM_CHANNEL, length, times-1, S2MM_DST_ADDR,
					(unsigned long)dma_reg_addr_wr);
			axi_dma_start(S2MM_CHANNEL, 0, (unsigned long)dma_reg_addr_wr);
			if(ioctl(fd, XDMA_START_WAIT_COMPLETE, &config_info) < 0)
			{
				perror("ioctl-wait");
			}
			axi_dma_reset(S2MM_CHANNEL, (unsigned long)dma_reg_addr_wr, FIFO_IP_RESET);
			nbyte = send(adc_param.clifd, (void *)adc_mem_addr, rx_len, 0);
			if(nbyte < 0)
				if(nbyte < 0 && errno == 32){
					/* Broken_PIPE=32, client closed */
					dma_dbg(MSG_DEBUG,"send:errno=%s\n", strerror(errno));
					close(adc_param.clifd);
					client_connect = E_CLIENT_DISCONNECT;
					break;
				}
		}while(1);
	}

	return NULL;
}
void *thread_sendto_func(void *arg)
{
	int ret = 0, nbyte = 0;
	cpu_set_t mask ;
	CPU_ZERO(&mask);
	CPU_SET(1, &mask);

	ret =pthread_setaffinity_np(pthread_self(),sizeof(mask),(const cpu_set_t*)&mask );
	if(ret < 0)
		printf("adc:pthread_setaffinity_np error... \n");

	while(1){
		if(cb_adc->front==cb_adc->rear)
		{
			//dma_dbg(MSG_WARNING,"cb_adc empty!\n");
			//pthread_mutex_unlock(&lock);
			//sleep(0);
			continue;
		}
		else
		{
			memcpy(buffer, cb_adc->data[cb_adc->front], BUFFER_MAX_SIZE);
			nbyte = send(adc_param.clifd, buffer, BUFFER_MAX_SIZE, 0);
			if(nbyte < 0 && errno == 32){
				/* Broken_PIPE=32, client closed */
				dma_dbg(MSG_DEBUG,"send:errno=%s\n", strerror(errno));
				close(adc_param.clifd);
				client_connect = E_CLIENT_DISCONNECT;
				break;
			}
			cb_adc->front=(cb_adc->front+1)%MaxSize;

		}
	}
	return NULL;
}


/**
 * @brief  SIGPIPE handle.to avoid program quit from wrong network connection
 *
 * @param[in]  signo              signal id
 * @retval                        void
 *
 */
static void sig_pipe(int signo)
{
	/*nothing to do*/
}
static void sig_usr1(int signo)
{
	unmap_memory(adc_mem_addr, S2MM_BUF_SIZE);
	unmap_memory(dma_reg_addr_wr, REG_SIZE);
	free(cb_adc);
        close(fd);
	exit(1);
}

int recv_msg(unsigned int *param)
{
	int msgid = -1;
	struct msg_st data;
	long int msgtype = 2;


	msgid = msgget((key_t)1234, 0666 | IPC_CREAT);
	if(msgid == -1)
	{
		fprintf(stderr, "msgget failed with error: %d\n", errno);
		return -1;
	}

	if(msgrcv(msgid, (void*)&data, sizeof(struct msg_st), msgtype, 0) == -1)
	{
		fprintf(stderr, "msgrcv failed with errno: %d\n", errno);
		return -1;
	}
	*param = data.param;

	return 0;
}
