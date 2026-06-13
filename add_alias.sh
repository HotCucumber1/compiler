#!/bin/bash

CURRENT_DIR=$(pwd)

ALIAS_NAME="stasis"
ALIAS_CMD="${CURRENT_DIR}/bin/compiler"

FULL_ALIAS="alias ${ALIAS_NAME}='${ALIAS_CMD}'"
BASHRC_PATH="${HOME}/.bashrc"

if grep -Fq "alias ${ALIAS_NAME}=" "$BASHRC_PATH"; then
    echo "Alias '${ALIAS_NAME}' is already exists. Skiped."
else
    echo "" >> "$BASHRC_PATH"
    echo "# Add from ${CURRENT_DIR}" >> "$BASHRC_PATH"
    echo "$FULL_ALIAS" >> "$BASHRC_PATH"

    echo "Alias '${ALIAS_NAME}' successfully added: ${ALIAS_CMD}"
    echo "To use it execute: source ~/.bashrc"
fi
