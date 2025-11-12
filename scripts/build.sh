#!/bin/bash

set -e

PIO_ENVIRONMENT=""

function parseArgs() {
    while [[ "$#" -gt 0 ]]; do
        case "$1" in
            --env|-e)
                PIO_ENVIRONMENT="${2}"
                shift 2
                ;;
            *)
                echo "Unrecognized argument ${1}"
                shift
                ;;

        esac
    done
}

parseArgs "$@"

if [[ -z "${PIO_ENVIRONMENT}" ]]; then
    echo "Platform IO environment is required. Exiting"
    exit 1
fi

echo "Building..."
pio run -e "${PIO_ENVIRONMENT}"

