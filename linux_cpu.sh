#!/bin/bash
VENDOR_ID=`lscpu | grep 'Vendor ID' | egrep -o '[^ ]*$'`
MODEL_NAME=`lscpu | grep 'Model name' | sed -n 's/Model name:\s*\(.*\)$/\1/p'`
SCALING_DRIVER=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_driver)
SCALING_GOVERNOR=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)
NO_TURBO_FILE="/sys/devices/system/cpu/intel_pstate/no_turbo"

echo "Driver: $SCALING_DRIVER, governor: $SCALING_GOVERNOR"
echo "Vendor ID: $VENDOR_ID\nModel name: $MODEL_NAME"

echo -n "Changing scaling_governor to performance: "
if ! sudo -n true 2>/dev/null; then echo ""; fi

sudo sh -c "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
if [[ $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor) == "performance" ]]; then
	echo "SUCCESS";
else
	echo "FAILED";
fi

if [ -e $NO_TURBO_FILE ]; then
	sudo sh -c "echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo"
	if [[ $(cat /sys/devices/system/cpu/intel_pstate/no_turbo) == 1 ]]; then
		echo "Succesfully disabled turbo boost using intel_pstate/no_turbo"
	else
		echo "Failed to disable turbo boost using intel_pstate/no_turbo"
	fi
fi
