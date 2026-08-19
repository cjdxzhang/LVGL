#ifndef BLE_PROVISIONING_H
#define BLE_PROVISIONING_H

#include <stdbool.h>

int ble_provisioning_start(void);
void ble_provisioning_stop(void);
bool ble_provisioning_is_connected(void);

#endif // BLE_PROVISIONING_H
