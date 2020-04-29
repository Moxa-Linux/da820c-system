# DA-820C system package

## base-system
Base system for DA-820C. 
It contains the necessary tools for setting up DA-820C

---

# DA-820C GPIO table

## Build the necessary drivers
### Build moxa-it87-gpio-driver
```bash=
git clone git@gitlab.syssw.moxa.com:MXcore-Package/moxa-it87-gpio-driver.git
cd moxa-it87-gpio-driver
make KRELEASE=$(uname -r) modules
make install
modprobe gpio-it87
```

### Build moxa-it87-serial-driver
```bash=
git clone git@gitlab.syssw.moxa.com:MXcore-Package/moxa-it87-serial-driver.git
cd moxa-it87-serial-driver
make KRELEASE=$(uname -r) modules
make install
modprobe it87_serial
```

### Build moxa-gpio-pca953x-driver
```bash=
git clone git@gitlab.syssw.moxa.com:MXcore-Package/moxa-gpio-pca953x-driver.git
cd moxa-gpio-pca953x-driver
make KRELEASE=$(uname -r) modules
make install
modprobe gpio-pca953x
```

### Build moxa-hid-ft260-driver
```bash=
git clone git@gitlab.syssw.moxa.com:MXcore-Package/moxa-hid-ft260-driver.git
cd moxa-hid-ft260-driver
make KRELEASE=$(uname -r) modules
make install
modprobe hid-ft260
```

---

## Export sys class gpio function
```bash=
init_gpio() {
	local gpio=${1}
	local direction=${2}
	local value=${3}
	local active_low=${4}

	if [ ! -e "/sys/class/gpio/gpio${gpio}" ]; then
		echo ${gpio} > "/sys/class/gpio/export"
	fi

	if [ "${direction}" == "out" ]; then
                echo ${direction} > "/sys/class/gpio/gpio${gpio}/direction"
                [ ! -z "${active_low}" ] && \
                        echo ${active_low} > "/sys/class/gpio/gpio${gpio}/active_low"
                [ ! -z "${value}" ] && \
                        echo ${value} > "/sys/class/gpio/gpio${gpio}/value"
	fi
}
```

## UART (on board)
### Initial UART GPIO pins
```bash=
# Port 0 (/dev/ttyM0)
init_gpio "449" "out" "0"
init_gpio "450" "out" "0"
init_gpio "451" "out" "0"

# Port 1 (/dev/ttyM1)
init_gpio "452" "out" "0"
init_gpio "453" "out" "0"
init_gpio "454" "out" "0"
```
### Set UART mode
```bash=
# Switch Port 0 to RS232 mode
echo 1 > /sys/class/gpio/gpio451/value
echo 0 > /sys/class/gpio/gpio449/value
echo 0 > /sys/class/gpio/gpio450/value
echo 0 > /sys/class/misc/it87_serial/serial1/serial1_rs485

# Switch Port 0 to RS485-2W
echo 0 > /sys/class/gpio/gpio451/value
echo 1 > /sys/class/gpio/gpio450/value
echo 0 > /sys/class/gpio/gpio449/value
echo 1 > /sys/class/misc/it87_serial/serial1/serial1_rs485

# Switch Port 0 to RS422/RS485-4W
echo 0 > /sys/class/gpio/gpio451/value
echo 0 > /sys/class/gpio/gpio450/value
echo 1 > /sys/class/gpio/gpio449/value
echo 1 > /sys/class/misc/it87_serial/serial1/serial1_rs485
```

```bash=
# Switch Port 1 to RS232 mode
echo 1 > /sys/class/gpio/gpio454/value
echo 0 > /sys/class/gpio/gpio452/value
echo 0 > /sys/class/gpio/gpio453/value
echo 0 > /sys/class/misc/it87_serial/serial2/serial2_rs485

# Switch Port 1 to RS485-2W
echo 0 > /sys/class/gpio/gpio454/value
echo 1 > /sys/class/gpio/gpio452/value
echo 0 > /sys/class/gpio/gpio453/value
echo 1 > /sys/class/misc/it87_serial/serial2/serial2_rs485

# Switch Port 1 to RS422/RS485-4W
echo 0 > /sys/class/gpio/gpio454/value
echo 0 > /sys/class/gpio/gpio452/value
echo 1 > /sys/class/gpio/gpio453/value
echo 1 > /sys/class/misc/it87_serial/serial2/serial2_rs485
```

## UART (on expansion module card, DN-SP08-I-TB)

### Bind USB-to-SMBUS driver ft260 and PCA9535 GPIO expander
```bash=
bind_ft260_driver(){
	for filename in /sys/bus/i2c/devices/i2c-*/name; do
		i2c_devname=$(cat ${filename})
		if [[ $i2c_devname == *"FT260"* ]]; then
			i2c_devpath=$(echo ${filename%/*})
			echo "pca9535 0x26" > ${i2c_devpath}/new_device
			echo "pca9535 0x27" > ${i2c_devpath}/new_device
		fi
	done
}
```

### Export GPIO for expansion module card
```bash=
export_batch_sysgpio() {
	local TARGET_GPIOCHIP=$1
	local GPIOCHIP_NAME=gpiochip
	local GPIO_FS_PATH=/sys/class/gpio
	local GPIO_EXPORT="export"

	if [ x"$2" == x"unexport" ]; then
		GPIO_EXPORT="unexport"
	fi

	# Export GPIOs
	ls $GPIO_FS_PATH | grep $GPIOCHIP_NAME | while read -r chip ; do
		GPIO_LABEL=$(cat $GPIO_FS_PATH/$chip/label)
		if [[ "$GPIO_LABEL" != *"$TARGET_GPIOCHIP"* ]]; then
			continue
		fi

		pinstart=$(echo $chip | sed s/$GPIOCHIP_NAME/\\n/g)
		count=$(cat $GPIO_FS_PATH/$chip/ngpio)
		for (( i=0; i<${count}; i++ )); do
			init_gpio $((${pinstart}+${i})) "out" "0"
		done
	done
}

export_batch_sysgpio "pca9535"
```
### GPIO table for expansion module card

