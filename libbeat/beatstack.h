/**
 * Simple management API around a containerized beatstack.
 *
 * Copyright Thinnect Inc. 2019
 * @author Raido Pahtma
 * @license <PROPRIETARY>
 */
#ifndef BEATSTACK_H_
#define BEATSTACK_H_

#include <stdbool.h>
#include <stdint.h>

#include "mist_comm.h"

/**
 * Callback function for netowork time offset changes.
 * @param offset - New network time offset.
 */
typedef void (*nw_time_changed_f) (uint32_t offset);

typedef struct
{
    uint32_t cluster_id;    // 16-bit timeslot number and 16-bit beat creator
    uint16_t router_addr;   // router address
    uint16_t partner_addr;  // partner address
    uint8_t  link_quality;  // partner link quality
} cluster_routers_t;

typedef struct
{
    uint8_t  rt_q_size;     // routing message queue size
    uint8_t  uc_tr_q_size;  // unicast transport message queue size
    uint8_t  bc_tr_q_size;  // broadcast transport message queue size
} beatstack_queue_sizes_t;


/**
 * Initialize and start beatstack.
 *
 * Further management should be done through the MistComm API.
 *
 * @param radio_address - the 16-bit AM radio address.
 * @param base - base communications layer that beatstack will use.
 * @param nwtc - Callback function to be called on network time changes.
 * @param file_sys_nr - File system number.
 *
 * @return A comms layer.
 */
comms_layer_t * beatstack_create (uint16_t radio_address,
                                  comms_layer_t * base,
                                  nw_time_changed_f nwtc,
                                  int file_sys_nr);

// -----------------------------------------------------------------------------

/**
 * Get beatstack version numbers MAJOR.MINOR.PATCH.
 */
uint8_t beatstack_version_major ();
uint8_t beatstack_version_minor ();
uint8_t beatstack_version_patch ();

/**
 * Get beatstack operating parameters.
 */
uint8_t beatstack_beats_in_cycle(); // BEATS_IN_CYCLE
uint8_t beatstack_nodes_in_beat();  // NODES_IN_BEAT
uint32_t beatstack_tmr_awake();     // TMR_AWAKE
uint32_t beatstack_tmr_inter();     // TMR_INTER_BEAT
int8_t beatstack_rx_dbm_acceptable(); // Minimum quality for forming relationships
int8_t beatstack_rx_dbm_threshold();  // Anthing below this is simply ignored

/**
 * Get beatstack current status.
 */
uint32_t beatstack_my_beat();

/**
 * Network time status.
 */
uint8_t beatstack_nw_time_priority();
uint32_t beatstack_nw_time_offset();
void beatstack_nw_time_set(uint32_t offset);

/**
 * Get cluster members.
 */
uint16_t * beatstack_cluster_members (uint8_t * p_members_count);
/**
 * Get cluster routers.
 */
cluster_routers_t * beatstack_cluster_routers (uint8_t * p_routers_count);
/**
 * Get node maximum queue sizes
 */
beatstack_queue_sizes_t * beatstack_queue_sizes ();
// -----------------------------------------------------------------------------

/**
 * Advanced use-case:
 * Intialize beatstack, container must already be configured.
 *
 * @param radio_address - the 16-bit AM radio address.
 * @param nwtc - Callback function to be called on network time changes.
 * @param file_sys_nr - File system number.
 *
 * @return A comms layer.
 */
comms_layer_t * beatstack_init (uint16_t radio_address,
                                nw_time_changed_f nwtc,
                                int file_sys_nr);

#endif//BEATSTACK_H_
