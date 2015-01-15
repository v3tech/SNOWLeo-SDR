#!/bin/sh

route add -host 255.255.255.255 dev eth0
route add -net 224.0.2.15 netmask 255.255.255.0 dev eth0
echo "Starting Zing axi-dma Application..."

cd /mnt/
insmod v3best-rx.ko
insmod v3best-tx.ko

./snowleosdr_ctl.elf &
./snowleosdr_rx.elf &
./snowleosdr_tx.elf &