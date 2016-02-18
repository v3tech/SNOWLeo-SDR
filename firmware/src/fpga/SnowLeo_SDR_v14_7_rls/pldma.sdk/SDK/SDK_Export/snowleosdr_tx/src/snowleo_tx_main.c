/**
 * $Id: snowleosdr_tx_main.c 2015-1
 *
 * @brief SNOWleo SDR TX main Module.
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


unsigned int dac_mode = E_GNURADIO_MODE;
unsigned int dac_state = E_STATE_UNINIT;
struct global_param dac_param;

int client_connect;
unsigned int cmd_buff[2];
int fd;
pthread_t thread_dac, thread_recv;
unsigned char *dac_mem_addr, *dma_reg_addr_rd;

struct ring_buf *p;


/**
 * Function declaration
 */

void *thread_dac_func(void *arg);
void *thread_recvfrom_func(void *arg);
int recv_msg(unsigned int *mode);
static void sig_pipe(int signo);
static void sig_usr1(int signo);



int main (int argc, char *argv[])
{

	int ret, i;

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
	fd = open ("/dev/v3best-tx", O_RDWR | S_IRUSR | S_IWUSR);
	if (fd < 0) {
		perror("open");
		exit(0);
	}
	printf ("/dev/v3best-tx opened, fd=%d\n",fd);

	/*************************************************/

	/**
	 * Memory Mapping
	 */
	dac_mem_addr = map_memory(MM2S_SRC_ADDR, MM2S_BUF_SIZE);
	memset(dac_mem_addr, 0,  MM2S_BUF_SIZE);

	dma_reg_addr_rd = map_memory(AXI_DMA_BASE_RD, REG_SIZE);


	/**
	 * Simple ring buffer
	 */
	p = (struct ring_buf *)malloc(sizeof(struct ring_buf));
	p->front=p->rear=0;
	for(i = 0; i < MaxSize; i++)
		p->data[i] = dac_mem_addr + (i * BUFFER_MAX_SIZE);

	/************************dac tcp socket**********************/
	dac_param.sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(dac_param.sockfd == -1)
	{
		printf("socket error:%s\n",strerror(errno));
		return -1;
	}

	bzero(&dac_param.server_addr, sizeof(dac_param.server_addr));
	dac_param.server_addr.sin_family = AF_INET;
	dac_param.server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	dac_param.server_addr.sin_port = htons(5005);
	ret = bind(dac_param.sockfd, (struct sockaddr *)&(dac_param.server_addr), sizeof(dac_param.server_addr));
	if(ret == -1)
	{
		printf("bind error:%s\n", strerror(errno));
		return -1;
	}

	ret = listen(dac_param.sockfd, 1);
	if(ret == -1)
	{
		printf("listen error\n");
		return -1;
	}
	while(1)
	{
		dac_param.client_len = sizeof(struct sockaddr);
		dac_param.clifd =accept(dac_param.sockfd, (struct sockaddr *)&(dac_param.client_addr),
				&(dac_param.client_len));
		if(dac_param.clifd == -1)
		{
			printf("accept error\n");
		}else{
			printf("da client connect!\n");
		}
		client_connect = E_CLIENT_CONNECT;
		/************************************************************/
		if(recv_msg(&dac_mode) < 0){
			dma_dbg(MSG_ERROR,"recv msg failed\n");
			close(dac_param.clifd);
			client_connect = E_CLIENT_DISCONNECT;
			printf("connect close!\n");
			continue;
		}
		if(dac_mode == E_GNURADIO_MODE){
			ret = pthread_create(&thread_dac, NULL, &thread_dac_func, &dac_mode);
			if (ret)
			{
				dma_dbg(MSG_ERROR,"ERROR; return code from pthread_create() is %d\n", ret);
				return -1;
			}
			ret = pthread_create(&thread_recv, NULL, &thread_recvfrom_func, NULL);
			if (ret)
			{
				dma_dbg(MSG_ERROR,"ERROR; return code from pthread_create() is %d\n", ret);
				return -1;
			}

			pthread_join(thread_dac,NULL);
			pthread_join(thread_recv,NULL);
			p->front = p->rear=0;
		}
		if(dac_mode == E_MATLAB_MODE){
			ret = pthread_create(&thread_dac, NULL, &thread_dac_func, &dac_mode);
			if (ret)
			{
				dma_dbg(MSG_ERROR,"ERROR; return code from pthread_create() is %d\n", ret);
				return -1;
			}
			pthread_join(thread_dac,NULL);
			close(dac_param.clifd);
			client_connect = E_CLIENT_DISCONNECT;
			printf("connect close!\n");
		}
	}

	return 0;
}


/**
 * @brief  send data to axi dma thread.
 *
 * @param[in]  temp               user parameters
 * @retval                        NULL
 *
 */