#### For one Card
| Device Node | GPIO#0 | GPIO#1 | GPIO#2 | GPIO#3 |
| ----------  | ------ | ------ | ------ | ------ |
| /dev/ttyM2  |   432  |  433   |   434  |   435  |
| /dev/ttyM3  |   436  |  437   |   438  |   439  |
| /dev/ttyM4  |   440  |  441   |   442  |   443  |
| /dev/ttyM5  |   444  |  445   |   446  |   447  |
| /dev/ttyM6  |   416  |  417   |   418  |   419  |
| /dev/ttyM7  |   420  |  421   |   422  |   423  |
| /dev/ttyM8  |   424  |  425   |   426  |   427  |
| /dev/ttyM9  |   428  |  429   |   430  |   431  |

#### GPIO value of UART modes
|   UART mode    | GPIO#0 | GPIO#1 | GPIO#2 | GPIO#3 |
| -------------- | ------ | ------ | ------ | ------ |
| RS232          |   1    |    1   |    0   |    0   |
| RS485-2W       |   0    |    0   |    0   |    1   |
| RS422/RS485-4W |   0    |    0   |    1   |    0   |

#### Example for controlling UART mode on expansion module card
```bash=
# export all pca9535 gpio pins
export_batch_sysgpio "pca9535"

# to switch expansion module card A port 0 (/dev/ttyM2) to RS232 mode
echo 1 > /sys/class/gpio/gpio432/value
echo 1 > /sys/class/gpio/gpio433/value
echo 0 > /sys/class/gpio/gpio434/value
echo 0 > /sys/class/gpio/gpio435/value

# to switch expansion module card A port 0 (/dev/ttyM2) to RS485-2W mode
echo 0 > /sys/class/gpio/gpio432/value
echo 0 > /sys/class/gpio/gpio433/value
echo 0 > /sys/class/gpio/gpio434/value
echo 1 > /sys/class/gpio/gpio435/value

# to switch expansion module card A port 0 (/dev/ttyM2) to RS422/RS485-4W mode
echo 0 > /sys/class/gpio/gpio432/value
echo 0 > /sys/class/gpio/gpio433/value
echo 1 > /sys/class/gpio/gpio434/value
echo 0 > /sys/class/gpio/gpio435/value
```

## Digital IO
### Initial Digital IO GPIO pins
```bash=
# setup DIO output pins (DO0~DO1)
init_gpio "467" "out" "0"
init_gpio "468" "out" "0"

# export DIO input pins (DI0~DI5)
init_gpio "506"
init_gpio "507"
init_gpio "508"
init_gpio "509"
init_gpio "470"
init_gpio "471"
```
### Example for controlling Digital IO
```bash=
# set output high for DO0
echo 1 > /sys/class/gpio/gpio467/value
# set output low for DO1
echo 0 > /sys/class/gpio/gpio468/value

# get input from DI0 to DI5
cat /sys/class/gpio/gpio506/value
cat /sys/class/gpio/gpio507/value
cat /sys/class/gpio/gpio508/value
cat /sys/class/gpio/gpio509/value
cat /sys/class/gpio/gpio470/value
cat /sys/class/gpio/gpio471/value
```

## Programmable LED
### Initial Programmable LED GPIO pins
```bash=
# setup LED output value, active_low is [High]
# 496 is LED index 0, 497 is LED index 1, and so on
init_gpio "496" "out" "0" "1"
init_gpio "497" "out" "0" "1"
init_gpio "498" "out" "0" "1"
init_gpio "499" "out" "0" "1"
init_gpio "500" "out" "0" "1"
init_gpio "501" "out" "0" "1"
init_gpio "502" "out" "0" "1"
init_gpio "503" "out" "0" "1"
```

### Example for controlling Programmable LED
```bash=
# light on LED from index 0 to index 7
echo 1 > /sys/class/gpio/gpio496/value
echo 1 > /sys/class/gpio/gpio497/value
echo 1 > /sys/class/gpio/gpio498/value
echo 1 > /sys/class/gpio/gpio499/value
echo 1 > /sys/class/gpio/gpio500/value
echo 1 > /sys/class/gpio/gpio501/value
echo 1 > /sys/class/gpio/gpio502/value
echo 1 > /sys/class/gpio/gpio503/value

# light off LED from index 0 to index 7
echo 0 > /sys/class/gpio/gpio496/value
echo 0 > /sys/class/gpio/gpio497/value
echo 0 > /sys/class/gpio/gpio498/value
echo 0 > /sys/class/gpio/gpio499/value
echo 0 > /sys/class/gpio/gpio500/value
echo 0 > /sys/class/gpio/gpio501/value
echo 0 > /sys/class/gpio/gpio502/value
echo 0 > /sys/class/gpio/gpio503/value
```

## Relay
### Initial Relay GPIO pins
```bash=
# setup relay output value
init_gpio "493" "out" "0"
```
### Example for controlling Relay
```bash=
# enable relay
echo 1 > /sys/class/gpio/gpio493/value

# disable relay
echo 0 > /sys/class/gpio/gpio493/value
```
