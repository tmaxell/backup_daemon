#!/bin/bash

g++ daemon.cpp -std=c++20 -O2 -o backupdaemon

DIR="/usr/local/bin/backupdaemon_service"
if [ ! -d "$DIR" ]; then
    mkdir $DIR
fi

cp backupdaemon config.yaml $DIR
chmod +x $DIR/backupdaemon

touch /etc/systemd/system/backupdaemon.service
cat << EOF > /etc/systemd/system/backupdaemon.service
[Unit]
Description=Backup Daemon Service
After=network.target
[Service] 
Type=simple
Restart=no
User=root
WorkingDirectory=/usr/local/bin/backupdaemon_service
ExecStart=/usr/local/bin/backupdaemon_service/backupdaemon
[Install]
WantedBy=multi-user.target
EOF

chmod 664 /etc/systemd/system/backupdaemon.service
systemctl daemon-reload
echo "Script finished"
