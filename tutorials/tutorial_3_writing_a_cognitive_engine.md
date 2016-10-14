# Tutorial 3: Writing a Cognitive Engine

In this tutorial we go through the procedure to define a new cognitive engine,
make it available to the ECR, and run a scenario with it. If you haven't
already, you may find it useful to review the documentation on the ECR and CE's 
found in the crts-manual.pdf.

Specifically, we'll be making a simple CE which calculates some statistics
and prints them out periodically. We'll also demonstrate how the CE can
exert control over the ECR's operation and observe how this impacts the
statistics. The statistics we will track include the number of received 
frames, the average error vector magnitude, the average packet error rate, 
and the average received signal strength indicator. We'll also modify the 
transmit gain periodically so that we'll observe some changes in the statistics 
over time. 

Move to the cognitive_engine directory in your local CRTS repository. Make 
copies of the cognitive engine template files. Note that in order to properly
integrate the CE into CRTS, the header and source files should begin with 'CE_'
and end in 'hpp' and 'cpp' respectively. This is done to identify the CE 
sources so that they can be integrated into the ECR code.

Run
\code{.sh}
  $ cd crts
\endcode

Then
\code{.sh}
  $ mkdir CE_Tutorial_3
  $ cp cognitive_engines/CE_Template/CE_Template.cpp /cognitive_engines/CE_Tutorial_3/CE_Tutorial_3.cpp
  $ cp cognitive_engines/CE_Template/CE_Template.hpp /cognitive_engines/CE_Tutorial_3/CE_Tutorial_3.hpp
\endcode

With these two files we will be defining a new class for our cognitive engine.
Edit both files so that each instance of 'CE_Template' is replaced with 
'CE_Tutorial_3'. Also edit the define statements at the top of the header file
from _CE_TEMPLATE_ to _CE_TUTORIAL_3_.

Now open up CE_Tutorial_3.hpp so we can add some necessary class members. We'll
need timers in order to know when to print the statistics out and update the
transmitter gain along with constants to represent how frequently this should 
be done and by how much the transmit gain should be increased. We'll also need 
a counter for the number of frames received and how many were invalid. Finally, 
we'll need to sum the error vector magnitude and received signal strength 
indicator. So in total we need to add the following members.

\code{.cpp}
  const float print_stats_period_s = 1.0;
  timer print_stats_timer;
  const float tx_gain_period_s = 1.0;
  const float tx_gain_increment = 1.0;
  time tx_gain_timer;
  int frame_counter;
  int frame_errs;
  float sum_evm;
  float sum_rssi;
\endcode

Now open up CE_Tutorial_3.cpp so we can implement our CE. First we need to
initialize all of our members in the constructor like so:

\code{.cpp}
  print_stats_timer = timer_create();
  timer_tic(print_stats_timer);
  tx_gain_timer = timer_create();
  timer_tic(tx_gain_timer);
  frame_counter = 0;
  frame_errs = 0;
  sum_evm = 0.0;
  sum_rssi = 0.0;
\endcode

Let's also make sure we clean up the timers in the destructor.

\code{.cpp}
  timer_destroy(print_stats_timer);
  timer_destrpy(tx_gain_timer);
\endcode

Now let's move on to the core of the CE, the execute function. Note that
the template has set up a generic structure to deal with each of the possible
events which can trigger the CE execution. So at this point we should be
considering what we want to happen for each event. We need to update
the class members to keep track of the statistics of interest. All of 
these statistics are based on received frames, so we should update them 
whenever a PHY event happens. Add the following code under the switch case
for PHY events.

\code{.cpp}
  frame_counter++;
  if (!ECR->CE_metrics.payload_valid)
    frame_errs++;
  sum_evm += pow(10.0, ECR->CE_metrics.stats.evm/10.0);
  sum_rssi += pow(10.0, ECR->CE_metrics.stats.rssi/10.0);
\endcode

Note that EVM and RSSI are reported in dB, but to acquire an average we
need to convert them to linear units.

We said that we wanted to print statistics every print_stats_period_s seconds. 
This doesn't depend on a particular event, so let's write this functionality 
in a block of code before the event switch. We want to check the elapsed time, 
print the statistics if enough time has elapsed, and then we'll need to reset 
the variables used to track statistics. We should also cover the case when zero
frames have been received. Something like the following should do the trick.

