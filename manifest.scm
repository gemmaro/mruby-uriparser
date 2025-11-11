(use-modules (guix profiles)
             (gnu packages commencement)
             (gnu packages web)
             (gnu packages build-tools)
             (gnu packages documentation))

(packages->manifest (list gcc-toolchain
                          uriparser
                          doxygen
                          bear))
