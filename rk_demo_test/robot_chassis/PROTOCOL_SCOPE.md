# 260721 底盘协议实施范围

本目录以 robot_file/底盘接口260721.doc 为协议基线，并以
openspec/changes/refactor-rk-demo-robot-chassis-v260721/ 中的设计决定消除原文歧义。

## 传输规则

- 请求：JSON + CRLFCRLF，不带长度头。
- 响应：4 字节小端长度 + JSON + CRLFCRLF，长度包含 4 字节结束符。
- 只接受上述严格格式，不猜测大端、无长度头或其他长度语义。

## 本期命令

| 命令 | 请求关键载荷 | 响应/处理 |
|---|---|---|
| cmd=heatbeat | 无 p | 仅 cmd=heatbeat,result=true 是有效心跳 |
| cmd=reBoot | p:{} | 通用结果；调用前由 UI 确认 |
| t=12 | p:{angle,speed} | 只提供接口，不接入现有业务 |
| t=13 | p.data:{x,y,z,tolerance} | 去指定坐标 |
| t=20 | p.data 为字符串化 {x,y,z} | 带基站坐标回仓充电 |
| t=21 | p:{} | 取消充电 |
| t=23 | p.data:{explore} | 自动探索开关 |
| t=34 | p.data 为字符串化 {mapname} | 锁定地图 |
| t=42 | p.data=1 | 只严格解析 p.data[\"8\"] 对象 |
| t=44 | p.data 为字符串化 {x,y,z} | 初始化机器人坐标 |
| t=45 | p.data:{stop} | 软急停；stop=0 由底盘恢复原导航 |
| t=52/53/54 | 开始/关闭/保存建图 | t=54 的 p.data 为字符串化 {name,alias} |
| t=55 | 无载荷 | Base64 → gzip → 小端宽高头 → 占据栅格 |
| t=58 | 无载荷 | p.data 字符串二次解析为地图名 JSON 数组 |
| t=90 | 无载荷 | 清除导航任务 |
| t=109 | p:{} | 恢复出厂设置 |
| t=1002 | p.data:{time} | 设置系统时间 |
| t=1012/3007 | t=1012 的 p.data 为字符串化 {name}，3007 无载荷 | 十六进制地图备份分段，整包 MD5 |
| t=3004~3006/3003 | 开始/分段/校验/解压 | 分段逐一确认，解压响应按 t=3003 |
| t=10003 | p.data:{enable} | 轮子使能；禁用前由 UI 确认 |

本期明确不实现 t=74 更新地图和 t=80 站点管理。其余文档命令也不暴露公共接口。

