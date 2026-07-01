#ifndef GATEWAY_CLIENT_H
#define GATEWAY_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define GATEWAY_CLIENT_OK 0
#define GATEWAY_CLIENT_ERR_PARAM (-1)
#define GATEWAY_CLIENT_ERR_CONNECT (-2)
#define GATEWAY_CLIENT_ERR_SEND (-3)
#define GATEWAY_CLIENT_ERR_RECV (-4)
#define GATEWAY_CLIENT_ERR_RESPONSE (-5)

typedef enum {
	GATEWAY_LOCATION_LIVING_ROOM = 0,
	GATEWAY_LOCATION_BEDROOM,
	GATEWAY_LOCATION_STUDY,
	GATEWAY_LOCATION_BALCONY,
	GATEWAY_LOCATION_BASE,
	GATEWAY_LOCATION_COUNT
} gateway_location_id_t;

int gateway_client_go_location(gateway_location_id_t id);
int gateway_client_clear_navigation(void);
int gateway_client_return_base(void);
int gateway_client_refresh_status(void);
int gateway_client_start_mapping(void);
int gateway_client_stop_mapping(void);
int gateway_client_save_map(const char *name, const char *alias);
int gateway_client_lock_map(const char *mapname);
int gateway_client_request_backup_map(const char *mapname);
int gateway_client_cancel_charge(void);
int gateway_client_heartbeat(void);
int gateway_client_set_wheel_enable(bool enable);
int gateway_client_manual_control(double speed, double angle);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GATEWAY_CLIENT_H */
