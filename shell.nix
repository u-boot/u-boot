# NixOS shell configuration to bootstrap the required dependencies

with import <nixpkgs> {
  crossSystem = {
    config = "aarch64-unknown-linux-gnu";
  };
};

stdenv.mkDerivation {
	# Build deps
  nativeBuildInputs = [
		buildPackages.bison
		buildPackages.flex
		gcc
	];
}
