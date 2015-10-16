# Example Scenarios
## 1. Two\_Channel\_DSA

This simple DSA scenario assumes that there are two CRs operating in FDD
and with two adjacent and equal bandwidth channels (per link) that they 
are permitted to use. A nearby interferer will be switching between these
two channels on one of the links, making it necessary for the CR's to
dynamically switch their operating frequency to realize good performance.

## 2. Two\_Channel\_DSA\_PU

This simple DSA scenario assumes that there are two radios considered primary
users (PU) and two cognitive seconday user (SU) radios. There are two adjacent 
and equal bandwidth channels (per link) that the cognitive radios are permitted 
to use. The PU's will switch their operating frequency as defined in their
"cognitive engines," making it necessary for the SU CR's to dynamically switch
their operating frequency to realize good performance and to avoid significantly
disrupting the PU links.

## 3. Two\_Node\_FDD\_Network

This scenario creates the most basic two node CR network. No actual
cognitive/adaptive behavior is defined by the cognitive engines in
this scenario, it is intended as the most basic example for a user
to become familiar with CRTS.

## 4. Three\_Node\_HD\_Network

This scenario defines a 3 node CR network all operating on a single
frequency, making it half duplex. This was intended as a demonstration
of the networking interfaces of the ECR. Note that there is currently
no mechanism to regulate channel access e.g. CSMA.

## 5. FEC\_Adaptation

This example scenario defines two CR's that will adapt their transmit FEC
scheme based on feedback from the receiver. A dynamic interferer is introduced
to make adaptation more important.

## 6. Interferer\_Test

This scenario defines a single interferer (used for development/testing)

## 7. Mod\_Adaptation

This example scenario defines two CR's that will adapt their transmit modulation
scheme based on feedback from the receiver. A dynamic interferer is introduced
to make adaptation more important.
