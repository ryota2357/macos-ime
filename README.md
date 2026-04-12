# macos-ime

Tiny macOS CLI to get, set, and list keyboard input sources (IMEs).

## Installation

### Build from source

Build the project using Make:

```bash
make
```

The binary will be created at `build/ime`. You can move it to a directory in your `PATH` (e.g., `/usr/local/bin/`) to use it from anywhere.

### Using Nix

Install with Nix:

```bash
nix profile add github:ryota2357/macos-ime
```

Or run directly without installing:

```bash
nix run github:ryota2357/macos-ime -- <command> [args]
```

## Usage

```
ime <command> [args]
```

### Commands

- `get` - Print the current keyboard input source ID
- `set <ID>` - Switch to the specified input source ID
- `list` - List enabled keyboard input source IDs
- `help` - Print the help message

### Examples

Print the current input source:

```console
$ ime get
com.apple.inputmethod.Kotoeri.RomajiTyping.Japanese
```

List all enabled keyboard input sources:

```console
$ ime list
com.apple.keylayout.ABC
com.apple.inputmethod.Kotoeri.RomajiTyping.Japanese
```

Switch to a specific input source:

```console
$ ime set com.apple.keylayout.ABC
```

Typical workflow — save the current input source, switch, then restore:

```console
$ saved=$(ime get)
$ ime set com.apple.keylayout.ABC
$ # ... do something ...
$ ime set "$saved"
```

## Requirements

- macOS
- Clang compiler (if building from source)

Tested on macOS 15 and 26.

## License

[MIT](./LICENSE)
