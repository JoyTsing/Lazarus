#!/bin/bash

LARS_REPORTER_PATH="./lars_reporter/build/src"
LARS_DNS_PATH="./lars_dns/build/src"
LARS_LBAGENT_PATH="./lars_loadbalance/build/src"
LARS_WEB_PATH="./lars_web"
LARS_API_EXAMPLE_PATH="./api/build/examples"
LARS_REACTOR_QPS_TEST_PATH="./lars_reactor/build/examples/qps_test"

usage() {
	echo ""
	echo "=======启动子系统=========="
	echo "Usage ./run_lazarus [reporter|dns|lbagent|web|test|reactor]"

	echo "=======Reactor测试工具============"
	echo "Usage ./run_lazarus reactor server"
	echo "Usage ./run_lazarus reactor qps [threadNum]"

	echo "=======LB Agent测试工具============"
	echo "Usage ./run_lazarus test qps ThreadNum"
	echo "Usage ./run_lazarus test example modid cmdid overload"
	echo "Usage ./run_lazarus test simulator ModID CmdID [errRate(0-10)] [queryCnt(0-999999)]"
}

# it just work
if [ "$1" = "test" ]; then
	if [ "$2" = "example" ]; then
		$LARS_API_EXAMPLE_PATH/example $3 $4 $5
	elif [ "$2" = "simulator" ]; then
		if [ $# -eq 4 ]; then
			$LARS_API_EXAMPLE_PATH/simulator $3 $4
		elif [ $# -eq 5 ]; then
			$LARS_API_EXAMPLE_PATH/simulator $3 $4 $5
		elif [ $# -eq 6 ]; then
			$LARS_API_EXAMPLE_PATH/simulator $3 $4 $5 $6
		else
			usage
		fi
	elif [ "$2" = "qps" ]; then
		$LARS_API_EXAMPLE_PATH/qps $3
	fi
elif [ "$1" = "reactor" ]; then
	if [ "$2" = "server" ]; then
		cd $LARS_REACTOR_QPS_TEST_PATH
		./qps_server
	elif [ "$2" = "qps" ]; then
		cd $LARS_REACTOR_QPS_TEST_PATH
		./qps_client $3
	fi
elif [ "$1" = "reporter" ]; then
	cd $LARS_REPORTER_PATH
	./reporter_server
elif [ "$1" = "dns" ]; then
	cd $LARS_DNS_PATH
	./dns_server
elif [ "$1" = "lbagent" ]; then
	cd $LARS_LBAGENT_PATH
	./loadbalance_agent
elif [ "$1" = "web" ]; then
	cd $LARS_WEB_PATH
	./larsWeb
elif [ "$1" = "help" ]; then
	usage
else
	usage
fi
