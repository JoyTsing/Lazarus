#!/bin/sh
if [ -e ./larsWeb ]; then
  rm larsWeb
fi
go build -o larsWeb
