/*********************************************************************************
 * MIT 许可证
 *
 * 版权所有 (c) 2021 Gregg E. Berman
 *
 * https://github.com/HomeSpan/HomeSpan
 *
 * 在此免费授予获得此软件和相关文档文件（“软件”）副本的任何人许可，以无限制方式处理软件，
 * 包括但不限于使用、复制、修改、合并、发布、分发、再许可和/或销售软件副本的权利，并允许向
 * 其提供软件的人员这样做，但须遵守以下条件：
 *
 * 上述版权声明和本许可声明应包含在软件的所有副本或重要部分中。
 *
 * 本软件按“原样”提供，不作任何明示或暗示的保证，包括但不限于适销性、
 * 特定用途的适用性和不侵权性的保证。在任何情况下，
 * 作者或版权持有人均不对因
 * 本软件或使用或以其他方式处理本软件而引起的或与之相关的任何索赔、损害或其他
 * 责任（无论是合同、侵权或其他行为）负责。
 *
 *  
 ********************************************************************************/
 
////////////////////////////////////////////////////////////
//                                                        //
//      HomeSpan：ESP32 的 HomeKit 实现                   //
//    ------------------------------------------------    //
//                                                        //
//       演示如何与 HomeSpan 一起实现网络服务器，         //
//       以创建可编程集线器，可为多达 16 个可调光          //
//                或不可调光的灯提供服务。                 //
//                                                        //
////////////////////////////////////////////////////////////

#include "HomeSpan.h" 
#include "DEV_LED.h"     
#include "DEV_Identify.h"       

#include <WebServer.h>                    // 包含 WebServer 库
WebServer webServer(80);                  // 在端口 80 上创建 WebServer

#define NLIGHTS 16                        // 灯泡的最大数量（由于只有 16 个 PWM 通道，因此限制为 16 个）

uint8_t pinList[]={0,4,5,12,14,15,16,17,18,19,22,23,25,26,27,32,33};      // 允许引脚列表
char lightNames[NLIGHTS][9];                                              // 默认光源名称的存储

nvs_handle lightNVS;                                                      // NVS 存储的句柄

struct {                                                                  // 用于存储引脚号和可调光标志的结构
  uint8_t pin=0;
  uint8_t dimmable=0;
} lightData[NLIGHTS];

////////////////////////////////////////////////////////////

void setup() {
 
  Serial.begin(115200);

  homeSpan.setLogLevel(1);

  homeSpan.setHostNameSuffix("");         // 使用空字符串作为后缀（而不是 HomeSpan 设备 ID）
  homeSpan.setPortNum(1201);              // 更改 HomeSpan 的端口号，以便我们可以将端口 80 用于 Web 服务器
  homeSpan.enableOTA();                   // 启用 OTA 更新
  homeSpan.setMaxConnections(5);          // 将最大连接数减少到 5（默认为 8），因为 WebServer 和连接客户端需要 2 个，而 OTA 需要 1 个
  homeSpan.setWifiCallback(setupWeb);     // 建立 WiFi 后需要启动 Web 服务器
  
  homeSpan.begin(Category::Bridges,"HomeSpan Light Hub","homespanhub");

  for(int i=0;i<NLIGHTS;i++)                              // 为每个灯创建默认名称
    sprintf(lightNames[i],"Light-%02d",i+1);

  size_t len;  
  nvs_open("LIGHTS",NVS_READWRITE,&lightNVS);             // 打开 LIGHTS NVS
  if(!nvs_get_blob(lightNVS,"LIGHTDATA",NULL,&len))       // 如果发现数据
    nvs_get_blob(lightNVS,"LIGHTDATA",&lightData,&len);   // 检索数据

  // 创建桥附件
  
  new SpanAccessory(1);  
    new DEV_Identify("HomeSpan Hub","HomeSpan","LS-123","Light Server","1.0",3);
    new Service::HAPProtocolInformation();
      new Characteristic::Version("1.1.0");

  // 为每个定义的光源动态创建一个新的附件

  for(int i=0;i<NLIGHTS;i++){
    if(lightData[i].pin>0){
      new SpanAccessory(i+2);
        new DEV_Identify(lightNames[i],"HomeSpan",lightNames[i],lightData[i].dimmable?"Dimmable":"Not Dimmable","1.0",0);    
        new DEV_GenericLED(lightData[i].pin,lightData[i].dimmable);       
    }
  }
  
} // 设置结束()

