# Provided Scenarios
## 1. basic\_two\_node\_network (located in example\_scenarios/)

This scenario creates the most basic two node CR network. No actual
cognitive/adaptive behavior is defined by the cognitive engines in
this scenario, it is intended as the most basic example for a user
to become familiar with CRTS. Note how initial subcarrier allocation
can be defined in three ways. In this scenario, we use the standard
allocation method which allows you to define guard band subcarriers,
central null subcarriers, and pilot subcarrier frequency, as well as
a completely custom allocation method where we specify each subcarrier
or groups of subcarriers. In this example we use both methods to
create equivalent subcarrier allocations.

## 2. fec\_adaptation (located in example\_scenarios/)

This example scenario defines two CR's that will adapt their transmit FEC
scheme based on feedback from the receiver. A dynamic interferer is introduced
to make adaptation more important.

## 3. mod\_adaptation (located in example\_scenarios/)

This example scenario defines two CR's that will adapt their transmit modulation
scheme based on feedback from the receiver. A dynamic interferer is introduced
to make adaptation more important.

## 4. network\_loading (located in example\_scenarios/)

This example scenario sets up two CR nodes which have asymmetric network loads.
The network loads are then periodically swapped by the scenario controller. The
cognitive engines used in this scenario will adapt their bandwidths based on the
network loads that they detect.

## 5. python\_flowgraph\_example (located in example\_scenarios/)

This scenario demonstrates how to setup and use a cognitive radio written in python
rather than the ECR.

## 6. two\_channel\_dsa (located in example\_scenarios/)

This simple DSA scenario assumes that there are two CRs operating in FDD
and with two adjacent and equal bandwidth channels (per link) that they 
are permitted to use. A nearby interferer will be switching between these
two channels on one of the links, making it necessary for the CR's to
dynamically switch their operating frequency to realize good performance.

## 7. two\_channel\_dsa\_pu (located in example\_scenarios/)

This simple DSA scenario assumes that there are two radios considered primary
users (PU) and two cognitive seconday user (SU) radios. There are two adjacent 
and equal bandwidth channels (per link) that the cognitive radios are permitted 
to use. The PU's will switch their operating frequency as defined in their
"cognitive engines," making it necessary for the SU CR's to dynamically switch
their operating frequency to realize good performance and to avoid significantly
disrupting the PU links.

## 8. constant\_snr\_bandwidth\_sweep (located in test\_scenarios/)

This scenario sweeps bandwidth and transmit gain such that SNR is maintained for
each data point. Performance is logged to a csv file.

## 9. control\_and\_feedback\_test (located in test\_scenarios/)

This scenario exists as a simple means of verifying the ability of a scenario
controller to exert control over nodes in the test scenario and receive feedback
from them.

## 10. interferer\_test (located in test\_scenarios/)

This scenario defines a single interferer (used for development/testing)

## 11. performance\_sweep (located in test\_scenarios/)

This scenario performs a nested sweep of bandwidth, FEC, and modulation. Performance
is recorded in a csv file.

## 12. subcarrier\_alloc\_test (located in test\_scenarios/)

This example scenario just uses a single node to illustrate how subcarrier
allocation can be changed on the fly by the CE. If you run uhd\_fft on a
nearby node before running this scenario you can observe the initial
subcarrier allocation defined in the scenario configuration file followed
by switching between a custom allocation and the default liquid-dsp allocation.

## 13. throughput\_test (located in test\_scenarios/)

This scenario was created to validate the calculations used by the ECR.get\_tx\_data\_rate()
function. From several experiments the calculation seems fairly accurate assuming
perfect frame reception.

## 14. tx\_gain\_sweep (located in test\_scenarios/)

This scenarios sweeps the tx gain of two nodes and logs their performance to
a csv file.
