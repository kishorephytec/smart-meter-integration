/*
 * PIPE.h
 *
 *  Created on: 06-Jul-2018
 *      Author: pro3
 */


#ifndef PIPE_H_
#define PIPE_H_


/*Pipe creation location*/
char * HostAPD_PIPE_recv 	= "/tmp/hostAPD_Daemon";	/*Host APD will send data in this PIPE && Daemon Read from this PIPE and send on Uart*/
char * HostAPD_PIPE_send 	= "/tmp/Daemon_hostAPD";	/*Daemon send data on this PIPE read from Uart && Host APD will read from this PIPE*/

#if _RestServer
char * RestServer_PIPE_recv = "/tmp/RestServer_Daemon";	/*Rest Server will send data in this PIPE && Daemon Read from this PIPE and send on Uart*/
char * RestServer_PIPE_send = "/tmp/Daemon_RestServer";	/*Daemon send data on this PIPE read from Uart && Rest Server will read from this PIPE*/
#endif

#if _AT_CMD
char * AT_CMD_PIPE_recv 	= "/tmp/AT_CMD_Daemon";	/*AT CMD App will send data in this PIPE && Daemon Read from this PIPE and send on Uart*/
char * AT_CMD_PIPE_send 	= "/tmp/Daemon_AT_CMD";	/*Daemon send data on this PIPE read from Uart && AT CMD App will read from this PIPE*/
#endif

/*changing read/write permissions of named-pipe*/
#define HostAPD_recv_PIPE_Permission 	"sudo chmod +0666 /tmp/hostAPD_Daemon"
#define HostAPD_send_PIPE_Permission 	"sudo chmod +0666 /tmp/Daemon_hostAPD"

#if _RestServer
#define RestServer_recv_PIPE_Permission "sudo chmod +0666 /tmp/RestServer_Daemon"
#define RestServer_send_PIPE_Permission "sudo chmod +0666 /tmp/Daemon_RestServer"
#endif

#if _AT_CMD
#define AT_CMD_recv_PIPE_Permission 	"sudo chmod +0666 /tmp/AT_CMD_Daemon"
#define AT_CMD_send_PIPE_Permission 	"sudo chmod +0666 /tmp/Daemon_AT_CMD"
#endif

int HostAPD_fd_recv,HostAPD_fd_send
#if _RestServer
,\
		RestServer_fd_recv,RestServer_fd_send
#endif
#if _AT_CMD
,\
	AT_CMD_fd_recv,AT_CMD_fd_send
#endif
	;

int creat_PIPE(int 	*fd,char	*PIPE_Path,char	*file_permission);

void	*HostAPD_pipe_rx_thread(void	*unused);
int	HostAPD_PIPE_Data_Reading(void);
int process_HostAPD_pipe_byte(unsigned char p_rx_char);

#if _AT_CMD
void	*AT_CMD_pipe_rx_thread(void		*unused);
int AT_CMD_PIPE_Data_Reading(void);
int process_AT_CMD_pipe_byte(unsigned char p_rx_char);
#endif

#if _RestServer
void	*Rest_Server_pipe_rx_thread(void		*unused);
int Rest_Server_PIPE_Data_Reading(void);
int process_Rest_Server_pipe_byte(unsigned char rs_rx_char);
#endif

#endif /* PIPE_H_ */
