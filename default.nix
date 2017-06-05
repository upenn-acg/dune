with import <nixpkgs> {};

# Build Dune and libdune.
# Dune
stdenv.mkDerivation {
  name = "dune-1.0";

  meta = {
    description = "Dune Kernel Module for Lightweight process virtualization.";
  };

  builder = ./builder.sh;
  buildInputs = [ sudo];
  src = ./.;

  # Create folders for installation and move file to appropriate folders thereafter.
  installPhase =
  ''
    mkdir -p $out/mod/
    mkdir -p $out/lib/
    mkdir -p $out/include/

    cp kern/dune.ko $out/mod/
  '';

}
