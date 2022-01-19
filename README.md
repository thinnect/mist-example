# mist-example

The Mist Example project demonstrates integration of the Thinnect mist
middleware library (libmist), the Thinnect mesh network library (libbeat)
and in the future the integration of security libraries.

The integrated reference action modules implement basic luminaire control
and motion detection features.

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

## Mist library setup

The mist library is linked to the firmware from `libmistmiddleware.a`. The
library needs to be initialized with `mist_middleware_init`, providing it a
pointer to an initialized radio layer. Initialization should be followed by
calls to `mist_register_handler` to add action handlers. After all handlers
have been registered, a call to `mist_middleware_start` will start the actual
mist middleware, loading stored rules and registering communication paths.
Handlers should be registered before start, because any rules loaded during
start that do not have a corresponding handler, are discarded.

## Persistent rule storage

The mist middleware stores rules that have an infinite timeout (UINT32_MAX) in
flash memory (TODO in the future other rules will also be stored in flash for
their duration).
Stored rules are marked with IDENT_TIMESTAMP and discarded automatically when
firmware is changed (IDENT_TIMESTAMP changes). The storage component uses
the [node-filesystem](https://github.com/thinnect/node-filesystem.git) wrapper
around [SPIFFS](https://github.com/pellepl/spiffs). The filesystem needs to be
initialized before `mist_middleware_start` is called.

# Device Announcement

The device announcement protocol is intended to allow Mist nodes to let other
nodes know about their existence in the networks and their properties.
Therefore devices periodically broadcast announcement packets and respond to
queries about additional details.

The announcement messages are used by *coreserver* to discover new devices,
detect changes of properties and react to reboots (verify correct rules are applied).

# Thinnect Mesh (libbeat)

The example application can be optionally built with the Thinnect mesh network
layer. The library needs to be obtained separately. The library bundle should
include a header `beatstack.h` and the static library `libbeat.a` for a given
architecture. These need to be stored as:
```
$(WORKSPACE_ROOT)/libbeat/beatstack.h
$(WORKSPACE_ROOT)/libbeat/$(MCU_ARCH)/libbeat.a
```

Additionally INCLUDE_BEATSTACK needs to be set to 1. This can be done in the
Makefile (or Makefile.private) or set on the command line:

```
make <PLAFTORM_NAME> INCLUDE_BEATSTACK=1
```

# Thinnect OTA (libota)

The example application can be optionally built with the Thinnect mesh update
layer. The library needs to be obtained separately. The library bundle should
include a header `updater.h` and the static library `libota.a` for a given
architecture. These need to be stored as:
```
$(WORKSPACE_ROOT)/libota/updater.h
$(WORKSPACE_ROOT)/libota/updater_fs.h/
$(WORKSPACE_ROOT)/libota/updater.h/
$(WORKSPACE_ROOT)/libota/updater_header.h/
$(WORKSPACE_ROOT)/libota/updater_lib.h
$(WORKSPACE_ROOT)/libota/$(MCU_ARCH)/libota.a
```

Additionally LIBOTA_CONFIG needs to be set to 1. This can be done in the
Makefile (or Makefile.private) or set on the command line:

```
make <PLAFTORM_NAME> LIBOTA_CONFIG=1
```

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
