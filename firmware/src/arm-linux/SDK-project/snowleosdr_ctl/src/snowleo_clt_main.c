/**
 * $Id: snowleo_sdr_main.c 2014-5
 *
 * @brief SNOWleo SDR main Module.
 *
 * @Author jacky <gaocc@v3best.com>
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
#include <sys/msg.h>

/**/
#define  PACKAGE_HEAD         0xF0
#define  SDR_HANDSHAKE        0x16

#define  SDR_RF_CTRL_TX_FREQ  0x17
#define  SDR_RF_CTRL_RX_FREQ  0x18
#define  SDR_RF_CTRL_TX_VGA   0x19
#define  SDR_RF_CTRL_RX_VGA   0x20
#define  SDR_RF_CTRL_TX_DC    0x21
#define  SDR_RF_CTRL_RX_DC    0x23
#define  SDR_RF_CTRL_SAMPRATE 0x24
#define  SDR_CUSTOM_CMD       0x22

enum {
	E_TX,
	E_RX,
};

struct global_param cmd_param;
unsigned int cmd_buff[2];
int fd, spi_fd;
pthread_t thread_cmd;
unsigned char *dma_reg_addr_wr;
unsigned char *dma_reg_addr_rd;

/**
 * Function declaration
 */
void *thread_cmd_func(void *arg);
static void sig_pipe(int signo);
int send_msg(long int msg_type, unsigned int param);
extern int ConfigureClocks(int fout);

int main (int argc, char *argv[])
{

	int ret;

	printf("Build: %s  %s\n", __TIME__, __DATE__);

	if (signal(SIGPIPE, sig_pipe) == SIG_ERR)
	{
		fprintf(stderr, "can't catch SIGPIPE\n");
		exit(1);
	}

	/**
	 * Memory Mapping
	 */

	dma_reg_addr_wr = map_memory(AXI_DMA_BASE_WR, REG_SIZE);
	dma_reg_addr_rd = map_memory(AXI_DMA_BASE_RD, REG_SIZE);

	/*snowleo lms6002 clock set*/
	if(ConfigureClocks(20000000) < 0){
		printf("Set si5351 failed, using default\n");
		snowleo_sdr_set_clock(I2C_DEV, 4);
	}else{
		snowleo_sdr_set_clock(I2C_DEV, 5);
	}
	/*snowleo lms6002 spi init*/
	spi_fd = snowleo_sdr_spi_init(SPI_DEV);
	transfer(spi_fd);

	/*******************cmd udp socket**************************/
	// Init socket
	cmd_param.server_addr.sin_family=AF_INET;
	cmd_param.server_addr.sin_port = htons(5006);
	cmd_param.server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	cmd_param.sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	if(bind(cmd_param.sockfd, (struct sockaddr *)&(cmd_param.server_addr), sizeof(cmd_param.server_addr)) < 0)
		perror("bind");
	/************************************************************/
	ret = pthread_create(&thread_cmd, NULL, &thread_cmd_func, NULL);
	if (ret)
	{
		dma_dbg(MSG_ERROR,"ERROR; return code from pthread_create() is %d\n", ret);
		return -1;
	}

	pthread_join(thread_cmd,NULL);
	return 0;
}

/**
 * @brief  recv command from network connection.
 *
 * @param[in]  temp               user parameters
 * @retval                        NULL
 *
 */
