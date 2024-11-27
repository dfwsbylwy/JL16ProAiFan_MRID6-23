# JL16ProAiFan_MRID6-23
 蛟龙16Pro控制台，风扇温控调节、RyzenAdj、NvidiaOC控制，开机自启。


本软件仅适用于蛟龙16Pro2023款，监测温度控制风扇转速。通过ec寄存器读写功能，读取cpu和gpu最大温度，计算当前温度对应的风扇转速，写入最大风扇转速限制风扇转速（因此需要使用带有最大风扇转速调节功能的bios，如v33、v35等版本），间接实现温度和风扇转速控制逻辑。该控制逻辑期望在较高的性能释放下，尽量减小风扇噪声，达到平台散热动态平衡，相应的平台温度会增加，来提高平台和环境的温度差，来补偿因风扇转速减小带来的散热能力损失。

使用资源声明：

https://github.com/Soberia/EmbeddedController  //BIOS核心寄存器读取、写入

https://github.com/GaoXanSheng/JiaoLongWMI  //原生API调用

https://github.com/dfwsbylwy/RyzenAdj   //AMD cpu 控制

https://github.com/cheungxiongwei/WMIC  //主板检测，防止其他机型误使用

https://github.com/Demion/nvapioc  //独显降压、最大频率

https://github.com/zhongyang219/TrafficMonitor（/blob/master/TrafficMonitor/auto_start_helper.cpp //开机自启）
