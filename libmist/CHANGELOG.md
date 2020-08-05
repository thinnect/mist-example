# Changelog

Changelog for the libmistmiddleware Mist library.

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

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
