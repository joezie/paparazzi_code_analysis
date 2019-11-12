#!/bin/bash
PAPARAZZI_PATH="/home/joezie/paparazzi"

if [ "$1" == "-h" ]; then
	find ${PAPARAZZI_PATH} -type f -name '*.h' > HeaderFileList.txt
elif [ "$1" == "-c" ]; then
	find ${PAPARAZZI_PATH} -type f -name '*.c' > SrcFileList.txt
elif [ "$1" == "-d" ]; then
	find ${PAPARAZZI_PATH} -type d > DirList.txt
else
	echo -e "Usage: ListFiles.sh <options>\n-h: list all header files in paparazzi project\n-c: list all source files in paparazzi project\n-d: list all directories in paparazzi project\n"
fi
