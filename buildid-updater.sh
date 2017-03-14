#!/usr/bin/env bash

rm $1
echo "#define BUILD_ID "$(date -u +'"1.1-%Y%m%d%H%M%S"') > $1