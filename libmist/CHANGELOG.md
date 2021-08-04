# Changelog

Changelog for the libmistmiddleware Mist library.

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [UNRELEASED]

## [0.5.2] - 2021-07-04

### Fixed
- Possible memory leak with the outgoing packet buffer for Mist data messages.

## [0.5.1] - 2021-06-22

### Changed
- Increase outgoing data size to 200 (2 fragments), CRC appended when frags > 1.

### Fixed
- Same sensor data getting passed to control modules multiple times (dynamic rules).

## [0.5.0] - 2021-04-27

### Added
- Support for additional data types in rules.
- Support for actively requesting missing rule fragments.

### Changed
- Rule size now up to 400 bytes.
- Max 9 rules by default.
- Multi-fragment transfers must have an XMODEM CRC at the end.

### Fixed
- Data stream IDs mixed up.
- subs_identify command now works.

## [0.4.8] - 2020-12-17

### Changed
- Mist rule maintenance period increased from 5 minutes to 15 minutes.

### Added
- Function for configuring spontaneous event backoff.

### Fixed
- Started event not sent immediately when rule started.

## [0.4.7] - 2020-12-11

### Changed
- Backoff management for spontaneous events (0, 10, 30, 70, 120).

## [0.4.6] - 2020-12-10

### Added
- Some new return values (MIST_ETIMEOUT, MIST_ESIZE, etc).

### Changed
- Spontaneous event calls now use less stack space.
- Data only sent to broadcast if multiple destinations or resource.
- Backoff management for spontaneous events (0, 10, 30, 70, 150, 300).

## [0.4.5] - 2020-11-21

### Fixed
- Some complex input rules being executed too often.

## [0.4.4] - 2020-11-20

### Added
- Support for complex control values - closed-loop illuminance, blink.

## [0.4.3] - 2020-10-26

### Added
- Support for settings block, passed with input.
- Metadata request handling.
- Output constraint checking.

## [0.4.2] - 2020-10-13

### Fixed
- Data timestamp parsing when data under top-level dt_data element.

### Added
- Support for illuminance rules.

## [0.4.1] - 2020-10-11

### Added
- Priority based rule sorting.
- Support for freeze.

### Added
- Support for illuminance rules.

## [0.4.0] - 2020-10-08

### Changed
- Library now uses refactored mist rule interpreter.

### Added
- Support for RTC rules.
- Support for user-button rules.

## [0.3.5] - 2020-09-16

### Added
- Support for up to 3 movement active levels with different timeouts.

## [0.3.4] - 2020-08-28

### Fixed
- Handle a rare nested movement rule case.
- Movement count 0 cancels movement action.

## [0.3.3] - 2020-08-05

### Fixed
- Lighting rule result data match did not work for more complex data.

## [0.3.2] - 2020-08-05

### Fixed
- Data with MIST_ITEM_INT32 sent incorrectly.

## [0.3.1] - 2020-07-24

### Added
- Complex data support for spontaneous events.

### Changed
- Semantic constraints are ignored for now.

## [0.3.0] - 2020-07-23

### Added
- Initial support for rule persisence in filesystem.
- Rule priority handling.
- Support for status rules.
- Limited support for central lux sensor rules.

### Fixed
- Data is no longer sent when rule specifies data should not be sent.

## [0.2.3] - 2020-07-16

### Added

- TX Data queueing.

### Fixed

- Incorrect interpretation of simple dim rules.
- Race causing RX packet corruption.

## [0.2.2] - 2020-06-30

### Fixed

- Incorrect interpretation of combined movement-idle rule syntax.

## [0.2.1] - 2020-05-31

### Added

- Library verison query function.

### Fixed

- Possible overflow bug in rule receive.

## [0.2.0] - 2020-05-30

### Added

- Limited support for lighting rules.
- Support for resource-type movement sensors.

## [0.1.0] - 2020-04-29

### Added

- Basic mist subscription / rule communications handling.
