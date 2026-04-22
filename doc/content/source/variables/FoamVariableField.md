# FoamVariableField

!syntax description /FoamVariables/FoamVariableField

## Overview

Permits shadowing of an OpenFOAM `volScalarField` using a constant monomial MooseVariable,
with data copied from OpenFOAM at the end of each OpenFOAM time step.
The underlying MOOSE variable (which has the same name)
can be transferred to other MOOSE apps like any other MOOSE variable.
Currently, only the boundary fields are copied.

## Example Input File Syntax

Below is mock-up example of a `FoamVariableField` and its transfer to a sub-app.

```
[FoamVariables]
    [temperature]
        type = FoamVariableField
        foam_variable = T
    []
[]
...
[Transfers]
    [to_subapp]
        type = MultiAppGeneralFieldNearestLocationTransfer
        source_variable = temperature
        to_multi_app = sub_app1
        variable = temp1
        execute_on = same_as_multiapp
    []
[]
```

!! Describe and include an example of how to use the FoamVariableField object.

!syntax parameters /FoamVariables/FoamVariableField

!syntax inputs /FoamVariables/FoamVariableField

!syntax children /FoamVariables/FoamVariableField
