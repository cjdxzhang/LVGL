#include "ble_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>

#include "ble_provisioning.h"

// 内部状态
static bool is_advertising = false;
static bool is_scanning = false;
static bool g_sim_mode = false;   // 模拟模式：PC 模拟器无 bluetoothctl 时启用
static ble_scan_callback_t scan_cb = NULL;
static void *scan_user_data = NULL;

// 持久 bluetoothctl 会话
// bluetoothctl 通过 D-Bus 连 bluetoothd，普通用户即可用（root-free）；
// 它注册的广播/agent 对象是"客户端进程持有"的，一次性 `printf ... | bluetoothctl`
// 管道 EOF 后进程退出、广播即销毁，所以必须保持会话存活到应用退出。
static FILE *g_btctl = NULL;
static pthread_mutex_t g_btctl_lock = PTHREAD_MUTEX_INITIALIZER;

static int ble_get_default_adapter(char *adapter, size_t size)
{
    FILE *fp;

    if (adapter == NULL || size == 0)
    {
        return -1;
    }

    fp = popen("ls /sys/class/bluetooth/ 2>/dev/null | grep '^hci' | head -n1", "r");
    if (fp == NULL)
    {
        return -1;
    }
    if (fgets(adapter, size, fp) == NULL)
    {
        pclose(fp);
        return -1;
    }
    pclose(fp);

    adapter[strcspn(adapter, "\r\n")] = '\0';
    return (adapter[0] != '\0') ? 0 : -1;
}

static FILE *ble_btctl_open(void)
{
    pthread_mutex_lock(&g_btctl_lock);
    if (g_btctl == NULL)
    {
        g_btctl = popen("bluetoothctl", "w");
        if (g_btctl == NULL)
        {
            printf("BLE: failed to open persistent bluetoothctl session\n");
        }
    }
    pthread_mutex_unlock(&g_btctl_lock);
    return g_btctl;
}

// 检测目标系统是否具备可用蓝牙环境（工具 + 控制器均需存在）
// rk3506 板子：BT 驱动加载后 /sys/class/bluetooth/ 下会有 hci0
// PC 模拟器：可能无 bluetoothctl 或无蓝牙适配器，此时进入模拟模式
static bool ble_btctl_available(void)
{
    // 1) bluetoothctl 工具必须存在
    if (system("command -v bluetoothctl > /dev/null 2>&1") != 0)
    {
        return false;
    }
    // 2) 必须有已注册的蓝牙控制器（hci0 等）
    return (system("ls /sys/class/bluetooth/ 2>/dev/null | grep -q hci") == 0);
}

// 向持久 bluetoothctl 会话发送命令（可含多行，逐行写入）
static int btctl_cmd(const char *cmd_str)
{
    // 模拟模式下不真正调用硬件，直接返回成功
    if (g_sim_mode)
    {
        return 0;
    }
    FILE *f = ble_btctl_open();
    if (f == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&g_btctl_lock);
    const char *p = cmd_str;
    while (*p)
    {
        const char *nl = strchr(p, '\n');
        if (nl)
        {
            fprintf(f, "%.*s\n", (int)(nl - p), p);
            p = nl + 1;
        }
        else
        {
            fprintf(f, "%s\n", p);
            p += strlen(p);
        }
    }
    fflush(f);
    pthread_mutex_unlock(&g_btctl_lock);
    return 0;
}

// 是否有可用的广播状态校验工具（btmgmt 读内核设置 / dbus-send 读 D-Bus 对象）
static bool ble_can_verify_state(void)
{
    return (system("command -v btmgmt > /dev/null 2>&1") == 0 ||
            system("command -v dbus-send > /dev/null 2>&1") == 0);
}

