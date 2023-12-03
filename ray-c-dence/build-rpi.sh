#!/usr/bin/env bash

set -Eeuo pipefail

gcc -o main main.c -lraylib -lm -std=c99 -lGL -ldrm -lgbm -lEGL -lGLESv2