\code{.cpp}
  if(timer_toc(print_stats_timer) > print_stats_period_s){
    if (frame_counter>0) {
      printf("Updated Received Frame Statistics:\n");
      printf("  Frames Received: %i\n", frame_counter);
      printf("  Average EVM:     %f\n", 10.0*log10(sum_evm/(float)frame_counter));
      printf("  Average PER:     %f\n", (float)frame_errs/(float)frame_counter);
      printf("  Average RSSI:    %f\n\n", 10.0*log10(sum_rssi/(float)frame_counter));
    
      // reset timer and statistics
      timer_tic(print_stats_timer);
      frame_counter = 0;
      frame_errs = 0;
      sum_evm = 0.0;
      sum_rssi = 0.0;
    } else {
      printf("Updated Received Frame Statistics:\n");
      printf("  Frames Received: 0\n");
      printf("  Average EVM:     -\n");
      printf("  Average PER:     -\n");
      printf("  Average RSSI:    -\n\n");
    }
  }
\endcode

Note that we report EVM and RSSI in dB and so must apply another conversion.

Now that we have written code to track and display some statistics on the
received frames, let's make a modification to the ECR's transmission so 
we can observe changes in the statistics over time. We need to make sure 
that the gain stays within the possible values, something like below would 
work. This should be placed above the event switch, just like the other 
timer-based code.

\code{.cpp}
  if(timer_toc(tx_gain_timer) > tx_gain_period_s){
    timer_tic(tx_gain_timer);
    
    float current_tx_gain = ECR->get_tx_gain();
    if(current_tx_gain < 25.0)
      ECR->set_tx_gain(current_gain + tx_gain_increment);
    else
      ECR->set_tx_gain(0.0);
  }
\endcode

Now that we've established the desired functionality for our CE, we need to
configure CRTS so that we can use it, and recompile the code. This is 
accomplished simply by running the following from the CRTS root directory.

\code{.sh}
  $ ./config_cognitive_engines
  $ make
\endcode

You should see you newly defined cognitive engine appear in the list of
the included cognitive engines.

Next, we'll need to define a scenario that uses this new CE. Since we'll just
be using two CR's, simply copy the scenarios/test\_scenarios/basic\_two\_node\_network
file to wherever you might like within the scenarios directory e.g.:

\code{.sh}
  $ cd scenarios
  $ cp example_scenarios/basic_two_node_network.cfg tutorial_3.cfg
\endcode

Open up tutorial_3.cfg. At the very top let's change the run time to be a 
bit longer.

\code{.cpp}
  run_time = 60.0;
\endcode

Let's also edit both nodes to have an initial transmit gain of 0, and of
course we need to use our new CE. Let's also disable metric printing so we
can focus on the statistics we've used in our CE. Make the following changes 
for both nodes.

\code{.cpp}
  tx_gain = 0;
  CE = "CE_Tutorial_3";
  print_rx_frame_metrics = 0;
\endcode

Now we can run the scenario using the same procedure as in the first tutorial. 
Login to three nodes on your testbed.

On node 1 run:
\code(.sh}
  $ ./crts_controller -m -s tutorial_3
\endcode

On nodes 2 and 3:
\code{.sh}
  $ ./crts_cognitive_radio -a <controller ip>
\endcode

You should see updated statistics being printed to the screen once every
second on nodes 2 and 3. You should further observe decreasing EVM and
increasing RSSI. Note that depending on the distance between the two 
nodes you may not detect frames at the lower gain settings or you might 
have distortion/clipping issues at the higher gains levels.

If you are having troubles, here are the completed files that you can compare
against.

// CE_Tutorial_3.hpp
\code{.cpp}
#ifndef _CE_TUTORIAL_3_
#define _CE_TUTORIAL_3_

#include "CE.hpp"
#include "timer.h"

class CE_Tutorial_3 : public Cognitive_Engine {

private:
  // internal members used by this CE
  const float print_stats_period_s = 1.0;
  timer print_stats_timer;
  const float tx_gain_period_s = 1.0;
  const float tx_gain_increment = 1.0;
  timer tx_gain_timer;
  int frame_counter;
  int frame_errs;
  float sum_evm;
  float sum_rssi;

public:
  CE_Tutorial_3();
  ~CE_Tutorial_3();
  virtual void execute(ExtensibleCognitiveRadio *ECR);
};

