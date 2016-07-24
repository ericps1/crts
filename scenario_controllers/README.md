# Example Scenario Controllers

We have put together several example CE's to illustrate some of the features and
capabilities of the ECR. Users are encouraged to reference these CE's to get a better
understanding of the ECR and how they might want to design their own CE's, but should 
be aware that there is nothing optimal about these examples.

## 1. SC\_Template

This is the template SC which is used by default. In this case we don't want to assume
anything about the nodes involved in the test so we don't enable any feedback. Still
we include an example of how to handle feedback in the execute function.

## 2. SC\_Network\_Loading

This SC was made specifically for the Network\_Loading scenario. It simply modifies the
network throughput for two CR nodes at a specified time interval.

## 3. SC\_BER\_Sweep

This SC was made to perform BER measurements as a funciton of transmit and receive gains.

## 4. SC\_CORNET\_Tutorial

This SC was made as a way to interface CRTS and CORNET 3D. It simply forwards feedback
and control messages between CRTS nodes and the CORNET 3D server.

## 5. SC\_Performance\_Sweep\_Utility

This SC provides a flexible way to sweep the operating parameters of two CR nodes to
obtain performance curves. Configuration files can be created to specify the sweep
parameters and values. Both nested and stepped sweeping structures are supported.