void *thread_cmd_func(void *arg)
{
	int ret = 0;
	while(1)
	{
		recvfrom(cmd_param.sockfd, (unsigned char *)&cmd_buff, sizeof(unsigned int)*2,
				0, (struct sockaddr*)&(cmd_param.client_addr), &(cmd_param.client_len));
		dma_dbg(MSG_DEBUG,"cmd[0] = 0x%x, cmd[1] = 0x%x\n", cmd_buff[0], cmd_buff[1]);
		if((cmd_buff[0]&0xFF000000)>>24!= PACKAGE_HEAD)
		{
			memset((unsigned char *)cmd_buff, 0, sizeof(unsigned int)*2);
			perror("package error!\n");
			continue;
		}

		switch ((cmd_buff[0]&0x00FF0000)>>16)
		{
		case SDR_HANDSHAKE:
		{
			long int msg_type;
			if((cmd_buff[0]&0x0000FF00)>>8 == E_TX){
				if((cmd_buff[0]&0x000000FF) == E_GNURADIO_MODE){
					msg_type = 1;
					if(send_msg(msg_type, E_GNURADIO_MODE) < 0){
						dma_dbg(MSG_ERROR,"send msg failed!\n");
						break;
					}
					dma_dbg(MSG_DEBUG, "TX: E_GNURADIO_MODE\n");
				}
				if((cmd_buff[0]&0x000000FF) == E_MATLAB_MODE){
					msg_type = 1;
					if(send_msg(msg_type, E_MATLAB_MODE) < 0){
						dma_dbg(MSG_ERROR,"send msg failed!\n");
						break;
					}
					dma_dbg(MSG_DEBUG, "TX: E_MATLAB_MODE\n");
				}
				break;
			}
			if((cmd_buff[0]&0x0000FF00)>>8 == E_RX){
				if((cmd_buff[0]&0x000000FF) == E_GNURADIO_MODE){
					msg_type = 2;
					if(send_msg(msg_type, E_GNURADIO_MODE) < 0){
						dma_dbg(MSG_ERROR,"send msg failed!\n");
						break;
					}
					dma_dbg(MSG_DEBUG, "RX: E_GNURADIO_MODE\n");
				}
				if((cmd_buff[0]&0x000000FF) == E_MATLAB_MODE){
					msg_type = 2;
					if(send_msg(msg_type, E_MATLAB_MODE) < 0){
						dma_dbg(MSG_ERROR,"send msg failed!\n");
						break;
					}
					msg_type = 2;
					if(send_msg(msg_type, cmd_buff[1]) < 0){
						dma_dbg(MSG_ERROR,"send msg failed!\n");
						break;
					}
					dma_dbg(MSG_DEBUG, "RX: E_MATLAB_MODE\n");
				}
				break;
			}
		}
		case SDR_RF_CTRL_TX_FREQ:
			snowleo_sdr_set_freq(spi_fd, cmd_buff[1], RF_TX_CHANNEL);
			dma_dbg(MSG_DEBUG,"recv SDR_RF_CTRL_TX_FREQ cmd\n");
			break;
		case SDR_RF_CTRL_TX_DC:
			snowleo_sdr_set_tx_dc(spi_fd, (uint8_t)((cmd_buff[0]&0x0000FF00)>>8), (uint8_t)(cmd_buff[0]&0x000000FF));
			dma_dbg(MSG_DEBUG,"recv SDR_RF_CTRL_TX_DC cmd\n");
			break;
		case SDR_RF_CTRL_TX_VGA:
			snowleo_sdr_set_vga(spi_fd, (cmd_buff[0]&0x0000FFFF)|(cmd_buff[1]&0xFFFF0000), RF_TX_CHANNEL);
			dma_dbg(MSG_DEBUG,"recv SDR_RF_CTRL_TX_VGA cmd\n");
			break;
		case SDR_RF_CTRL_RX_DC:
			snowleo_sdr_set_rx_dc(spi_fd, (uint8_t)((cmd_buff[0]&0x0000FF00)>>8), (uint8_t)(cmd_buff[0]&0x000000FF));
			dma_dbg(MSG_DEBUG,"recv SDR_RF_CTRL_TX_DC cmd\n");
			break;
		case SDR_RF_CTRL_SAMPRATE:
			if(ConfigureClocks(cmd_buff[1]) < 0){
				dma_dbg(MSG_DEBUG,"Set si5351 failed, using default\n");
				snowleo_sdr_set_clock(I2C_DEV, 0);
			}else{
				snowleo_sdr_set_clock(I2C_DEV, 5);
			}
			dma_dbg(MSG_DEBUG,"recv SDR_RF_CTRL_SAMPRATE cmd\n");
			break;
		case SDR_RF_CTRL_RX_FREQ:
			snowleo_sdr_set_freq(spi_fd, cmd_buff[1], RF_RX_CHANNEL);
			dma_dbg(MSG_DEBUG,"recv SDR_RF_CTRL_RX_FREQ cmd\n");
			break;
		case SDR_RF_CTRL_RX_VGA:
			snowleo_sdr_set_vga(spi_fd, (cmd_buff[0]&0x0000FFFF)|((cmd_buff[1]&0xFF000000)>>8), RF_RX_CHANNEL);
			dma_dbg(MSG_DEBUG,"recv SDR_RF_CTRL_RX_VGA cmd\n");
			break;

		case SDR_CUSTOM_CMD:
		{
			uint8_t data_size = cmd_buff[0]&0x000000FF;
			uint8_t data_rd[data_size];
			uint8_t data_wr[data_size];
			int i;
			if((recvfrom(cmd_param.sockfd, data_wr, data_size, 0,
					(struct sockaddr*)&(cmd_param.client_addr), &(cmd_param.client_len)))<=0)
			{
				break;
			}
			if(((cmd_buff[0]&0x0000FF00)>>8)==0) {

				snowleo_spi_custom_write(spi_fd, data_wr, data_size);
			}

			if(((cmd_buff[0]&0x0000FF00)>>8)==1) {
				snowleo_spi_custom_read(spi_fd, data_wr, data_rd, data_size);
				for(i = 0; i < data_size; i++)
					printf("reg[0x%x] = %d,", data_wr[i], data_rd[i]);
				printf("\n");
				if((ret = sendto(cmd_param.sockfd, data_rd, data_size, 0,
						(const struct sockaddr *)&(cmd_param.client_addr), cmd_param.client_len))<=0)
				{
					break;
				}
			}

			dma_dbg(MSG_DEBUG,"recv SDR_CUSTOM cmd\n");
			break;
		}
		default:
			break;
		}

		memset((unsigned char *)cmd_buff, 0, sizeof(unsigned int)*2);
	}

	return NULL;
}

/**
 * @brief  SIGPIPE handle to avoid program quit from wrong network connection
 *
 * @param[in]  signo              signal id
 * @retval                        void
 *
 */
static void sig_pipe(int signo)
{
	/*nothing to do*/
}


int send_msg(long int msg_type, unsigned int param)
{
	struct msg_st data;
	int msgid = -1;


	msgid = msgget((key_t)1234, 0666 | IPC_CREAT);
	if(msgid == -1)
	{
		fprintf(stderr, "msgget failed with error: %d\n", errno);
		return -1;
	}
	data.msg_type = msg_type;
	data.param = param;

	if(msgsnd(msgid, (void*)&data, sizeof(struct msg_st), 0) == -1)
	{
		fprintf(stderr, "msgsnd failed\n");
		return -1;
	}
	return 0;
}
