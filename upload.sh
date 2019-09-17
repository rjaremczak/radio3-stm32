#!/bin/bash
echo $PATH
JLinkExe -autoconnect 0 -device STM32F103C8 -if SWD -speed 1000 -commandfile load_flash.jlink
