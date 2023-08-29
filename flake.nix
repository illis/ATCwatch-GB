{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/release-23.05";
    flake-utils.url = "github:numtide/flake-utils";
    zig.url = "github:mitchellh/zig-overlay";
    rust-overlay.url = "github:oxalica/rust-overlay";

    flake-compat = {
      url = "github:edolstra/flake-compat";
      flake = false;
    };

  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    ...
  } @ inputs: let
    overlays = [
      # Other overlays
      (final: prev: { 
        zigpkgs = inputs.zig.packages.${prev.system};
      })
      (import inputs.rust-overlay)
    ];

    # Our supported systems are the same supported systems as the Zig binaries
    systems = builtins.attrNames inputs.zig.packages;
  in
    flake-utils.lib.eachSystem systems (
      system: let
        pkgs = import nixpkgs {inherit overlays system;};
        rust = pkgs.rust-bin.stable.latest.minimal.override {
          extensions = [ "rust-src" ];
          targets = [ "thumbv7em-none-eabihf" ];
        }; 

      in {
        devShells.default = pkgs.mkShell {
          ARDUINO_DIRECTORIES_DATA= "./.arduino/data";
          ARDUINO_DIRECTORIES_DOWNLOADS= "./.arduino/data/staging";
          ARDUINO_DIRECTORIES_USER= "./ATCwatch/";
          ARDUINO_FQBN = "nRF52D6Fitness:nRF5:dsd6Watch:softdevice=onlySoftDevice";

          nativeBuildInputs = with pkgs; [
            zigpkgs.master
            rust
            gcc-arm-embedded
            bear # for generating commpile_commands for clangd completion
            python311Packages.adafruit-nrfutil
            python311Packages.requests
            python311Packages.docker
            #arduino
            arduino-cli
            python311Packages.invoke # for tasks.py
          ];

          shellHook = ''
          '';
        };
      }
    );
}