// 查询广播是否真正开启：
// 1) 内核 legacy 广播 —— btmgmt info 的 current settings 含 advertising
// 2) D-Bus 自定义广播 —— 适配器的 LEAdvertisingManager1.ActiveInstances > 0
//    注意不能去 org.bluez 的 GetManagedObjects 里 grep LEAdvertisement1：
//    广播对象是客户端进程导出的，不归 org.bluez 自己持有。
// 注意：bluetoothctl/btmgmt 这类工具失败时也常返回 0（如权限不足），
// 所以"是否真正在广播"必须用状态读取来确认，不能只看命令退出码。
static bool ble_is_advertising_active(void)
{
    char adapter[32];
    char cmd[256];

    if (system("btmgmt info 2>&1 | grep -q 'current settings:.*advertising'") == 0)
    {
        return true;
    }

    if (ble_get_default_adapter(adapter, sizeof(adapter)) == 0)
    {
        snprintf(cmd, sizeof(cmd),
                 "dbus-send --system --print-reply --dest=org.bluez /org/bluez/%s "
                 "org.freedesktop.DBus.Properties.Get "
                 "string:org.bluez.LEAdvertisingManager1 string:ActiveInstances 2>/dev/null "
                 "| grep -Eq 'byte (0x)?[1-9A-Fa-f][0-9A-Fa-f]*'",
                 adapter);
        if (system(cmd) == 0)
        {
            return true;
        }
    }

    if (system("bluetoothctl show 2>/dev/null | grep -q 'ActiveInstances:[[:space:]]*[1-9]' ") == 0)
    {
        return true;
    }
    return false;
}

// 开启/关闭 LE 广播
// 首选 btmgmt：直接走内核 mgmt 层，设置持久生效（需 root）；
// 否则用持久 bluetoothctl 会话（root-free，广播随会话存活）。
// 无论哪种方式都以真实状态校验（btmgmt info / D-Bus 广播对象），
// 校验不过就明确报失败，避免"假成功"导致手机搜不到设备却无任何提示。
static int ble_set_advertising(bool on)
{
    const char *action = on ? "enable" : "disable";
    const char *mode   = on ? "on" : "off";
    char cmd[128];
    int command_result;

    // 模拟模式下不真正操作硬件（正常流程不会走到这里，防御性保护）
    if (g_sim_mode)
    {
        return 0;
    }

    // 1) btmgmt：内核 legacy 广播，持久；需要 root/mgmt 权限（root 运行时生效）
    snprintf(cmd, sizeof(cmd), "btmgmt advertising %s > /dev/null 2>&1", mode);
    command_result = system(cmd);
    if (command_result != 0)
    {
        printf("BLE: btmgmt advertising 命令失败: ret=%d\n", command_result);
    }
    if (ble_is_advertising_active() == on)
    {
        printf("BLE: advertising %s confirmed via btmgmt (kernel)\n", mode);
        return 0;
    }
    printf("BLE: btmgmt could not %s advertising (app 可能非 root), trying persistent bluetoothctl session...\n",
           action);

    // 2) 持久 bluetoothctl 会话（root-free，D-Bus 连 bluetoothd，广播随会话存活）
    if (btctl_cmd(on ? "advertise on" : "advertise off") == 0)
    {
        if (ble_is_advertising_active() == on)
        {
            printf("BLE: advertising %s confirmed via bluetoothctl session\n", mode);
            return 0;
        }
        if (!ble_can_verify_state())
        {
            // 板上没有任何校验工具时无法确认，明确提示"未验证"
            printf("BLE: %s advertising via bluetoothctl (UNVERIFIED: no btmgmt/dbus-send)\n", action);
            return 0;
        }
        printf("BLE: bluetoothctl accepted but state check failed (advertising not visible)\n");
    }
    printf("BLE: FAILED to really %s advertising\n", action);
    return -1;
}

// 初始化蓝牙模块：开启电源、设置名称、确保可被发现
int ble_manager_init(void)
{
    printf("BLE: Initializing...\n");

    if (ble_btctl_available())
    {
        g_sim_mode = false;
        printf("BLE: real mode (bluetooth adapter detected)\n");

        // 0. 打印 BlueZ 版本，便于排查 advertise 命令兼容性
        btctl_cmd("version");

        // 1. 开启蓝牙电源
        if (btctl_cmd("power on") == 0)
        {
            if (ble_provisioning_start() != 0)
            {
                printf("BLE: provisioning GATT service init failed\n");
                return -1;
            }

            // 2. 设置设备名称（使用默认名称 rk3506B）
            ble_manager_set_name("rk3506B");

            // 3. 配置默认广播参数
            btctl_cmd("menu advertise\ndiscoverable on\ndiscoverable-timeout 0\nback");

            printf("BLE: Initialization done (real)\n");
            return 0;
        }
        // 有工具但硬件不可用（如 PC 无适配器）→ 降级为模拟
        printf("BLE: power on failed, fall back to SIMULATION mode\n");
    }
    else
    {
        printf("BLE: no bluetooth adapter/bluetoothctl -> SIMULATION mode\n");
    }

    // 模拟模式：PC 上无蓝牙硬件时的降级路径
    g_sim_mode = true;
    printf("BLE: Initialization done (SIMULATED)\n");
    return 0;
}

