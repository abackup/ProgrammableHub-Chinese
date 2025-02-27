# ProgrammableHub - HomeSpan 项目

ProgrammableHub 是一个实用示例，展示了如何在 HomeSpan 旁边添加网络服务器以创建可编程轻量级服务器中心。ProgrammableHub 使用 [HomeSpan](https://github.com/HomeSpan/HomeSpan) HomeKit 库构建，旨在作为 Arduino 草图在 ESP32 设备上运行。

该项目使用的硬件：

* ESP32 开发板，例如 [Adafruit HUZZAH32 – ESP32 Feather 开发板](https://www.adafruit.com/product/3405)
* 一个或多个带限流电阻的 LED

此草图演示了许多高级 HomeSpan 技术，包括：

* 动态添加和删除 HomeKit 配件
* 使用 ESP32 的非易失性存储 (NVS) 保存任意数据
* 实现独立 Web 服务器以与 HomeSpan 一起运行

### 注释

* 此草图不是对固定数量的 LED 配件进行硬编码，而是根据存储在 NVS 中的数据在运行时动态创建最多 16 个可调光 LED。
* LED 配件名为 Light-01 到 Light-16，但您可以通过 iPhone 或 iMac 上的 Home App 将其更改为您想要的任何名称。
* 为了帮助识别每个 LED，名称也被复制为每个附件的序列号。
* 独立的 Web 界面用于允许用户指定哪些灯与哪些 ESP32 引脚相关联，以及灯是否可调光。此信息存储在以下结构中：

```C++
struct {
uint8_t pin=0;
uint8_t dimmable=0;
} lightData[NLIGHTS];
```

* 上述结构作为 NVS *blob* 名称 *LIGHTDATA* 存储在 NVS 空间名称 *LIGHTS* 中。
* 启动时，草图会创建一个桥接器作为附件 #1，然后动态添加附件 #2 到 #17，每个 Light-01 到 Light-16 都具有定义的 ESP32 引脚。
* 不可调光灯仅使用 `Characteristic::On()` 实现开/关功能。
* 可调光灯使用 `Characteristic::Brightness()` 实现额外的亮度控制。
* 设备的 Web 界面名称为 *homespanhub.local*（参见 [screenshot](images/WebInterface.png)）。已使用方法 `homeSpan.setHostNameSuffix("")` 抑制了正常的设备 ID 后缀
* 必须重新启动设备（手动或通过网络界面）才能将更新传播到您的 Home App。
* 如果 Home App 在重新启动后不久未反映您的更改，请尝试重新启动 Home App。
* 网络界面使用 ESP32-Arduino 标准 WebServer 库实现，该库与 HomeSpan 使用的 WiFi/TCP 堆栈兼容。***请注意，ESPAsyncWebServer 需要不同的 TCP 堆栈，不能与 HomeSpan 一起使用。***
* 值得注意的配置设置：
  * HomeSpan 通常使用端口 80 进行 HomeKit 通信。要允许 Web 界面使用端口 80，必须使用 `homeSpan.setPortNum(port)` 将 HomeSpan 端口更改为其他端口。在此草图中，端口更改为 1201（任意数字）。
  * HomeSpan 通常为 HomeKit 控制器保留 8 个 TCP 连接。由于 WebServer 消耗 2 个 TCP 套接字（一个用于服务器，一个用于客户端），我们需要指示 HomeSpan 不要为自己使用所有 8 个 TCP 连接。此外，此草图启用了 OTA 更新，这也需要一个 TCP 套接字。因此，必须使用方法 `homeSpan.setMaxConnections(5)` 将为 HomeSpan 保留的 TCP 套接字总数减少到 5 个。
  * 在设备建立 WiFi 连接之前，无法启动 WebServer。设置 WebServer 的代码包装在函数 `setupWeb()` 中，并通过回调 `homeSpan.setWifiCallback(setupWeb)` 启动。

---

### 反馈或问题？

请考虑添加到 [讨论板](https://github.com/HomeSpan/HomeSpan/discussions)，或直接发送电子邮件至 [homespan@icloud.com](mailto:homespan@icloud.com)。



