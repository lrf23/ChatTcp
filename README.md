# ChatTcp
基于TCP的简单聊天程序
功能：  
1.多人聊天、登录  
2.互传文件  


使用方式：   
1.servercode里是服务器的代码

2.clientcode里有两个文件夹，makeexe文件夹里myqt1.exe可以点击直接运行，src文件夹里是对应的c++源代码。

3.可以通过运行makeexe里的myqt1.exe,并输入自己服务器的IP，以及端口号12345（注意端口号一定要是这个，设置服务器的时候也要开这个端口） 用户名随意，直接连接到服务器，进行测试。

一些缺陷:   
在多人传输文件时，会出现一些掉包的bug，后续考虑使用队列缓存解决这个问题。
