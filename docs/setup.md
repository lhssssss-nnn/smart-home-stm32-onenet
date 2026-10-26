# 环境与配置指南

## 工具与设备

- STM32F103C8T6 目标板、DHT11、光敏模拟电路、ESP8266 AT 模块。
- LED、蜂鸣器、风扇及适配的电机驱动，SWD 调试器，稳定的供电。
- Keil MDK-ARM / ARM Compiler 5、STM32F1 设备包、串口工具。
- 微信开发者工具、自己的小程序 AppID、OneNET 产品和设备。

## 固件配置

复制 `firmware/USER/app_config.local.example.h` 为同目录下的 `app_config.local.h`，填写：

| 宏 | 含义 |
|---|---|
| `APP_WIFI_SSID` | 热点名称 |
| `APP_WIFI_PASSWORD` | 热点密码 |
| `APP_ONENET_PRODUCT_ID` | OneNET 产品标识 |
| `APP_ONENET_DEVICE_NAME` | 设备名称，同时用于 MQTT clientid |
| `APP_ONENET_TOKEN` | 对该设备有效的 MQTT 认证凭据 |
| `APP_ONENET_HOST` | 设备接入主机，默认沿用原工程 |
| `APP_ONENET_PORT` | 默认 1883，当前 AT 指令使用 TCP |

字符串必须是合法 C 字符串。原固件采用普通 TCP，修改端口或主机名本身不会启用 TLS；如需 SSL 连接，还需调整 AT 模式、固件能力和证书策略。

`PROID / DEVID / TOKEN` 和 MQTT 主题均从统一配置派生，避免只修改主题却忘记修改 CONNECT 参数。

## 云平台物模型

在自己的 OneNET 产品中建立 W / S / G / L / B / F 属性，并与 [协议说明](protocol.md) 的类型对齐，创建对应设备与凭据。不要直接复制别人的产品、设备或已过期凭据。

当前设备：

1. ESP8266 加入热点并获得 IP。
2. 建立平台 TCP 连接。
3. STM32 发送 MQTT CONNECT，等待成功 CONNACK。
4. 订阅属性设置主题。
5. 主循环定时发布环境属性，解析控制下行。

联网、TCP、MQTT 和业务属性是不同层次，调试时按上述顺序确认。具体服务地址与授权格式以自己的 OneNET 产品接入文档为准。

## 小程序配置

复制 `miniprogram/config.local.example.js` 为 `config.local.js`：

- `authorization`：OneNET HTTPS API 的授权信息。
- `product_id`、`device_name`：与设备端一致。

`config.js` 统一生成查询 / 设置 API 地址，两个页面使用相同配置。设备端认证和应用 API 授权不应仅因为都叫 token 就视作可互换。

在微信开发者工具导入 `miniprogram/`，填写自己的 AppID，并按平台要求配置 request 合法域名。公开示例的 `touristappid` 不代表已经完成正式账号、真机或上线配置。

本地配置虽不提交 Git，但它会参与小程序构建。正式部署应评估使用受控后端 / 云函数和最小权限授权，避免在客户端分发长期高权限凭据。

## 联调顺序

1. 单独验证 DHT11 数据与光敏 ADC 随遮挡变化。
2. 查看设备启动日志，确认 TCP、MQTT 登录与订阅。
3. 在平台查看 W / S / G 是否更新。
4. 小程序查询并显示环境值。
5. 用 L 验证 LED，再用 F 验证各档 PWM，最后验证 B 与阈值共同触发报警。
6. 分别验证设备离线、非法参数和网络恢复情况，避免只依据界面弹窗判断执行结果。

## 本地检查

从仓库根目录执行：

```powershell
python scripts/check_repository.py
node --check miniprogram/config.js
node --check miniprogram/pages/index/index.js
node --check miniprogram/pages/my/my.js
```

Python 脚本不需要第三方依赖，检查 Git 将发布的文件。它不会连接 OneNET 或输出本地凭据。