//////////////////////////////////////

void loop(){
  
  homeSpan.poll();
  webServer.handleClient();               // 每次循环需要处理一次 webServer
  
} // 循环结束()

//////////////////////////////////////

void setupWeb(){
  Serial.print("Starting Light Server Hub...\n\n");
  webServer.begin();

  // 内联创建 Web 例程

  webServer.on("/", []() {
  
    String content = "<html><body><form action='/configure' method='POST'><b>HomeSpan Light Server Hub Configuration</b><br><br>";
    content += "Select pins and check box if dimmable:<br><br>";

    for(int i=0;i<NLIGHTS;i++){
      content += "<span style=\"color:";
      if(lightData[i].pin==0)
        content += "grey";
      else if(lightData[i].dimmable)
        content += "red";
      else
        content += "blue";
      content += ";\">";
        
      content += String(lightNames[i]) + ": ";
      
      content += "<select name='p" + String(i) + "'>";
      for(int j=0;j<sizeof(pinList);j++){
        content += "<option value='" + String(pinList[j]) + "'";
        if(lightData[i].pin==pinList[j])
          content += " selected";
        content += ">";
        if(j>0)
          content += "Pin " + String(pinList[j]);
        else
          content += "None";
        content += "</option>";
      }
      content += "</select> ";
      content += "<input type='checkbox' value='1'";
      if(lightData[i].dimmable)
        content += " checked";
      content += " name='t" + String(i) + "'></span><br>";      
    }
    
    content += "<br><input type='reset'><input type='submit' value='Update'></form><br>";
    
    webServer.send(200, "text/html", content);
    
  });  

  webServer.on("/configure", []() {

    for(int i=0;i<NLIGHTS;i++)      // 清除可调光状态，因为复选框仅在被选中时才提供数据
      lightData[i].dimmable=0;
    
    for(int i=0;i<webServer.args();i++){
      switch(webServer.argName(i).charAt(0)){
        case 'p':
          lightData[webServer.argName(i).substring(1).toInt()].pin=webServer.arg(i).toInt();
          break;
        case 't':
          lightData[webServer.argName(i).substring(1).toInt()].dimmable=webServer.arg(i).toInt();
          break;
      }
    }

    String content = "<html><body>Settings Saved!<br><br>";

    for(int i=0;i<NLIGHTS;i++)
      if(lightData[i].pin)
        content += lightNames[i] + String(": Pin=") + String(lightData[i].pin) + String(lightData[i].dimmable?" Dimmable":"") + "<br>";
      else
        lightData[i].dimmable=0;
    
    content += "<br><button onclick=\"document.location='/'\">Return</button> ";
    content += "<button onclick=\"document.location='/reboot'\">Reboot</button>";

    nvs_set_blob(lightNVS,"LIGHTDATA",&lightData,sizeof(lightData));        // 更新数据
    nvs_commit(lightNVS);                                                   // 存储到 NVS

    webServer.send(200, "text/html", content);
  
  });

  webServer.on("/reboot", []() {
    
    String content = "<html><body>Rebooting!  Will return to configuration page in 10 seconds.<br><br>";
    content += "<meta http-equiv = \"refresh\" content = \"10; url = /\" />";
    webServer.send(200, "text/html", content);

    for(int j=0;j<sizeof(pinList);j++)            // 这似乎是必要的，以确保重启时所有引脚都与 LED PWM 断开连接
      gpio_reset_pin((gpio_num_t)pinList[j]);     // 否则 ESP32 似乎保留了一些有关引脚连接的信息？
      
    ESP.restart();
  });

} // 设置Web
