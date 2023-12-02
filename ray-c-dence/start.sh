while :; do
    curl -s http://$HUE_SERVER.local/api/$HUE_KEY/groups | jq -r 'to_entries | .[] | .key + ":" + .value.name' > groups.txt
    ./main
done