void *thread_dac_func(void *arg)
{
	//pthread_detach(pthread_self());
	cpu_set_t mask ;
	CPU_ZERO(&mask);
	CPU_SET(2,&mask);
	int ret = 0;
	unsigned int mode = *((unsigned int *)arg);
	struct axi_dma_config_info config_info;
	config_info.channel = MM2S_CHANNEL;
	config_info.cycle = 3;
	config_info.length = PACKET_SIZE*2;

	ret =pthread_setaffinity_np(pthread_self(),sizeof(mask),(const cpu_set_t*)&mask );
	if(ret < 0)
		printf("dac:pthread_setaffinity_np error... \n");

	if(mode == E_GNURADIO_MODE){
		if(dac_state == E_STATE_MATLAB){
			if(ioctl(fd, XDMA_START_WAIT_COMPLETE, &config_info) < 0)
			{
				perror("ioctl-wait");
			}
			axi_dma_reset(MM2S_CHANNEL, (unsigned long)dma_reg_addr_rd, IP_RESET);
		}
		dac_state = E_STATE_GNURADIO;
		dma_dbg(MSG_DEBUG,"DAC: gnuradio mode is running!\n");
		while(client_connect){
			if(p->front==p->rear)
			{
				//dma_dbg(MSG_WARNING,"cb empty!\n");
				//pthread_mutex_unlock(&lock);
				sleep(0);
				continue;
			}
			else
			{

				axi_dma_init(MM2S_CHANNEL, PACKET_SIZE*2, 3, MM2S_SRC_ADDR+(p->data[p->front]-dac_mem_addr), (unsigned long)dma_reg_addr_rd);
				if(ioctl(fd, XDMA_START_WAIT_COMPLETE, &config_info) < 0)
				{
					perror("ioctl-wait");
				}
				axi_dma_reset(MM2S_CHANNEL, (unsigned long)dma_reg_addr_rd, IP_RESET);
				p->front=(p->front+1)%MaxSize;
			}
		}
		axi_dma_reset(MM2S_CHANNEL, (unsigned long)dma_reg_addr_rd, FIFO_IP_RESET);
	}
	if(mode == E_MATLAB_MODE){
		int i, length = 0, times = 0, len = 0, nbyte = 0;

		do {
			nbyte = recv(dac_param.clifd, dac_mem_addr + len, 16384, 0);
			len = len + nbyte;
			if(nbyte == 0)
				break;
		}while(1);
		dma_dbg(MSG_DEBUG,"recv %d bytes!\n", len);
		if(len % 8 != 0){
			dma_dbg(MSG_WARNING,"Data length not divisible by 8, \
					we will send the data alignment.\n");
			len = len + (8 - len%8);
		}

		if(dac_state == E_STATE_MATLAB){
			iowrite32(0x11, (unsigned long)dma_reg_addr_rd + MM2S_START);
			check_dma_done(MM2S_CHANNEL, (unsigned long)dma_reg_addr_rd);
			iowrite32(0x00, (unsigned long)dma_reg_addr_rd + MM2S_START);
		}
		dac_state = E_STATE_MATLAB;
		if(len < LENGTH_MAX){
			for(i = len; i>=8; i-=8) {
				if(len%i==0) {
					length = i;
					times = len/i;
					dma_dbg(MSG_DEBUG,"file_length = %d, length = %d, times = %d\n", len, length, times);
					break;
				}
			}
		} else {
			for(i = LENGTH_MAX; i>=8; i-=8) {
				if(len%i==0) {
					length = i;
					times = len/i;
					dma_dbg(MSG_DEBUG,"file_length = %d, length = %d, times = %d\n", len, length, times);
					break;
				}
			}
		}
		axi_dma_init(MM2S_CHANNEL, length, times-1, MM2S_SRC_ADDR, (unsigned long)dma_reg_addr_rd);
		axi_dma_start(MM2S_CHANNEL, 0, (unsigned long)dma_reg_addr_rd);
		dma_dbg(MSG_DEBUG,"DAC: loopback mode for matlab is running!\n");
	}
	return NULL;
}
void *thread_recvfrom_func(void *arg)
{
	int ret = 0;
	//	cpu_set_t mask ;
	//	CPU_ZERO(&mask);
	//	CPU_SET(0,&mask);
	//
	//	ret =pthread_setaffinity_np(pthread_self(),sizeof(mask),(const cpu_set_t*)&mask );
	//	if(ret < 0)
	//		printf("dac:pthread_setaffinity_np error... \n");


	while(1){
		//pthread_mutex_lock(&lock);
		if((p->rear+1)%MaxSize==p->front)
		{
			//dma_dbg(MSG_WARNING,"cb full!\n");
			//pthread_mutex_unlock(&lock);
			sleep(0);
			continue;
		}
		else
		{
			ret = recv(dac_param.clifd, p->data[p->rear], PACKET_SIZE*8, MSG_WAITALL);
			if(ret == 0){
				close(dac_param.clifd);
				client_connect = E_CLIENT_DISCONNECT;
				printf("connect close!\n");
				break;
			}else if(ret < 0){
				printf("recv error!\n");
				continue;
			}
			p->rear=(p->rear+1)%MaxSize;
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
	unmap_memory(dac_mem_addr, MM2S_BUF_SIZE);
	unmap_memory(dma_reg_addr_rd, REG_SIZE);
	free(p);
        close(fd);
	exit(1);
}


int recv_msg(unsigned int *param)
{
	int msgid = -1;
	struct msg_st data;
	long int msgtype = 1;


	msgid = msgget((key_t)1234, 0666 | IPC_CREAT);
	if(msgid == -1)
	{
		dma_dbg(MSG_ERROR, "msgget failed with error: %d\n", errno);
		return -1;
	}

	if(msgrcv(msgid, (void*)&data, sizeof(struct msg_st), msgtype, 0) == -1)
	{
		dma_dbg(MSG_ERROR, "msgrcv failed with errno: %d\n", errno);
		return -1;
	}
	*param = data.param;

	return 0;
}