#endif
\endcode

// CE_Tutorial_3.cpp
\code{.cpp}
#include "ECR.hpp"
#include "CE_Tutorial_3.hpp"

// constructor
CE_Tutorial_3::CE_Tutorial_3(int argc, char **argv, ExtensibleCognitiveRadio *_ECR) {

  // save the ECR pointer (this should not be removed)
  ECR = _ECR;

  print_stats_timer = timer_create();
  timer_tic(print_stats_timer);
  tx_gain_timer = timer_create();
  timer_tic(tx_gain_timer);
  frame_counter = 0;
  frame_errs = 0;
  sum_evm = 0.0;
  sum_rssi = 0.0;
}

// destructor
CE_Tutorial_3::~CE_Tutorial_3() {
  timer_destroy(print_stats_timer);
  timer_destroy(tx_gain_timer);
}

// execute function
void CE_Tutorial_3::execute(ExtensibleCognitiveRadio *ECR) {

  if (timer_toc(tx_gain_timer) > tx_gain_period_s) {
    timer_tic(tx_gain_timer);

    float current_tx_gain = ECR->get_tx_gain_uhd();
    if(current_tx_gain < 25.0)
      ECR->set_tx_gain_uhd(current_tx_gain + tx_gain_increment);
    else
      ECR->set_tx_gain_uhd(0.0);
  }

  if (timer_toc(print_stats_timer) > print_stats_period_s) {
    timer_tic(print_stats_timer);
    
    if (frame_counter>0) {
      printf("Updated Received Frame Statistics:\n");
      printf("  Frames Received: %i\n", frame_counter);
      printf("  Average EVM:     %f\n", 10.0*log10(sum_evm/(float)frame_counter));
      printf("  Average PER:     %f\n", (float)frame_errs/(float)frame_counter);
      printf("  Average RSSI:    %f\n\n", 10.0*log10(sum_rssi/(float)frame_counter));

      // reset statistics
      frame_counter = 0;
      frame_errs = 0;
      sum_evm = 0.0;
      sum_rssi = 0.0;
    } else {
      printf("Updated Received Frame Statistics:\n");
      printf("  Frames Received: 0\n");
      printf("  Average EVM:     -\n");
      printf("  Average PER:     -\n");
      printf("  Average RSSI:    -\n");
    }
  }

  switch(ECR->CE_metrics.CE_event) {
    case ExtensibleCognitiveRadio::TIMEOUT:
      // handle timeout events
      break;
    case ExtensibleCognitiveRadio::PHY:
      // handle physical layer frame reception events
      frame_counter++;
      if (!ECR->CE_metrics.payload_valid)
        frame_errs++;
      sum_evm += pow(10.0, ECR->CE_metrics.stats.evm/10.0);
      sum_rssi += pow(10.0, ECR->CE_metrics.stats.rssi/10.0);
      break;
    case ExtensibleCognitiveRadio::UHD_OVERFLOW:
      // handle UHD overflow events
      break;
    case ExtensibleCognitiveRadio::UHD_UNDERRUN:
      // handle UHD underrun events
      break;
    case ExtensibleCognitiveRadio::USRP_RX_SAMPS:
      // handle samples received from the USRP when simultaneously
      // running the receiver and performing additional sensing
      break;
  }
}
\endcode

// Tutorial_3.cfg
\code{.cpp}
// general scenario parameters
num_nodes = 2;
run_time = 60.0;

