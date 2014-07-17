#!/bin/sh
# Fix to restart wacom drivers after suspend
# Taken from https://bugs.launchpad.net/ubuntu/+source/linux/+bug/1251441

touch /etc/pm/sleep.d/wacomsuspendfix

cat << EOF > /etc/pm/sleep.d/wacomsuspendfix
#!/bin/sh
# Fix to restart wacom drivers after suspend
case "\${1}" in
    resume|thaw)
  udevadm trigger --action=change
  ;;
esac
EOF

chmod +x /etc/pm/sleep.d/wacomsuspendfix