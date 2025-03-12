{
  description = "My TextEditor project";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = import nixpkgs { inherit system; };
  in
  {
    devShells.${system}.default = pkgs.mkShell {
      buildInputs = [
        pkgs.cmake
        pkgs.ncurses
        pkgs.gcc
      ];

      shellHook = ''
          export LD_LIBRARY_PATH='/nix/store/8s7f5jgn8bbvj552hb5glsgwfx881kfy-ncurses-6.4.20221231/lib'
        '';
    };
  };
}
