#!/bin/bash
PROJECT_DIR=$(pwd)
TARGET_DIRS=("lars_reactor" "lars_reporter" "lars_dns" "lars_loadbalance" "api" "lars_web")
for dir in "${TARGET_DIRS[@]}"; do
	pushd "$PROJECT_DIR"/"$dir" || exit
	./build.sh || exit
	popd || exit
done
