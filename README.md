# remote controller
remote controller linux kernel driver and its user space control utility for Nuxitron set-top-box

Requirements:

- One remote controller should control both 'TV / panel' and set-top-box
- The remote controller should be the one which is used for set-top-box
- Whenever a new 'TV / panel' comes, it should be easily integrated to
the system by a technician
- It should provide blocking IO and select / poll support for applications

Before this design, all the logic had been implemented in a 16 bit NEC
microcontroller (MCU). Everytime a new 'TV / panel' appeared, it required
the attention of an engineer to find out the codes of keys and reengineer
the MCU firmware. Additionally, it lacked blocking IO and select / poll
support for applications. Therefore, it did not meet all the requirements
listed above and a new design was necessary.

What was done here is to split the functionality into two parts:
- Logic has been moved to CPU in the form of a linux kernel driver
- Bit-banging (Manchester and PWM coding) still resides in MCU

In this new design, the communication channel between CPU and MCU is I2C.
Configuration (codes of keys) are stored in EEPROM. Both CPU and MCU have
access to EEPROM but only CPU is allowed to update EEPROM. User space
utility is used to create codes of keys (panel data). It instructs the
user to which key he/she should press and then learns the key code through
the cooperation of between CPU and MCU. After that, they are saved
to a 'TV / panel' file.  Configuration can be programmed to EEPROM via
the linux kernel driver. Again, user space utility can be used for this
purpose.

In the TV mode, MCU sends all the keys to the TV (almost) and in the PC
mode, its sends all the keys to the CPU (almost). That state changes and
others (EEPROM updating, which should not happen often!) are handled by
the two sides with the help of a state machine.

In this new design, there should not be a need to add or change any code
unless a bug is exist. It is enough to know key codes of a new 'TV / panel'.
That can be easily achieved by a lay person when the utility program is used.
e.g. utility program instructs key 'OK' should be pressed, and when 'OK' is
pressed, it learns the key code automatically. It even learns compound keys
which are used to switch between different sources and changes with every
'TV / panel'. New design allowed a much cheaper 8 bit MCU to be used instead
of 16 bit NEC along with hitting all the targets set out for this project.

I designed the whole system with the help of Serkan Buyukabali (implemented
MCU side) and Atil Koprucu (hardware engineer) and implemented CPU side of it.

This repository only includes the CPU part of the design.

i2c.[c\h] -> user space utility
remote.[c\h] -> remote controller linux kernel driver (written for 2.6.11)
panel* -> Codes of keys for 'TV / panel'

