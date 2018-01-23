#!/bin/bash

pids=$(ps x | grep kiwistrmd | grep -v grep | awk '{print $1}')
for pid in $pids; do
	echo "Killing kiwistrmd process ${pid}..."
	kill -9 $pid
done

