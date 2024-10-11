# Simple MultiApp

This is a simple multiapp that runs for one time step,
that checks the initial temperature from a MOOSE mesh's boundary
is successfully transferred to the OpenFOAM boundary.

The MOOSE mesh is an L shape rotated 90 degrees.
The OpenFOAM mesh slots into the L,
hence there are two boundaries over which to transfer the temperature.

We have a serial and parallel version of the test.

```txt
    +--------------------+
    |     MOOSE mesh     |
    |                    |
    |       +------------+
    |       | Foam mesh  |
    |       |            |
    +-------+------------+
```
