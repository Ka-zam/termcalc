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
c 'c0/2.45e9'        # 0.122364... m, free-space wavelength at 2.45 GHz
c 'sqrt(mu0/eps0)'   # 376.730... ohm, free-space impedance
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
| Math | `sin` `cos` `tan` `asin` `acos` `atan` `sinh` `cosh` `tanh` `asinh` `acosh` `atanh` `exp` `log` `log10` `log2` `ln` `sqrt` `cbrt` `abs` `floor` `ceil` `round` |
| Math (2-arg) | `pow(x,y)` `atan2(y,x)` `max(a,b)` `min(a,b)` `mod(a,b)` |
| Bitwise | `popcount` `clz` `ctz` `bnot` `not8` `not16` `not32` |
| Bitwise (2-arg) | `bxor(a,b)` `band(a,b)` `bor(a,b)` `shl(x,n)` `shr(x,n)` |
| Format | `hex()` `bin()` `oct()` `dec()` |
| Bytes | `toKiB` `toMiB` `toGiB` `toTiB` `toKB` `toMB` `toGB` `toTB` |

### Constants

Physical constants use SI units and the [CODATA 2022 recommended
values](https://physics.nist.gov/cuu/Constants/). Exact values and constants derived
only from exact values are retained to `double` precision.

| Area | Constants |
|------|-----------|
| Math/state | `pi` (pi), `e` (Euler's number), `ans` (last result) |
| Vacuum / RF | `c0` (m/s), `mu0` (H/m), `eps0` (F/m), `eta0` or `Z0` (Ω), `Y0` (S), `ke` (N·m²/C²) |
| Electronics / thermal | `qe` (C), `h` (J·s), `hbar` (J·s), `kB` or `kboltz` (J/K), `eV` (J), `me` (kg), `alpha` (dimensionless) |
| Electrical metrology | `phi0` (Wb), `G0` (S), `RK` (Ω), `KJ` (Hz/V) |
| Binary bytes | `KiB` `MiB` `GiB` `TiB` (1024-based) |
| Decimal bytes | `KB` `MB` `GB` `TB` (1000-based) |

Names are case-sensitive where needed: `e` is Euler's number, `qe` is the
elementary charge, `kB` is the Boltzmann constant, and `kb` is a decimal
kilobyte.

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
c 'c0/5.8e9'             # 0.0516883548276 (free-space wavelength, m)
c 'kB*290'               # 4.0038821e-21 (thermal noise density, W/Hz)
c 'h*10e9/eV'            # 4.13566769692e-05 (10 GHz photon energy, eV)
c '1/(2*pi*50*1e-12)'    # 3183098861.84 (50-ohm RC corner, Hz)
```
