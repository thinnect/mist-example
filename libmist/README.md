# Mist middleware library

# Introduction

TODO intro to libmist.

# Spontaneous events

Some data sampling and transmission activities are controlled by rules applied
to a device, but others are transmitted as they occur - spontaneously. Spontaneous
events may start from changes in the environment, but in some cases are also
software generated. Spontaneous resource events are transmitted using broadcast
and depending on the underlying network configuration, may be prioritized higher
than other data.

## Spontaneous event backoff configuration

In order to control the impact of high-priority spontaneous events, the middleware
applies a default backoff scheme to all spontaneous resource events. The backoff
time increases with subsequent events if at least 2 minutes have not passed
between the events. The second event in a series is allowed after at least 10
seconds have passed, the third after another 30 have passed and so on. The
default backoff values are: 0, 10, 30, 70, 120.

The backoff value can be adjusted for each spontaneous event source with the
`mist_configure_spontaneous_event_backoff(min, max, step)` function.

The backoff value is calculated with the formula:
    `backoff = MIN(MAX(step * ((1 << events) - 1), minimum), maximum)`
If more than 2 times the  maximum time has passed since the last value, events
is reset to 0. The smallest possible `backoff_minimum` value is 1, the default
step value is 10, the default maximum value is 120.
