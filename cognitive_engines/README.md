# Example Cognitive Engines
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
channel power rather than based on reliability of the link.

## 4. CE\_FEC\_Adaptation

This CE determines which FEC scheme is appropriate based on the received signal quality
and updates its control information so that the transmitter will use the appropriate
scheme.

## 5. CE\_Mod\_Adaptation

This CE determines which modulation scheme is appropriate based on the received signal 
quality and updates its control information so that the transmitter will use the 
appropriate scheme.

## 6. CE\_Transparent

This CE makes no adaptations and is mostly used for debug.
