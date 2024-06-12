## Traffic light control linux kernel module
This is a simple Linux kernel module that implements a traffic light using three LEDs. The logic is implemented based on a table-driven state machine. The code uses the new linux kernel descriptor-based GPIO interface.

### Prerequisites
Download raspberry pi kernel headers:
```bash
sudo apt install raspberrypi-kernel-headers
```
Create following symlink:
```bash
ln -s /usr/src/linux-headers-$(uname -r)/ /lib/modules/$(uname -r)/build
```

### Build and install
Build the module using make:
```bash
make
```
Load the module using `insmod`
```bash
sudo insmod traffic_light.ko
```

### Usage
Turn on the traffic light:
```bash
sudo sh -c 'echo 1 >  /sys/kernel/traffic_light_control/power'
```
Turn off the traffic light:
```bash
sudo sh -c 'echo 0 >  /sys/kernel/traffic_light_control/power'
```

### Configuration
The module assumes that the red, yellow, and green LEDs are connected to GPIO pins 14, 15, and 18, respectively. The default timing is as follows:
+ Green: 3 seconds
+ Yellow: 1 second
+ Red: 3 seconds

These settings can be changed by editing the code.

### Notes
+ Ensure that the GPIO pins are correctly connected to the corresponding LEDs.
+ Modify the timings and GPIO pins in the source code if different configurations are required.
