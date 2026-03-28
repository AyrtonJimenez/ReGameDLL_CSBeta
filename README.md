# ReGameDLL_CSBeta

Reverse-engineered GameDLL restoration for Counter-Strike Beta 6.1.

## What This Project Is

`ReGameDLL_CSBeta` is a reverse-engineering and restoration project focused on the Linux GameDLL from Counter-Strike Beta 6.1.

The project follows the same broad technical idea that made `ReGameDLL_CS` possible, but its target is specific to Counter-Strike Beta 6.1. This repository is a reconstruction of the server-side GameDLL from the Linux `cs_i386.so` binary, using the DWARF debug information embedded in that build.

The intention is not merely to recover source code artefacts, but to turn the recovered code into a practical and maintainable codebase that can be studied, repaired and extended with care.

## Project Goals

- Provide a repaired and maintainable version of the Counter-Strike Beta 6.1 GameDLL.
- Preserve original historical behaviour wherever it is practical to do so.
- Establish a sound foundation for possible future modding support and plugin-facing APIs.
- Document and retain an important stage in the technical history of Counter-Strike.

## Current Status

This repository is presently in a restoration and repair phase.

- Development, testing and compilation have only been validated on Windows.
- The current working build environment is Visual Studio 2019 with the `v142` toolset.
- Linux-related codepaths and general Linux support are not yet complete.
- No stable modding API or plugin API is currently documented or guaranteed.

Accordingly, this project should not yet be presented as a finished cross-platform replacement for the original game library.

## Build Instructions

At present, the only build path that has been tested in practice is the Windows Visual Studio workflow.

1. Open `mp.sln` in Visual Studio 2019.
2. Ensure the project is built with the `v142` toolset for `Win32`.
3. Build the `mp` project in either `Release` or `Debug`, as appropriate.

The solution currently produces `mp.dll`.

No Linux build process is documented here because Linux support in this repository has not yet been completed.

## Licence

This project is distributed under the terms of the GNU General Public License, version 3. See [LICENSE](./LICENSE) for the full licence text.

## Acknowledgements

- https://github.com/nagist/cs16nd
- https://github.com/rehlds/ReGameDLL_CS

This project owes a particular debt to [@nagist](https://github.com/nagist), whose open-source publication of `cs16nd` was of exceptional importance.

That work materially accelerated the completion of `ReGameDLL_CS`, and it also provided major assistance and reference value for this Counter-Strike Beta 6.1 restoration effort. It should be stated plainly that this contribution was not properly acknowledged by `ReGameDLL_CS`, despite its significance.

With that said, this repository expresses sincere respect to both `cs16nd` and `ReGameDLL_CS`. Each project, in its own way, has contributed meaningfully to the preservation, understanding and reconstruction of classic Counter-Strike code.

## Contributing and Project Direction

Contributions are welcome where they improve correctness, maintainability and historical fidelity.

The immediate priority is to continue repairing and validating the restored codebase. Longer term, the project may grow carefully considered support for mods and plugin-facing interfaces, but such work should be treated as future development rather than as an established public surface today.
