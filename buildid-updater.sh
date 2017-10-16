#!/usr/bin/env bash

rm $1
echo "#define BUILD_ID "$(date -u +'"2.2-%Y%m%d%H%M%S"') > $1