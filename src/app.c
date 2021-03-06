/***********************************************************************************************//**
 * \file   app.c
 * \brief  Event handling and application code for Empty NCP Host application example
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************>/

/* standard library headers */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

/* BG stack headers */
#include "bg_types.h"
#include "gecko_bglib.h"

/* Own header */
#include "app.h"
#include "infrastructure.h"

// App booted flag
static bool appBooted = false;

int8                rssi;
uint8               packet_type;
bd_addr             address;
uint8               address_type;
uint8               bonding;
uint8array          data;





enum {IBEACON_HANDLER, EDDYSTONE_HANDLER};


/**
 * @brief Function for creating a custom advertisement package
 *
 * The function builds the advertisement package according to Apple iBeacon specifications,
 * configures this as the device advertisement data and starts broadcasting.
 */
void iBeaconADV(void)
{
  /* This function sets up a custom advertisement package according to iBeacon specifications.
   * The advertisement package is 30 bytes long. See the iBeacon specification for further details.
   */

  static struct {
    uint8_t flagsLen;     /* Length of the Flags field. */
    uint8_t flagsType;    /* Type of the Flags field. */
    uint8_t flags;        /* Flags field. */
    uint8_t mandataLen;   /* Length of the Manufacturer Data field. */
    uint8_t mandataType;  /* Type of the Manufacturer Data field. */
    uint8_t compId[2];    /* Company ID field. */
    uint8_t beacType[2];  /* Beacon Type field. */
    uint8_t uuid[16];     /* 128-bit Universally Unique Identifier (UUID). The UUID is an identifier for the company using the beacon*/
    uint8_t majNum[2];    /* Beacon major number. Used to group related beacons. */
    uint8_t minNum[2];    /* Beacon minor number. Used to specify individual beacons within a group.*/
    uint8_t txPower;      /* The Beacon's measured RSSI at 1 meter distance in dBm. See the iBeacon specification for measurement guidelines. */
  }
  bcnBeaconAdvData
    = {
    /* Flag bits - See Bluetooth 4.0 Core Specification , Volume 3, Appendix C, 18.1 for more details on flags. */
    2,  /* length  */
    0x01, /* type */
    0x04 | 0x02, /* Flags: LE General Discoverable Mode, BR/EDR is disabled. */

    /* Manufacturer specific data */
    26,  /* length of field*/
    0xFF, /* type of field */

    /* The first two data octets shall contain a company identifier code from
        * the Assigned Numbers - Company Identifiers document */
       /* 0x004C = Apple */
       { UINT16_TO_BYTES(0x004C) },

       /* Beacon type */
       /* 0x0215 is iBeacon */
       { UINT16_TO_BYTE1(0x0215), UINT16_TO_BYTE0(0x0215) },

       /* 128 bit / 16 byte UUID */
       { 0xE2, 0xC5, 0x6D, 0xB5, 0xDF, 0xFB, 0x48, 0xD2, \
         0xB0, 0x60, 0xD0, 0xF5, 0xA7, 0x10, 0x96, 0xE0 },

       /* Beacon major number */
       /* Set to 34987 and converted to correct format */
       { UINT16_TO_BYTE1(34987), UINT16_TO_BYTE0(34987) },

       /* Beacon minor number */
       /* Set as 1025 and converted to correct format */
       { UINT16_TO_BYTE1(1025), UINT16_TO_BYTE0(1025) },

       /* The Beacon's measured RSSI at 1 meter distance in dBm */
       /* 0xC3 is -61dBm */
       0xC3
    };

  //
  uint8_t iBeaconlen = sizeof(bcnBeaconAdvData);
  uint8_t *pData = (uint8_t*)(&bcnBeaconAdvData);

 // struct gecko_msg_le_gap_bt5_set_mode_rsp_t *bt5_set_mode_response;

  /* Set 0 dBm Transmit Power */
  gecko_cmd_le_gap_set_advertise_tx_power(IBEACON_HANDLER, 0);

  /* Set custom advertising data using the Enhanced Advertising scheme */

  gecko_cmd_le_gap_bt5_set_adv_data(IBEACON_HANDLER, 0,iBeaconlen, pData);

  /* Set advertising parameters. 100ms advertisement interval. All channels used.
    * The first parameter defines the Advertiser Handle, the second two parameters are minimum and
    * maximum advertising interval, both in units of (milliseconds * 1.6).
    * The third parameter '7' sets advertising on all channels. */

  gecko_cmd_le_gap_set_advertise_timing(IBEACON_HANDLER, 160, 160, 0, 0);


   /* Start advertising in user mode and enable connections 2000 events before expiring ... just a random number*/
   //bt5_set_mode_response = gecko_cmd_le_gap_bt5_set_mode(0,le_gap_user_data, le_gap_non_connectable, 0,/*le_gap_non_resolvable*/ le_gap_identity_address);

   gecko_cmd_le_gap_start_advertising(IBEACON_HANDLER, le_gap_user_data, le_gap_non_connectable);

}


