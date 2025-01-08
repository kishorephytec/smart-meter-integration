
#include "CONFIG_PROJECT_MAC.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tri_tmr.h"
#include "common.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "list_latest.h"
#include "sw_timer.h"
#include "phy.h"
#include "mac_config.h"
#include "mac.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "mac_pib.h"
#include "sm.h"
#include "fec.h"
#include "fan_mac_ie.h"
#include "timer_service.h"


/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/
/*Umesh : 02-01-2018*/
//static sw_tmr_t sw_bradcast_timer = {0};
/*this varriable was not used anywhere*/
static int32_t  ChannelTable [150] = {0};
/*set the number generator seed. Don't allow zero */
static uint32_t my_seed = 1;

/*
** =============================================================================
** Public Variable Definitions
** =============================================================================
**/

int32_t  HopSequenceTable [150] = {0};
uint16_t total_nos_of_channel=0x00;

//Below shows a sample output of the function for a fixed mac address.

//For tableLen-131 mac 00:13:50:04:00:00:05:f8 FirstElement=122(7a) StepSize=119(77)

uint8_t MACAddr[8] = {0}; 
/*

b.	For broadcast sequence computation, construct an 8 octet EUI-64 (byte[8]) as follows:
i.	Set byte[0] through byte[5] to 0.
ii.	Set byte[6] to the MSByte of the Broadcast Schedule Identifier.
iii.	Set byte[7] to the LSByte of the Broadcast Schedule Identifier. 

uint8_t MACAddr[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0xXX,0xYY};

*/

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/
#ifdef WISUN_FAN_MAC
extern self_info_fan_mac_t mac_self_fan_info;
#endif

/*
** ============================================================================
** External Function Prototypes
** ============================================================================
*/

#ifdef WISUN_FAN_MAC
extern void  get_self_extended_address_reverse (uint8_t *macAddr);
#endif
/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

/*None*/

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

uint8_t create_channel_list_for_tr51();

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/ 

void wisun_seed_rand(uint32_t seed) {
   if(seed == 0) {
     my_seed = 1;
   } else {
     my_seed = seed;
   }
}

/* The old standard glibc linear congruential RNG */
int32_t wisun_rand(void) {
      uint32_t val = ((my_seed * 1103515245) + 12345) & 0x7fffffff;
      my_seed = val;
      return val;
}

int check_prime(int x){
    int i = 0;
    for(i=2;x%i!=0;i++);
        if(x==i) {
            printf("\nNext prime is %d\n",x);
            return x;
        }
        else {
            return 0;
        }
}

/*
 *   n number of channels in band
 *   m nearset prime # equal to or larger than n
 *   chanTable where to put the result BETTER BE AT LEAST m in LENGTH
 */

void calcChanTable (uint16_t n, uint16_t m, int32_t * chanTable) 
{
    int32_t i = 0,j = 0,k = 0; // Names as per annex B in TR-51
    // wisun_rand() is seeded with the value 1
    wisun_seed_rand(1);
    // Each element of the Channel Table from is set to INVALID
    for (i=0; i < m; i++) {
        chanTable[i] = -1; // tag as invalid
    }
    for (i=0; i < n; i++) {
        j = wisun_rand() % n;
        // While element of the Table from 0 to i has the value j
        k=0;
        while (k <= i) { 
            if (j == chanTable[k]) { 
                j = wisun_rand() % n; 
                k=0; // new value of j so check that it's  not a repeat
            }
            else {
                k=k+1;
            }
        }
        chanTable[i] = j;
    }
}

/* reference Section 7.1 of the TR-51 specification */
/* mac is a set of 8 octets MSB to LSB containing the mac address */
void compute_CFD(uint8_t *mac, uint8_t *first, uint8_t *step , uint16_t chtbl_len) {
    *first = (mac[5] ^ mac[6] ^ mac[7]) % chtbl_len;
    *step = (mac[7] % (chtbl_len - 1)) + 1;
}



uint8_t create_channel_list_for_tr51()
{
	//int32_t  rand1 , rand2 , rand3, rand4;
        uint32_t current_max_channels=0x00;
        uint16_t length = 0;
        PLME_get_request( phyMaxSUNChannelSupported, &length, &current_max_channels );//current max channel according to phy mode
        total_nos_of_channel = current_max_channels;
        uint16_t Next_higher_prime_no;
	
	uint8_t firstElement = 0;
	uint8_t StepSize = 0;
	uint8_t CurrentElement = 0;
	uint8_t I = 0; // The Hopping Slot Number ..


	/*rand1 = wisun_rand();
	rand2 = wisun_rand();
	rand3 = wisun_rand();
	rand4 = wisun_rand();

	printf("\n RAND1 :: %d, RAND2 :: %d, RAND3 :: %d, RAND4 :: %d\n", rand1,rand2,rand3,rand4);*/
#ifdef WISUN_FAN_MAC         
        get_self_extended_address_reverse(MACAddr);
#endif        
	 for(Next_higher_prime_no=total_nos_of_channel+1;;Next_higher_prime_no++){
        if(check_prime(Next_higher_prime_no)){
            break;
        }
    }
	printf("\n Next Higher Prime no is  :: %d\n", Next_higher_prime_no);

	calcChanTable (total_nos_of_channel, Next_higher_prime_no, (int32_t* )&ChannelTable[0]);


	compute_CFD(&MACAddr[0], &firstElement, &StepSize , Next_higher_prime_no );

	/* Now  calculate the Channel shedule
		3.	Compute N, where N is the number of valid channels in the channel table.
		4.	Set CurrentElement to FirstElement.  Set I = 0.
		5.	Repeat the following N times to compute the channels for the N slots in the hop sequence
			a.	While ChannelTable[CurrentElement] == invalid,  increment CurrentElement by StepSize
			b.	Set the I-th channel in the hop sequence to ChannelTable[CurrentElement]. 
			c.	Increment I.

	*/
	CurrentElement = firstElement;
	I = 0;

	//HopSequenceTable
	for ( I = 0 ; I <=total_nos_of_channel; I++)
	{
		CurrentElement = ((CurrentElement + StepSize)% Next_higher_prime_no);
		
		if (ChannelTable[CurrentElement] != -1)
			HopSequenceTable[I] = ChannelTable[CurrentElement];

		
		
	}
	return 0;
}


