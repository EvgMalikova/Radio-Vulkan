General Concept
===============

The project targets to unify the approach to offscreen and interactive hybrid rendering for both rasterization and ray-tracing pipelines.
Proposed Input prameters are:

1) Input data (bin or fits file)

2) Rendering type: interactive, saving to image, offscreen streaming

3) Type of pipeline: ray-tracing/rasterization

+

4) Extracted information about graphic hardware and system configuration (support of ray-tracing, number of devices connected via nvLink, mpi nodes available, etc.)


Output: vkImage that is used for binary array extraction with saving to disc or streaming or as an input to interactive rendering.

The rest is expected to be as hidden as possible.
