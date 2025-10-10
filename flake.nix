{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, ... }@inputs:
    inputs.flake-utils.lib.eachDefaultSystem
      (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
          qoi = pkgs.pkgsMusl.stdenv.mkDerivation {
            pname = "qoi";
            version = "git";
            src = ./.;
            hardeningDisable = ["all"];
            installPhase = ''
              make prefix=$out install
            '';
          };
        in {
          packages = {
            inherit qoi;
            default = qoi;
          };
          devShells.default = pkgs.mkShell {
            hardeningDisable = ["all"];
            buildInputs = [
              # pkgs.musl
              pkgs.valgrind
              pkgs.raylib
            ];
          };
        }
      );
}
