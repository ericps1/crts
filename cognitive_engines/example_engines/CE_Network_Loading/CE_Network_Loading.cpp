#include "CE_Network_Loading.hpp"

#define DEBUG 0
#if DEBUG == 1
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) /*__VA_ARGS__*/
#endif

// constructor
CE_Network_Loading::CE_Network_Loading(int argc, char **argv, ExtensibleCognitiveRadio *_ECR) {
  
  ECR = _ECR;

  // bandwidths are initialized to be evenly allocated
  active_bw_cfg_req = false;
  net_saturated = false;
  bw_cfg = 1;
  tx_bw_cfg_req = 1;

  t_settle = timer_create();
  timer_tic(t_settle);

  // define subcarrier allocation 0
  for (int i = 0; i < num_sc; i++) {
    if ((i+1 > num_sc/2 - guard_sc_0) && (i < num_sc/2 + guard_sc_0))
      sc_alloc_0[i] = OFDMFRAME_SCTYPE_NULL;
    else if (abs((int)((float)num_sc/2.0 - (float)i - 0.5))%sc_pilot_freq == 0)
      sc_alloc_0[i] = OFDMFRAME_SCTYPE_PILOT;
    else
      sc_alloc_0[i] = OFDMFRAME_SCTYPE_DATA;
  }

  // define subcarrier allocation 1
  for (int i = 0; i < num_sc; i++) {
    if ((i+1 > num_sc/2 - guard_sc_1) && (i < num_sc/2 + guard_sc_1))
      sc_alloc_1[i] = OFDMFRAME_SCTYPE_NULL;
    else if (abs((int)((float)num_sc/2.0 - (float)i - 0.5))%sc_pilot_freq == 0)
      sc_alloc_1[i] = OFDMFRAME_SCTYPE_PILOT;
    else
      sc_alloc_1[i] = OFDMFRAME_SCTYPE_DATA;
  }

  // define subcarrier allocation 2
  for (int i = 0; i < num_sc; i++) {
    if ((i+1 > num_sc/2 - guard_sc_2) && (i < num_sc/2 + guard_sc_2))
      sc_alloc_2[i] = OFDMFRAME_SCTYPE_NULL;
    else if (abs((int)((float)num_sc/2.0 - (float)i - 0.5))%sc_pilot_freq == 0)
      sc_alloc_2[i] = OFDMFRAME_SCTYPE_PILOT;
    else
      sc_alloc_2[i] = OFDMFRAME_SCTYPE_DATA;
  }
}

// destructor
CE_Network_Loading::~CE_Network_Loading() {
  timer_destroy(t_settle);
}

// execute function
void CE_Network_Loading::execute() {
 
  // Update the status of network saturation based on thresholds
  // with hysteresis
  int queued_bytes = ECR->get_tx_queued_bytes();
  //printf("Queued Bytes: %i\n", queued_bytes);
  if ((!net_saturated) && (queued_bytes > sat_thresh_upper)){
    dprintf("Transmitter saturated\n");
    net_saturated = true;
  }
  else if (net_saturated && (queued_bytes < sat_thresh_lower)){
    net_saturated = false;
    dprintf("Transmitter no longer saturated\n");
  }

  // Request more bandwidth if all of the following conditions are met:
  // The network load is saturating our transmission, the bandwidth being 
  // used can be increased, and we are not already requesting increased 
  // bandwidth.
  if (net_saturated && (bw_cfg < 2) && (!active_bw_cfg_req) && 
      (timer_toc(t_settle)>settle_period)){
    active_bw_cfg_req = true;
    tx_bw_cfg_req = bw_cfg + 1;
    timer_tic(t_settle);
    dprintf("Requesting increased bandwidth to cfg %i\n", tx_bw_cfg_req);
    memcpy(tx_control_info, (void*)&tx_bw_cfg_req, sizeof(tx_bw_cfg_req));
    ECR->set_tx_control_info(tx_control_info);
  }

  // Remove request for increased bandwidth if the network is no longer
  // saturated
  if ((!net_saturated) && active_bw_cfg_req) {
    active_bw_cfg_req = false;
    tx_bw_cfg_req = bw_cfg;
    dprintf("Removing request for increased bandwidth\n");
    memcpy(tx_control_info, (void*)&tx_bw_cfg_req, sizeof(tx_bw_cfg_req));
    ECR->set_tx_control_info(tx_control_info);
  }

  // Handle bandwidth requests from other nodes
  if ((ECR->CE_metrics.CE_event == ExtensibleCognitiveRadio::PHY_FRAME_RECEIVED) &&
      (ECR->CE_metrics.control_valid)){
    ECR->get_rx_control_info(rx_control_info);
    memcpy(&rx_bw_cfg_req, (void*)rx_control_info, sizeof(rx_bw_cfg_req));
  
    // The other node has requested more bandwdith than currently allocated
    if (rx_bw_cfg_req > (2-bw_cfg)) {
      
      // Send control frames to grant the request if our transmitter is 
      // currently not saturated or if we are currently using more bandwidth
      // than the other node
      if (((!net_saturated) || (bw_cfg == 2))) {
        dprintf("Granting request for bandwidth cfg %i\n", rx_bw_cfg_req);
        bw_cfg--;
        tx_bw_cfg_req--;
        memcpy(tx_control_info, (void*)&tx_bw_cfg_req, sizeof(tx_bw_cfg_req));
        ECR->set_tx_control_info(tx_control_info);
        for (int i=0; i<3; i++)
          ECR->transmit_control_frame(NULL, 0);

        // Update tx/rx configuration
        set_bw_cfg(ECR, bw_cfg);
      }
    }
  }

  // If the other node has granted our request
  if ((ECR->CE_metrics.CE_frame == ExtensibleCognitiveRadio::CONTROL) && active_bw_cfg_req && 
      (rx_bw_cfg_req == 2-tx_bw_cfg_req)){
    dprintf("Request for increased bandwidth has been granted\n");
    active_bw_cfg_req = false;
    bw_cfg = tx_bw_cfg_req;
    set_bw_cfg(ECR, bw_cfg);
  }
}

void CE_Network_Loading::set_bw_cfg(ExtensibleCognitiveRadio *ECR, int bw_cfg) {
 
  dprintf("BW cfg set to: %i\n", bw_cfg);
  
  // Update the tx/rx parameters
  switch (bw_cfg) {
    case (0):
      ECR->set_rx_subcarrier_alloc(sc_alloc_2);
      ECR->set_tx_subcarrier_alloc(sc_alloc_0);
      ECR->set_tx_gain_uhd(15.0);
      break;
    case (1):
      ECR->set_rx_subcarrier_alloc(sc_alloc_1);
      ECR->set_tx_subcarrier_alloc(sc_alloc_1);
      ECR->set_tx_gain_uhd(18.0);
      break;
    case (2):
      ECR->set_rx_subcarrier_alloc(sc_alloc_0);
      ECR->set_tx_subcarrier_alloc(sc_alloc_2);
      ECR->set_tx_gain_uhd(21.0);
      break;
  }
}
