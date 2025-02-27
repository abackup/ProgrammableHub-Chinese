
////////////////////////////////////
//      设备专用 LED 服务        //
////////////////////////////////////

#include "extras/PwmPin.h"                          // 各种 PWM 函数库

////////////////////////////////////

struct DEV_GenericLED : Service::LightBulb {       // 通用 LED

  // 此版本的 LED 服务适用于可调光和不可调光的设备。
  // 电源状态和 LED 亮度的状态均存储在 NVS 中，以便在设备重启时恢复。
  
  LedPin *LED;                                      // 参考 LedPin
  SpanCharacteristic *power;                        // 参考论特征
  SpanCharacteristic *level;                        // 参考亮度特性
  boolean isDimmable;                               // 标记以指示灯光是否可调光
 
  DEV_GenericLED(int ledPin, uint8_t dimmable=0) : Service::LightBulb(){

    power=new Characteristic::On(0,true);           // 第二个参数为真，因此 On Characteristic 的值（初始设置为 0）将保存在 NVS 中
    isDimmable=dimmable;

    if(isDimmable){
      level=new Characteristic::Brightness(50,true); // 第二个参数为真，因此亮度特征的值（初始设置为 50）将保存在 NVS 中              
      level->setRange(5,100,1);                       // 将亮度范围设置为最小 5% 到最大 100%，步长为 1%
    }

    this->LED=new LedPin(ledPin);                   // 配置 PWM LED 以输出至引脚号“ledPin”

    Serial.printf("Configuring LED: Pin=%d %s\n",LED->getPin(),isDimmable?"(Dimmable)":""); // 初始化消息

    LED->set(power->getVal()*(isDimmable?(level->getVal()):100));             // 在启动时将LED设置为其初始状态。
    
  } // 结束构造函数

  boolean update(){                              // update() 方法

    LOG1("Updating LED on pin=");
    LOG1(LED->getPin());
    LOG1(":  Current Power=");
    LOG1(power->getVal()?"true":"false");
    if(isDimmable){
      LOG1("  Current Brightness=");
      LOG1(level->getVal());
    }
  
    if(power->updated()){
      LOG1("  New Power=");
      LOG1(power->getNewVal()?"true":"false");
    }

    if(isDimmable && level->updated()){
      LOG1("  New Brightness=");
      LOG1(level->getNewVal());
    } 

    LOG1("\n");
    
    LED->set(power->getNewVal()*(isDimmable?(level->getNewVal()):100));       // 更新物理 LED 以反映新值
   
    return(true);                               // 返回 true
  
  } // 更新

};
      
//////////////////////////////////
