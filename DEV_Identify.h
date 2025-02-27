
//////////////////////////////////
//      特定于设备的服务        //
//////////////////////////////////

struct DEV_Identify : Service::AccessoryInformation {

  int nBlinks;                    // 识别程序中内置 LED 闪烁的次数
  SpanCharacteristic *identify;   // 参考识别特征
  
  DEV_Identify(const char *name, const char *manu, const char *sn, const char *model, const char *version, int nBlinks) : Service::AccessoryInformation(){
    
    new Characteristic::Name(name);                   // 根据上述参数设置值，创建所有必需的特征
    new Characteristic::Manufacturer(manu);
    new Characteristic::SerialNumber(sn);    
    new Characteristic::Model(model);
    new Characteristic::FirmwareRevision(version);
    identify=new Characteristic::Identify();          // 存储对识别特征的引用以供下面使用

    this->nBlinks=nBlinks;                            // 存储 LED 闪烁的次数

    pinMode(homeSpan.getStatusPin(),OUTPUT);          // 确保 LED 已设置为输出
  }

  boolean update(){
       
    for(int i=0;i<nBlinks;i++){
      digitalWrite(homeSpan.getStatusPin(),LOW);
      delay(250);
      digitalWrite(homeSpan.getStatusPin(),HIGH);
      delay(250);
    }

    return(true);                               // 返回 true
    
  } // 更新
  
};
