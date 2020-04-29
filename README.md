# mist-example

The Mist Example project demonstrates integration of the Thinnect mist
middleware library and in the future the integration of the mesh network and
security libraries.

The integrated reference action module implements a basic luminaire control
feature set. (*TODO* not functional currently).

# Mist subscriptions (rules)

Rules are instructions that are sent to the Mist nodes to be executed, rules
are the main way to direct the behavior of the Mist devices.

Mist protocol communications are handled by the library and the built-in
rule interpreter. Support for custom rule format interpreters is planned.
The user is able to register an action handler with the Mist middleware and
the middleware will call the handler according to the rule specification
with parameters extracted from the rule.
See [mist_middleware.h](libmist/mist_middleware.h) for details on the API
functions needed to register a handler.

This repository contains a binary distribution of the Mist middleware that can
be linked into the final application. See the [libmist](libmist) directory.
Currently only a release for the efr32xg21 family is available.

# Device Announcement

The device announcement protocol is intended to allow Mist nodes to let other
nodes know about their existence in the networks and their properties.
Therefore devices periodically broadcast announcement packets and respond to
queries about additional details.

The announcement messages are used by *coreserver* to discover new devices,
detect changes of properties and react to reboots (verify correct rules are applied).

# Setup
This repository relies on several dependencies that are all publically available
on [GitHub](https://github.com) and they have been linked to the repository as
submodules. After cloning the repository, the submodules need to be initialized
and updated.
```
git clone git@github.com:thinnect/mist-example.git
cd mist-example
git submodule init
git submodule update
```

Additionally some utilities and third-party libraries need to be installed,
follow the [node-apps INSTALL guide](https://github.com/thinnect/node-apps/blob/master/INSTALL.md).

Initially the submodules will be pointing to the correct commits/states of their
respective repositories, but in a detached-head state. The actual branches they
should be checked out to are listed in the [gitmodules](.gitmodules) file.
Install [git-submodule-gizmos](https://github.com/raidoz/git-submodule-gizmos)
to get a tool for easier management of the branches.

# Build and flash

Build with `make PLATFORM_NAME` and flash to the device with `make PLATFORM_NAME install`.

For example to install on a Thinnect TestSystemBoard2, run:
```
make tsb2
make tsb2 install
```

# Adding support for external platforms

Modify the PLATFORM_DIRS variable in the Makefile and add the path to the
location of the new platforms
`PLATFORMS_DIRS := $(ZOO)/thinnect.node-buildsystem/make $(ZOO)/thinnect.dev-platforms/make PATH_TO_MY_CUSTOM_PLATFORMS/make`

# MistComm
Radio communications are performed throught the
[MistComm](https://github.com/thinnect/mist-comm/tree/euis) communications API.
The API is still a prototype, but now supports global addressing
without having to know the network hierarcy. It uses IEEE EUI-64 identifiers for
identifying endpoints (device identity, not just network address). It includes
an address translation layer and EUI-64 and link-local addresses are resolved
automatically.
*The Mist Middleware relies on the EUI-64 addressing capabilities*

*The MistComm API is still likely to change*

# Coding rules

Code in this repository must conform to the
[BARR-C:2018](https://barrgroup.com/Embedded-Systems/Books/Embedded-C-Coding-Standard)
coding rules with the following permitted exceptions:
- Lines are allowed to be up to 120 characters long, keeping them shorter is
  however recommended.
- static module-level variables must be prefixed with m_, global ones with g_.
- TABs may be used for initial indentation (but not for alignment).
