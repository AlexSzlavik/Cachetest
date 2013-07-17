#!/bin/sh
#a function to move the cores to userspace and set the cpu frequency
set_cpu_freq() {
	if [ $# -lt 2 ] 
	then
		echo "ERROR: function set_cpu_freq accepts 2 parameters: cpu# freq."  >&2
		exit 1;
	fi

	#get CPU number and frequency
	cpu=$1
	freq=$2

	#check if the cpu is online 
	if  grep -q 0  /sys/devices/system/cpu/cpu$cpu/online 
	then
		#enable core
		echo 1 > /sys/devices/system/cpu/cpu$cpu/online 
	fi

    #check if it is possible to change cpu configurations
    if [ ! -e /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_governor ]            
	then
                echo "ERROR: It is not possible to change cpu$cpu configurations."  >&2
                exit 1
        fi 

    #check if we can move the cpu to userspace
    if ! grep -q 'userspace' /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_available_governors 
	then 
                echo "ERROR: There is no userspace governor available for cpu$cpu" >&2
                exit 1
        fi
        
    #check if the provided frequency is in range 
    if ! grep -q $freq /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_available_frequencies
	then
		echo "ERROR: Provided frequency is not in the acceptable range for cpu$cpu" >&2
                exit 1
	fi
	
	#move cpu to userspace
	echo 'userspace' > /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_governor
	
	#update the cpu frequency	
	echo "$freq" > /sys/devices/system/cpu/cpu$cpu/cpufreq/scaling_setspeed

	#print out the info
	echo -e "\e[00;32mCore $cpu\e[00m is online, running at $freq"	
}

#Function to pin all IRQs in /proc/irq/## to the specified core in $1
set_IRQ_core() {
	if [ $# -ne 1 ]; then
		echo "ERROR: set_IRQ_core requires exactly one argument." 
		return 1;
	fi

	#Disable the IRQ Balancer

	core_id=$1

	#This will pin all IRQs which are possible to be pinned
	#Some might not be able to be changed, even if it's to the same core
	for i in `ls -d /proc/irq/*/`; do 
		(echo $core_id > ${i}smp_affinity) 2> /dev/null;
		if [ $? -ne 0 ]; then
			echo "WARNING: Couldn't pin IRQ `basename ${i}` to core $core_id" >&2
		fi
	done
}

move_PIDs_to_core() {
	if [ $# -ne 1 ]; then
		echo "ERROR: move_PIDs_to_core requires target core ID"
		return 1;
	fi

	core_id=$1

	#Move all PIDs in the system to core_id
	for i in `ps -e | awk '/[0-9]*/{print $1}'`;do
		taskset -pc $core_id $i &> /dev/null;
	done
	return 0
}

