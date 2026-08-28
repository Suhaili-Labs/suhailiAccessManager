{
  description = "NDI Access Manager on Linux - TUI for managing the NDI configuration file";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "accessman";
          version = "1.2.1";

          src = ./.;

          buildInputs = [ pkgs.ftxui ];

          # nlohmann/json is vendored as json.hpp in the repo root.
          # FTXUI comes from nixpkgs instead of the makefile's git-clone
          # cache, since network access is not available in the Nix sandbox.
          buildPhase = ''
            runHook preBuild
            $CXX -O2 -std=c++17 -c tui.cpp -o tui.o
            $CXX -o accessman tui.o \
              -lftxui-component -lftxui-dom -lftxui-screen \
              -lpthread -ldl
            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall
            install -Dm755 accessman $out/bin/accessman
            runHook postInstall
          '';

          meta = with pkgs.lib; {
            description = "Terminal UI tool for managing the NDI configuration file on Linux";
            homepage = "https://github.com/Suhaili-Labs/suhailiAccessManager";
            license = licenses.mit;
            platforms = platforms.linux;
            mainProgram = "accessman";
          };
        };

        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/accessman";
        };

        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            gcc
            gnumake
            cmake
            git
            ftxui
          ];
        };
      });
}
