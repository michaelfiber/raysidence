#!/usr/bin/env bash

set -Eeuo pipefail

source creds.sh

# start temp updater in the background.
./update-temp.sh &

# Start the main program after curling the groups.
# this way the program can simply exit to force a reload of the groups too.
while :; do
    curl -s http://$HUE_SERVER.local/api/$HUE_KEY/groups | jq -r 'to_entries | .[] | .key + ":" + .value.name' > groups.txt
    ./main
done
