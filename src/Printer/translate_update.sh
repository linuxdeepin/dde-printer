#!/bin/bash

cd $(dirname $0)

ts_list=(`ls translations/*.ts`)

for ts in "${ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    lupdate  Printer.pro -ts "${ts}"
done
