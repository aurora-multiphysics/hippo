# FoamSideAdvectiveFluxIntegral

!syntax description /UserObjects/FoamSideAdvectiveFluxIntegral

An OpenFOAM equivalent of the [`SideAdvectiveFluxIntegral`](https://mooseframework.inl.gov/source/postprocessors/SideAdvectiveFluxIntegral.html),
which calculates the advected flux of a `foam_Variable` by the vector `advective_velocity`, which as a default of velocity vector `U`.


## Example Input File Syntax

To calculate the mass flow rate through a boundary patch, we want the side advective flux integral of the density

```
[Postprocessors]
    ...
    [m_dot]
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar=rho
        boundary = inlet
    []
[]
```


!syntax parameters /UserObjects/FoamSideAdvectiveFluxIntegral

!syntax inputs /UserObjects/FoamSideAdvectiveFluxIntegral

!syntax children /UserObjects/FoamSideAdvectiveFluxIntegral
