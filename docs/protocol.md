# 属性与通信协议

## 分层职责

```text
STM32：DHT11 / ADC / GPIO / PWM + JSON + MQTT 3.1.1
ESP8266：UART AT 固件 + Wi-Fi + TCP
OneNET：MQTT 设备接入与物模型 API
微信小程序：HTTPS GET / POST + 页面数据绑定
```

UART 连接 USART2，115200，8 数据位、1 停止位、无校验。设备不是通过 HTTP 上报，也未使用 ESP8266 的 MQTT AT 客户端命令。

## MQTT 会话

- CONNECT：user 使用产品标识，clientid 使用设备名称，password 使用本地配置的认证信息。
- 协议名 MQTT，protocol level 4，即 MQTT 3.1.1。
- Keep Alive：256 秒；Clean Session：1。
- 当前应用发布 QoS0、retain=0，订阅请求 QoS0。
- 普通 AT 发送：`AT+CIPSEND=<长度>`，等待 `>` 后发送指定长度 MQTT 二进制数据。
- 网络接收由 ESP8266 经 `+IPD,<长度>:<载荷>` 输出，随后 STM32 解包。

## 上报

主题：

```text
$sys/{product_id}/{device_name}/thing/property/post
```

示例值仅用于说明：

```json
{
  "id": "123",
  "params": {
    "W": {"value": 25},
    "S": {"value": 60},
    "G": {"value": 42}
  }
}
```

W 是温度整数，S 是湿度整数，G 是相对光照整数。DHT11 结构体保留小数部分，但当前上报只取整数。G 并非 lux。

## 下发

订阅主题：

```text
$sys/{product_id}/{device_name}/thing/property/set
```

设备解析 `params` 中的属性，例如：

```json
{"id":"example-request","params":{"L":true,"B":true,"F":2}}
```

| 属性 | 控制含义 |
|---|---|
| L=false / true | LED 关闭 / 打开 |
| B=false / true | 阈值报警关闭 / 使能 |
| F=0 | 风扇停止 |
| F=1 / 2 / 3 | PWM 比较值 300 / 500 / 750 |

报警使能后，温度整数 >25℃、湿度整数 >70 或相对光照 >90 时鸣叫。B=true 本身不等于无条件打开蜂鸣器。阈值目前在固件本地定义，没有云端修改接口。

TIM1 时钟按 72MHz、PSC=71、ARR=999 计算为 1kHz；三个档位为约30%、50%、75%占空比，未测得对应转速。

## 小程序 API

- 查询：HTTPS GET `/thingmodel/query-device-property`，携带产品标识和设备名。
- 控制：HTTPS POST `/thingmodel/set-device-property`，JSON 包含 `product_id`、`device_name`、`params`。
- 首页 5 秒轮询最新环境属性，控制页 switch / picker 更新 L / B / F。

当前查询按返回数组固定下标读取，控制页先更新本地提示，再请求平台。设备暂未完整实现物模型执行回复和 L / B / F 实际状态上报，所以页面提示不能作为设备动作确认。

## 时间参数

| 事件 | 配置 |
|---|---|
| TIM3 周期 | 100ms |
| 光敏 ADC | TIM3 TRGO 触发，约10Hz |
| 环境属性上报 | 累计40个周期，标称约4s，受主循环执行影响 |
| 小程序查询 | 5s |
| DHT11 读取 | 当前随主循环调用，后续需按器件手册独立调度 |

## 参考资料

- [OneNET 物模型通信主题](https://iot.10086.cn/doc/iot_platform/book/device-connect%26manager/MQTT/topic.html)
- [OneNET 设置设备属性](https://iot.10086.cn/doc/aiot/fuse/detail/1418)
- [MQTT 3.1.1 规范](https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html)
- [ESP8266 AT TCP/IP 示例](https://docs.espressif.com/projects/esp-at/en/release-v2.1.0.0_esp8266/AT_Command_Examples/TCP-IP_AT_Examples.html)
