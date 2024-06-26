## load balance

```
$$\                           $$\ $$\                 $$\                                         
$$ |                          $$ |$$ |                $$ |                                        
$$ | $$$$$$\   $$$$$$\   $$$$$$$ |$$$$$$$\   $$$$$$\  $$ | $$$$$$\  $$$$$$$\   $$$$$$$\  $$$$$$\  
$$ |$$  __$$\  \____$$\ $$  __$$ |$$  __$$\  \____$$\ $$ | \____$$\ $$  __$$\ $$  _____|$$  __$$\ 
$$ |$$ /  $$ | $$$$$$$ |$$ /  $$ |$$ |  $$ | $$$$$$$ |$$ | $$$$$$$ |$$ |  $$ |$$ /      $$$$$$$$ |
$$ |$$ |  $$ |$$  __$$ |$$ |  $$ |$$ |  $$ |$$  __$$ |$$ |$$  __$$ |$$ |  $$ |$$ |      $$   ____|
$$ |\$$$$$$  |\$$$$$$$ |\$$$$$$$ |$$$$$$$  |\$$$$$$$ |$$ |\$$$$$$$ |$$ |  $$ |\$$$$$$$\ \$$$$$$$\ 
\__| \______/  \_______| \_______|\_______/  \_______|\__| \_______|\__|  \__| \_______| \_______|

```

![](/img/lb.png)

* `UDP Server`服务，并运行LB算法，对业务提供节点获取和节点调用结果上报服务；为了增大系统吞吐量，使用3个UDP Server服务互相独立运行LB算法：modid+cmdid % 3 = i的那些模块的服务与调度，由第i+1个UDP Server线程负责

* `Dns Service Client`：是dnsserver的客户端线程，负责根据需要，向dnsserver获取一个模块的节点集合（或称为获取路由）；UDP Server会按需向此线程的MQ写入获取路由请求，DSS Client将MQ到来的请求转发到dnsserver，之后将dnsserver返回的路由信息更新到对应的UDP Server线程维护的路由信息中

* `Report Service Client`：是reporter的客户端线程，负责将每个模块下所有节点在一段时间内的调用结果、过载情况上报到reporter Service端，便于观察情况、做报警；本身消费MQ数据，UDP Server会按需向MQ写入上报状态请求。