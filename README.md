# termcalc

Fast terminal calculator. C23, ~35KB binary.

## Build

```bash
# Install dependencies (Debian/Ubuntu)
sudo apt install libreadline-dev

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Install to ~/.local/bin
cmake --install build
```

## Usage

```bash
c 34e6*1e-9          # 0.034
c 2^10               # 1024
c 'sin(pi/2)'        # 1
c 'hex(255)'         # 0xFF
c '0xFF & 0b1111'    # 15
c '4*GiB'            # 4294967296
c 4 * GiB            # 4294967296
c                    # interactive mode
```

## Features

### Operators
| Type | Operators |
|------|-----------|
| Arithmetic | `+` `-` `*` `/` `%` `^` `**` |
| Bitwise | `&` `\|` `~` `<<` `>>` |

### Number Formats
| Format | Example |
|--------|---------|
| Decimal | `42`, `3.14`, `1e-9` |
| Hex | `0xFF`, `0x1A2B` |
| Binary | `0b1010` |
| Octal | `0o755` |

### Functions
| Category | Functions |
|----------|-----------|
| Math | `sin` `cos` `tan` `asin` `acos` `atan` `sinh` `cosh` `tanh` `exp` `log` `log10` `log2` `ln` `sqrt` `cbrt` `abs` `floor` `ceil` `round` |
| Math (2-arg) | `pow(x,y)` `atan2(y,x)` `max(a,b)` `min(a,b)` `mod(a,b)` |
| Bitwise | `popcount` `clz` `ctz` `bnot` `not8` `not16` `not32` |
| Bitwise (2-arg) | `bxor(a,b)` `band(a,b)` `bor(a,b)` `shl(x,n)` `shr(x,n)` |
| Format | `hex()` `bin()` `oct()` `dec()` |
| Bytes | `toKiB` `toMiB` `toGiB` `toTiB` `toKB` `toMB` `toGB` `toTB` |

### Constants
| Constant | Value |
|----------|-------|
| `pi` | 3.14159... |
| `e` | 2.71828... |
| `ans` | last result |
| `KiB` `MiB` `GiB` `TiB` | 1024-based |
| `KB` `MB` `GB` `TB` | 1000-based |

### Interactive Mode
- **Up/Down** - history navigation (prefix search if text entered)
- **Ctrl+R** - reverse history search
- **Ctrl+A/E** - start/end of line
- History saved to `~/.c_history`

## Examples

```bash
c '1 << 10'              # 1024
c 'hex(255)'             # 0xFF
c 'bin(0xFF)'            # 0b11111111
c 'bxor(0xF0, 0xFF)'     # 15
c 'not8(0xF0)'           # 15
c '4*GiB'                # 4294967296
c 'toMiB(4*GiB)'         # 4096
c 'popcount(0xFF)'       # 8
```