// 设置广播名称
int ble_manager_set_name(const char *name)
{
    if (name == NULL) return -1;
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "system-alias %s", name);
    return btctl_cmd(cmd);
}

// 开启广播
int ble_manager_start_advertising(void)
{
    if (is_advertising)
    {
        printf("BLE: Already advertising\n");
        return 0;
    }
    if (g_sim_mode)
    {
        is_advertising = true;
        printf("BLE: Advertising started (SIMULATED)\n");
        return 0;
    }

    printf("BLE: Starting advertising...\n");

    // 1) 主菜单打开 discoverable（经典蓝牙，手机系统设置可发现）
    btctl_cmd("discoverable on");

    // 2) 广播内容带上设备名 + 可发现，手机能显示 "rk3506" 而不是"未命名设备"
    btctl_cmd("menu advertise\nname on\ndiscoverable on\ndiscoverable-timeout 0\nback");

    // 3) 真正开启广播（btmgmt 优先：内核持久；bluetoothctl 兜底）
    if (ble_set_advertising(true) == 0)
    {
        is_advertising = true;
        printf("BLE: Advertising started (real)\n");
        return 0;
    }
    printf("BLE: Failed to start advertising\n");
    return -1;
}

// 停止广播
int ble_manager_stop_advertising(void)
{
    if (!is_advertising) return 0;
    if (g_sim_mode)
    {
        is_advertising = false;
        printf("BLE: Advertising stopped (SIMULATED)\n");
        return 0;
    }
    if (ble_set_advertising(false) == 0)
    {
        is_advertising = false;
        printf("BLE: Advertising stopped (real)\n");
        return 0;
    }
    return -1;
}

// 模拟扫描线程：PC 上模拟发现一个外设，便于调试扫描 UI
static void *sim_scan_thread(void *arg)
{
    struct timespec ts;
    ts.tv_sec = 1;
    ts.tv_nsec = 0;
    nanosleep(&ts, NULL);   // 模拟扫描耗时

    if (scan_cb)
    {
        ble_device_info_t fake;
        memset(&fake, 0, sizeof(fake));
        strncpy(fake.address, "AA:BB:CC:DD:EE:FF", sizeof(fake.address) - 1);
        strncpy(fake.name, "SIM_FAKE_DEVICE", sizeof(fake.name) - 1);
        fake.rssi = -55;
        printf("BLE: [SIM] scan found: %s (%s) rssi=%d\n", fake.name, fake.address, fake.rssi);
        scan_cb(&fake, scan_user_data);
    }
    is_scanning = false;
    return NULL;
}

// 开始扫描
int ble_manager_start_scan(ble_scan_callback_t callback, void *user_data)
{
    if (callback == NULL)
    {
        printf("BLE: Scan callback is NULL\n");
        return -1;
    }
    scan_cb = callback;
    scan_user_data = user_data;

    if (g_sim_mode)
    {
        // 模拟扫描：起一个后台线程，稍后回调一个假设备
        pthread_t tid;
        if (pthread_create(&tid, NULL, sim_scan_thread, NULL) != 0)
        {
            printf("BLE: failed to create sim scan thread\n");
            return -1;
        }
        pthread_detach(tid);
        is_scanning = true;
        printf("BLE: [SIM] scan started\n");
        return 0;
    }

    // 真实模式：bluetoothctl scan on 会持续输出到 stdout，
    // 需要重定向管道/文件后解析，目前尚未实现，明确返回失败。
    printf("BLE: real scan not yet implemented (need bluetoothctl output parsing)\n");
    is_scanning = false;
    return -1;
}

int ble_manager_stop_scan(void)
{
    if (!is_scanning) return 0;
    is_scanning = false;
    printf("BLE: scan stopped\n");
    return 0;
}

bool ble_manager_is_connected(void)
{
    if (g_sim_mode)
    {
        return false;
    }
    return ble_provisioning_is_connected();
}
