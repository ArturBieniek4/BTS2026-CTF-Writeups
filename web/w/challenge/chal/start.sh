#!/bin/sh
ADMIN_PASSWORD=$(node -e "console.log(require('crypto').randomBytes(16).toString('hex'))")
FLAG='BtSCTF{fake_flag}'
export FLAG
export ADMIN_PASSWORD
node bot.js &
node server.js
