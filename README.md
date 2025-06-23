# Renormalize

McStas and McXtrace contain functionality to store particle-ray states in MCPL files.
Prior to `McCode` `v3.5.28` (and `MCPL` `v2.2.0`) the weights of the stored particles were normalized to 
the number of _starting_ particles in the simulation.
Under such conditions, the outputs of two simulations could not be combined by simply concatenating the MCPL files.
This package provides a simple command-line tool to renormalize the McCode-like weights while concatenating the files.

# Obsolescence
Following the release of `McCode` `v3.5.28` and `MCPL` `v2.2.0`, this package is now obsolete.