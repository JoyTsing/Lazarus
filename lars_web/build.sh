#!/bin/sh
if [ -e ./larsWeb ]; then
  rm larsWeb
fi
go build -o larsWeb && echo "web build successful" || echo "web build failed"
