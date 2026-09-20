
GEN_CSRCS += custom.c
GEN_CSRCS += app_manager.c
GEN_CSRCS += serial.c
GEN_CSRCS += status_bar.c
GEN_CSRCS += wifi_manager.c
GEN_CSRCS += system_manager.c
GEN_CSRCS += custom_imgbtn.c
GEN_CSRCS += ui_label.c
GEN_CSRCS += ble_manager.c
GEN_CSRCS += ble_provisioning.c
GEN_CSRCS += device_binding_config.c
GEN_CSRCS += robot_tcp.c
GEN_CSRCS += tcp_service.c

DEPPATH += --dep-path $(PRJ_DIR)/custom
VPATH += :$(PRJ_DIR)/custom

CFLAGS += "-I$(PRJ_DIR)/custom"
AFLAGS += "-I$(PRJ_DIR)/custom"
