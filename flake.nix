{
  description = "Tiny macOS CLI to get, set, and list keyboard input sources";

  inputs.nixpkgs.url = "github:nixos/nixpkgs?ref=nixpkgs-25.11-darwin";

  outputs =
    { self, nixpkgs }:
    let
      mk_macos-ime =
        pkgs:
        let
          stdenv = pkgs.stdenv;
          lib = pkgs.lib;
        in
        stdenv.mkDerivation (finalAttrs: {
          name = "macos-ime";
          src = pkgs.lib.cleanSource ./.;
          nativeBuildInputs = [ pkgs.installShellFiles ];
          makeFlags = [ "CC=${pkgs.stdenv.cc.targetPrefix}cc" ];
          installPhase = ''
            runHook preInstall
            mkdir -p $out/bin
            cp build/ime $out/bin/
            installShellCompletion --cmd ime \
              --bash completions/ime.bash \
              --zsh  completions/_ime \
              --fish completions/ime.fish
            runHook postInstall
          '';
          doCheck = true;
          meta = {
            license = lib.licenses.mit;
            mainProgram = "ime";
          };
        });
    in
    {
      packages.x86_64-darwin.default = mk_macos-ime nixpkgs.legacyPackages.x86_64-darwin;
      packages.aarch64-darwin.default = mk_macos-ime nixpkgs.legacyPackages.aarch64-darwin;
    };
}