// Node 1
node1 : {
  // general node parameters
  node_type = "cognitive_radio";
  cognitive_radio_type = "ecr";
  server_ip = "192.168.1.38";

  // network parameters
  crts_ip = "10.0.0.2";
  target_ip = "10.0.0.3";
  net_traffic_type = "stream";
  net_mean_throughput = 2e6;

  // cognitive engine parameters
  cognitive_engine = "CE_Tutorial_3";
  ce_timeout_ms = 200.0;

  // log/report settings
  print_rx_frame_metrics = 0;
  log_phy_rx = 1;
  log_phy_tx = 1;
  log_net_rx = 1;
  log_net_tx = 1;
  generate_octave_logs = 1;

  // initial USRP settings
  rx_freq = 862.5e6;
  rx_rate = 2e6;
  rx_gain = 10.0;
  tx_freq = 857.5e6;
  tx_rate = 2e6;
  tx_gain = 0.0;

  // initial liquid OFDM settings
  tx_gain_soft = -12.0;
  tx_modulation = "bpsk"; 
  tx_crc = "crc32";
  tx_fec0 = "v27";
  tx_fec1 = "none";
  // tx_cp_len = 16;
  // rx_cp_len = 16;

  tx_subcarriers = 32;
  tx_subcarrier_alloc_method = "standard";
  tx_guard_subcarriers = 4;
  tx_central_nulls = 6;
  tx_pilot_freq = 4;

  rx_subcarriers = 32;
  rx_subcarrier_alloc_method = "standard";
  rx_guard_subcarriers = 4;
  rx_central_nulls = 6;
  rx_pilot_freq = 4;
};

// Node 2
node2 : {
  // general node parameters
  type = "cognitive_radio";
  cognitive_radio_type = "ecr";
  server_ip = "192.168.1.39";

  // virtual network parameters
  crts_ip = "10.0.0.3";
  target_ip = "10.0.0.2";
  net_traffic_type = "stream";
  net_mean_throughput = 2e6;

  // cognitive engine parameters
  cognitive_engine = "CE_Tutorial_3";
  ce_timeout_ms = 200.0;

  // log/report settings
  print_rx_frame_metrics = 0;
  log_phy_rx = 1;
  log_phy_tx = 1;
  log_net_rx = 1;
  log_net_tx = 1;
  generate_octave_logs = 1;

  // initial USRP settings
  rx_freq = 857.5e6;
  rx_rate = 2e6;
  rx_gain = 10.0;
  tx_freq = 862.5e6;
  tx_rate = 2e6;
  tx_gain = 0.0;

  // initial liquid OFDM settings
  tx_gain_soft = -12.0;
  tx_modulation = "bpsk";
  tx_crc = "crc32";
  tx_fec0 = "v27";
  tx_fec1 = "none";
  tx_delay_us = 1e3;
  // tx_cp_len = 16;
  // rx_cp_len = 16;

  tx_subcarriers = 32;
  tx_subcarrier_alloc_method = "custom";
  tx_subcarrier_alloc : {
    // guard band nulls
    sc_type_1 = "null";
    sc_num_1 = 4;

    // pilots and data
    sc_type_2 = "pilot";
    sc_type_3 = "data";
    sc_num_3 = 3;
    sc_type_4 = "pilot";
    sc_type_5 = "data";
    sc_num_5 = 3;
    sc_type_6 = "pilot";

    // central nulls
    sc_type_7 = "null";
    sc_num_7 = 6;

    // pilots and data
    sc_type_8 = "pilot";
    sc_type_9 = "data";
    sc_num_9 = 3;
    sc_type_10 = "pilot";
    sc_type_11 = "data";
    sc_num_11 = 3;
    sc_type_12 = "pilot";

    // guard band nulls
    sc_type_13 = "null";
    sc_num_13 = 4;
  }

  rx_subcarriers = 32;
  rx_subcarrier_alloc_method = "custom";
  rx_subcarrier_alloc : {
    // guard band nulls
    sc_type_1 = "null";
    sc_num_1 = 4;

    // pilots and data
    sc_type_2 = "pilot";
    sc_type_3 = "data";
    sc_num_3 = 3;
    sc_type_4 = "pilot";
    sc_type_5 = "data";
    sc_num_5 = 3;
    sc_type_6 = "pilot";

    // central nulls
    sc_type_7 = "null";
    sc_num_7 = 6;

    // pilots and data
    sc_type_8 = "pilot";
    sc_type_9 = "data";
    sc_num_9 = 3;
    sc_type_10 = "pilot";
    sc_type_11 = "data";
    sc_num_11 = 3;
    sc_type_12 = "pilot";

    // guard band nulls
    sc_type_13 = "null";
    sc_num_13 = 4;
  }
};
\endcode


