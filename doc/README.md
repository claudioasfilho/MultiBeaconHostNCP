# MultiBeacon Host Mode


This demonstrate how to integrate an iBeacon and an Eddystone Beacon to the same advertisement set, running on a Host device connected to a NCP Bluetooth device.

Tools and SDKs:

This was tested with:

Silicon Labs Bluetooth Stack 2.13.3.0


GNU GCC Compiler: Apple LLVM version 9.0.0 (clang-900.0.39.2)
Target: x86_64-apple-darwin16.7.0
Thread model: posix


Development Board: Silicon Labs WSTK with BRD4159A (EFR32BG13P632F512GM48)

Usage:

In order to avoid modifying the Make File, please clone this to the following folder on the Bluetooth SDK structure.

/Applications/Simplicity Studio.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v2.x/app/bluetooth/examples_ncp_host/

It should compile as is.

In order to execute it, please use the following command:

./exe/MultiBeaconHostNCP /dev/tty.(Serial Port) 115200

If you desire to use this with a different Host, you will need to modify the Makefile in order to accommodate the cross compiler.

Target Device Images:

Both the NCP and Stand Alone Bootloader images can be found on the File Structure.

Stand Alone Bootloader - StdBootloaderUartBgapiBG13-combined.s37
Bluetooth NCP Image - ncp-empty-target-brd4104a-iar.hex
