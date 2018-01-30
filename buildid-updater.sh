#!/usr/bin/env bash

rm $1
echo "#define BUILD_ID "$(date -u +'"3.0b-%Y%m%d%H%M%S"') > $1