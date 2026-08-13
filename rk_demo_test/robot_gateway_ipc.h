#ifndef ROBOT_GATEWAY_IPC_H
#define ROBOT_GATEWAY_IPC_H

#define ROBOT_GATEWAY_IPC_SOCKET_PATH "/run/robot_gateway.sock"

int robot_gateway_ipc_start(void);
void robot_gateway_ipc_stop(void);

#endif
