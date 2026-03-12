#!/usr/bin/bash

WIDTH=$(tput cols)
HEIGHT=$(tput lines)

args=(-w "$WIDTH" -h "$HEIGHT")

while getopts "w:h:s:f:" opt; do
    case $opt in
        w) args=(-w "$OPTARG" "${args[@]:2}") ;;
        h) args=(-w "${args[1]}" -h "$OPTARG") ;;
        s) args+=(-s "$OPTARG") ;;
        f) args+=(-f "$OPTARG") ;;
        *) exit 1 ;;
    esac
done

exec ./build/engine "${args[@]}"
