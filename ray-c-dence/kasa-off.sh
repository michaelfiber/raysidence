#!/usr/bin/env bash

set -Eeuo pipefail

source creds.sh

while IFS= read -r line; do
    echo "Processing: $line"
    $KASA_BIN --host $line --type plug off
done <<< $($KASA_BIN --json | jq -r '. | to_entries | .[] | .key')