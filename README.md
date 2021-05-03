# Distrac: A High-Performance Distributed Event-Based Tracing Tool

## Current State

The library is already working for small projects. One example
is [Paracooba](https://github.com/maximaximal/Paracooba). This also includes
an [example definition file](https://github.com/maximaximal/Paracooba/blob/master/Paracooba.distracdef).

## Future Goals

  - Refine saving traces to be thread-local and done in-memory.
  - Better causal-link-recovery using possibly a SMT solver
  - Improved analysis possibilities
  - Automatic network syncing
