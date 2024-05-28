## dns_service

映射一些服务器主机的ID和真实主机IP的关系，DnsService服务模型采用了one loop per thread TCP服务器，主要是基于Lars-Reactor：
* 主线程Accepter负责接收连接（agent端连接）
* Thread loop们负责处理连接的请求、回复；（agent端发送查询请求，期望获取结果）

![DNS](/img/dns.png)