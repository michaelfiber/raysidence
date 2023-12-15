#!/usr/bin/env bash

set -Eeuo pipefail

source ./creds.sh

while :; do
    curl -s "$WEATHER_URL" | jq -r '(.current.temp|tostring) + "°F, " + .current.weather[0].main' > temp.txt
    sleep 240
done
