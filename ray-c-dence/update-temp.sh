#!/usr/bin/env bash

set -Eeuo pipefail

source ./creds.sh

while :; do
    curl -s wttr.in/$HOME_TOWN?format=j2 | jq -r '.current_condition[0].weatherDesc[0].value + ", " + .current_condition[0].temp_F + "°F"' > temp.txt
    sleep 1800
done
