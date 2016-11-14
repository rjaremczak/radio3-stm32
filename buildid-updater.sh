#!/usr/bin/env bash

rm $1
echo "#define BUILD_ID "$(date -u +'"0.1-SNAPSHOT-%Y%m%d%H%M%S"') > $1