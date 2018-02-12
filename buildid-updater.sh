#!/usr/bin/env bash

rm $1
echo "#define BUILD_ID "$(date -u +'"3.1b-%Y%m%d%H%M%S"') > $1