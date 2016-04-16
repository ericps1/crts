# Example Cognitive Engines

We have put together several example CE's to illustrate some of the features and
capabilities of the ECR. Users are encouraged to reference these CE's to get a better
understanding of the ECR and how they might want to design their own CE's, but should 
be aware that there is nothing optimal about these examples.

## 1. CE\_Two\_Channel\_DSA\_Link\_Reliability

This CE is intended for the 2 Channel DSA scenario. It operates by switching channels
whenever it detects that the link is bad, assuming the source of error to be from the
interferer. Once the decision is made at the receiver, the node will update control
information transmitted to the other node, indicating the new frequency it should
transmit on.

## 2. CE\_Two\_Channel\_DSA\_PU

This CE is used to create a primary user for the 2 Channel DSA PU scenario. The PU
will simply switch it's operating frequencies at some regular interval.

## 3. CE\_Two\_Channel\_DSA\_Spectrum\_Sensing

This CE is similar to the fist CE listed, but makes its adaptations based on measured
channel power rather than based on reliability of the link. The transmitter changes
its center frequency based on sensed channel power whereas the receiver will change
its center frequency when it has not received any frames for some period of time.

## 4. CE\_FEC\_Adaptation

This CE determines which FEC scheme is appropriate based on the received signal quality
and updates its control information so that the transmitter will use the appropriate
scheme. This is just a demonstration, no particular thought was put into the switching
points.

## 5. CE\_Mod\_Adaptation

This CE determines which modulation scheme is appropriate based on the received signal 
quality and updates its control information so that the transmitter will use the 
appropriate scheme. This is just a demonstration, no particular thought was put into
the switching points.

## 6. CE\_Template

This CE makes no adaptations but serves as a template for creating new CE's.

## 7. CE\_Subcarrier\_Alloc

This CE illustrates how a CE can change the subcarrier allocation of its transmitter.
The method for setting the receiver subcarrier allocation is identical.

## 8. CE\_Network\_Loading

When a pair of these CEs are communicating they will negotiate to adapt their occupied 
bandwidths based on the network loads they detect. Note that this example simply
shows bandwidth adaptation based on network load, the bands actually being used by the
radios do not overlap so they aren't really sharing spectrum.