/*
 * This is how the Eddystone Beacon is Structured
 * Modify the URL to a custom URL and make sure
 * the Length parameter accommodates the number of characters
 * */

#define EDDYSTONE_DATA_LEN           (30)
static uint8_t eddystone_data[EDDYSTONE_DATA_LEN] = {

  0x03,          // Length of service list
  0x03,          // Service list
  0xAA, 0xFE,    // Eddystone ID
  0x10,          // Length of service data
  0x16,          // Service data
  0xAA,  0xFE,   // Eddystone ID
  0x10,          // Frame type Eddystone-URL
  0x00,          // Tx power
  0x00,          // http://www., 0x01=https://www.
  's','i','l','a','b','s','.','c','o','m'
};


void EddyStoneADV(void)
{
  /* EddyStone Beacon
     * Add the EddyStone beacon related commands.
     * We will use the same commands as in the iBeacon but with the Eddystone data and Handler
     * gecko_cmd_le_gap_bt5_set_adv_data(EDDYSTONE_HANDLER, 0,30, eddystone_data);
     * gecko_cmd_le_gap_bt5_set_adv_parameters(EDDYSTONE_HANDLER,160,160,7,0);
     * gecko_cmd_le_gap_bt5_set_mode(EDDYSTONE_HANDLER,le_gap_user_data, le_gap_non_connectable,0,le_gap_non_resolvable);
     *
     * */
  gecko_cmd_le_gap_bt5_set_adv_data(IBEACON_HANDLER, 0, EDDYSTONE_DATA_LEN, eddystone_data);
  gecko_cmd_le_gap_set_advertise_timing(EDDYSTONE_HANDLER,160,160,0,0);
  gecko_cmd_le_gap_start_advertising(EDDYSTONE_HANDLER,le_gap_user_data, le_gap_non_connectable);



}



/***********************************************************************************************//**
 *  \brief  Event handler function.
 *  \param[in] evt Event pointer.
 **************************************************************************************************/
void appHandleEvents(struct gecko_cmd_packet *evt)
{
  if (NULL == evt) {
    return;
  }

  // Do not handle any events until system is booted up properly.
  if ((BGLIB_MSG_ID(evt->header) != gecko_evt_system_boot_id)
      && !appBooted) {
#if defined(DEBUG)
    printf("Event: 0x%04x\n", BGLIB_MSG_ID(evt->header));
#endif
    usleep(50000);
    return;
  }

  /* Handle events */
  switch (BGLIB_MSG_ID(evt->header)) {
    case gecko_evt_system_boot_id:

        appBooted = true;
        printf("System booted. Starting iBeacon... \n");
        iBeaconADV();
        printf("Starting EddyStone Beacon... \n");
        EddyStoneADV();
        break;


    default:
      break;
  }
}
