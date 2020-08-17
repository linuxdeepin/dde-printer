#!/bin/bash
# this file is used to auto-generate .qm file from .ts file.
# author: shibowen at linuxdeepin.com

cd $(dirname $0)

lupdate -no-obsolete Deamon.pro

ts_list=(`ls translations/*.ts`)
for ts in "${ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    lupdate -no-obsolete  Deamon.pro -ts "${ts}"
    lrelease "${ts}"
done
