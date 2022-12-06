#!/bin/sh
if echo "$1" | grep -Eq 'x86_64-'; then
  echo x86_64
else
  echo "$1" | grep -Eo '^[[:alnum:]_]*'
fi
