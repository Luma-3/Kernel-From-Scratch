#!/bin/bash
# =============================================================================
# build_toolchain.sh - Build a STATIC i386-elf cross-compilation toolchain
# =============================================================================
# This script builds binutils, gcc, and gdb as static binaries so the
# toolchain can be moved between machines without dependency issues.
#
# Usage: ./build_toolchain.sh [--prefix /path/to/install]
# Default prefix: ./cross
# =============================================================================

set -euo pipefail

# ── Configuration ────────────────────────────────────────────────────────────
TARGET="i386-elf"
BINUTILS_VERSION="2.46.0"
GCC_VERSION="15.2.0"
GDB_VERSION="17.1"

GMP_VERSION="6.3.0"
MPFR_VERSION="4.2.1"
MPC_VERSION="1.3.1"

BISON_VERSION="3.8.2"
FLEX_VERSION="2.6.4"

XORRISO_VERSION="1.5.6"
GRUB_VERSION="2.12"

BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz"
GDB_URL="https://ftp.gnu.org/gnu/gdb/gdb-${GDB_VERSION}.tar.xz"
GMP_URL="https://ftp.gnu.org/gnu/gmp/gmp-${GMP_VERSION}.tar.xz"
MPFR_URL="https://ftp.gnu.org/gnu/mpfr/mpfr-${MPFR_VERSION}.tar.xz"
MPC_URL="https://ftp.gnu.org/gnu/mpc/mpc-${MPC_VERSION}.tar.gz"
BISON_URL="https://ftp.gnu.org/gnu/bison/bison-${BISON_VERSION}.tar.xz"
FLEX_URL="https://github.com/westes/flex/releases/download/v${FLEX_VERSION}/flex-${FLEX_VERSION}.tar.gz"
XORRISO_URL="https://ftp.gnu.org/gnu/xorriso/xorriso-${XORRISO_VERSION}.tar.gz"
GRUB_URL="https://ftp.gnu.org/gnu/grub/grub-${GRUB_VERSION}.tar.xz"

# ── Mirrors GNU français (fallback si rate-limit) ────────────────────────────
GNU_MIRROR_LIP6="https://ftp.lip6.fr/pub/gnu"
GNU_MIRROR_IBCP="https://mirror.ibcp.fr/pub/gnu"
GNU_MIRROR_UNIV_REIMS="https://ftp.univ-reims.fr/mirror/ftp.gnu.org/pub/gnu"

# URLs miroir par paquet GNU
GMP_MIRRORS=(
    "${GNU_MIRROR_LIP6}/gmp/gmp-${GMP_VERSION}.tar.xz"
    "${GNU_MIRROR_IBCP}/gmp/gmp-${GMP_VERSION}.tar.xz"
    "${GNU_MIRROR_UNIV_REIMS}/gmp/gmp-${GMP_VERSION}.tar.xz"
)
MPFR_MIRRORS=(
    "${GNU_MIRROR_LIP6}/mpfr/mpfr-${MPFR_VERSION}.tar.xz"
    "${GNU_MIRROR_IBCP}/mpfr/mpfr-${MPFR_VERSION}.tar.xz"
    "${GNU_MIRROR_UNIV_REIMS}/mpfr/mpfr-${MPFR_VERSION}.tar.xz"
)
MPC_MIRRORS=(
    "${GNU_MIRROR_LIP6}/mpc/mpc-${MPC_VERSION}.tar.gz"
    "${GNU_MIRROR_IBCP}/mpc/mpc-${MPC_VERSION}.tar.gz"
    "${GNU_MIRROR_UNIV_REIMS}/mpc/mpc-${MPC_VERSION}.tar.gz"
)
BINUTILS_MIRRORS=(
    "${GNU_MIRROR_LIP6}/binutils/binutils-${BINUTILS_VERSION}.tar.gz"
    "${GNU_MIRROR_IBCP}/binutils/binutils-${BINUTILS_VERSION}.tar.gz"
    "${GNU_MIRROR_UNIV_REIMS}/binutils/binutils-${BINUTILS_VERSION}.tar.gz"
)
GCC_MIRRORS=(
    "${GNU_MIRROR_LIP6}/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz"
    "${GNU_MIRROR_IBCP}/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz"
    "${GNU_MIRROR_UNIV_REIMS}/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz"
)
GDB_MIRRORS=(
    "${GNU_MIRROR_LIP6}/gdb/gdb-${GDB_VERSION}.tar.xz"
    "${GNU_MIRROR_IBCP}/gdb/gdb-${GDB_VERSION}.tar.xz"
    "${GNU_MIRROR_UNIV_REIMS}/gdb/gdb-${GDB_VERSION}.tar.xz"
)
BISON_MIRRORS=(
    "${GNU_MIRROR_LIP6}/bison/bison-${BISON_VERSION}.tar.xz"
    "${GNU_MIRROR_IBCP}/bison/bison-${BISON_VERSION}.tar.xz"
    "${GNU_MIRROR_UNIV_REIMS}/bison/bison-${BISON_VERSION}.tar.xz"
)
# Flex n'est pas sur GNU FTP — fallback vers le tarball GitHub uniquement
FLEX_MIRRORS=()
XORRISO_MIRRORS=(
    "${GNU_MIRROR_LIP6}/xorriso/xorriso-${XORRISO_VERSION}.tar.gz"
    "${GNU_MIRROR_IBCP}/xorriso/xorriso-${XORRISO_VERSION}.tar.gz"
    "${GNU_MIRROR_UNIV_REIMS}/xorriso/xorriso-${XORRISO_VERSION}.tar.gz"
)
GRUB_MIRRORS=(
    "${GNU_MIRROR_LIP6}/grub/grub-${GRUB_VERSION}.tar.xz"
    "${GNU_MIRROR_IBCP}/grub/grub-${GRUB_VERSION}.tar.xz"
    "${GNU_MIRROR_UNIV_REIMS}/grub/grub-${GRUB_VERSION}.tar.xz"
)

JOBS="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

# ── Parse arguments ─────────────────────────────────────────────────────────
PREFIX=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix) PREFIX="$2"; shift 2 ;;
        --prefix=*) PREFIX="${1#*=}"; shift ;;
        -h|--help)
            echo "Usage: $0 [--prefix /path/to/install]"
            echo "  Default prefix: <script_dir>/cross"
            exit 0
            ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [[ -z "$PREFIX" ]]; then
    PREFIX="${SCRIPT_DIR}/cross"
fi

# Make PREFIX absolute
PREFIX="$(mkdir -p "$PREFIX" && cd "$PREFIX" && pwd)"

WORKDIR="${SCRIPT_DIR}/.toolchain_build"
SRCDIR="${WORKDIR}/src"
BUILDDIR="${WORKDIR}/build"
LOGDIR="${WORKDIR}/logs"

export PATH="${PREFIX}/bin:${PATH}"

# ── Colors ───────────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

info()    { echo -e "${CYAN}[INFO]${NC}  $*"; }
success() { echo -e "${GREEN}[OK]${NC}    $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $*"; }
error()   { echo -e "${RED}[ERR]${NC}   $*"; }
header()  { echo -e "\n${BOLD}════════════════════════════════════════════════════════════${NC}"; \
            echo -e "${BOLD}  $*${NC}"; \
            echo -e "${BOLD}════════════════════════════════════════════════════════════${NC}\n"; }

# ── Dependency check ─────────────────────────────────────────────────────────
check_deps() {
    header "Checking host dependencies"
    local missing=()
    # Only check for build tools — GMP, MPFR, MPC, bison, flex are built from source
    for cmd in gcc g++ make tar m4; do
        if ! command -v "$cmd" >/dev/null 2>&1; then
            missing+=("$cmd")
        fi
    done

    # wget or curl needed for downloads
    if ! command -v wget >/dev/null 2>&1 && ! command -v curl >/dev/null 2>&1; then
        missing+=("wget or curl")
    fi

    if [[ ${#missing[@]} -gt 0 ]]; then
        error "Missing dependencies: ${missing[*]}"
        echo ""
        echo "These are basic build tools that should be available on most systems."
        echo "Ask your sysadmin to install: ${missing[*]}"
        exit 1
    fi
    success "All build tools found"
    info "GMP, MPFR, MPC, bison, flex will be built from source (no sudo needed)"
}

# ── Download ─────────────────────────────────────────────────────────────────
# Usage: download <dest_dir> <primary_url> [mirror_url...]
# Essaie l'URL primaire, puis les miroirs en cas d'échec (rate-limit, timeout…)
download() {
    local dest="$1"; shift
    local urls=("$@")
    local filename
    filename="$(basename "${urls[0]}")"

    if [[ -f "${dest}/${filename}" ]]; then
        info "Already downloaded: ${filename}"
        return
    fi

    local success_flag=false
    for url in "${urls[@]}"; do
        info "Downloading ${filename} from ${url}..."
        if command -v wget >/dev/null 2>&1; then
            if wget -q --timeout=30 --tries=2 -P "$dest" "$url" 2>/dev/null; then
                success_flag=true; break
            fi
        elif command -v curl >/dev/null 2>&1; then
            if curl -L --max-time 60 --retry 2 --silent --show-error \
                    -o "${dest}/${filename}" "$url" 2>/dev/null; then
                success_flag=true; break
            fi
        fi
        warn "Échec depuis ${url}, passage au miroir suivant..."
    done

    if $success_flag; then
        success "Downloaded ${filename}"
    else
        error "Impossible de télécharger ${filename} depuis tous les miroirs"
        exit 1
    fi
}

extract() {
    local archive="$1"
    local dest="$2"
    local dirname="$3"

    if [[ -d "${dest}/${dirname}" ]]; then
        info "Already extracted: ${dirname}"
        return
    fi

    info "Extracting $(basename "$archive")..."
    case "$archive" in
        *.tar.gz)  tar -xzf "$archive" -C "$dest" ;;
        *.tar.xz)  tar -xJf "$archive" -C "$dest" ;;
        *.tar.bz2) tar -xjf "$archive" -C "$dest" ;;
    esac
    success "Extracted ${dirname}"
}

# ── Build functions ──────────────────────────────────────────────────────────

# Note: We do NOT use LDFLAGS="-static" because the host system lacks
# static versions of libm/libstdc++. Instead, portability is achieved by:
#   1. Building GMP/MPFR/MPC locally as static libs (--disable-shared)
#   2. Linking them into the toolchain via --with-gmp/mpfr/mpc
# The resulting binaries depend only on standard system libs (libc, libm, etc.)
# which are present on any Linux machine.

# Local prefix for GMP/MPFR/MPC (built from source, no sudo needed)
LOCAL_DEPS_PREFIX=""

build_gmp() {
    header "Building GMP ${GMP_VERSION} (local, static)"

    if [[ -f "${LOCAL_DEPS_PREFIX}/lib/libgmp.a" ]]; then
        success "GMP already built, skipping"
        return
    fi

    local builddir="${BUILDDIR}/gmp"
    rm -rf "$builddir"
    mkdir -p "$builddir"
    cd "$builddir"

    info "Configuring GMP..."
    "${SRCDIR}/gmp-${GMP_VERSION}/configure" \
        --prefix="${LOCAL_DEPS_PREFIX}" \
        --disable-shared \
        --enable-static \
        CC="gcc -std=gnu17" \
        > "${LOGDIR}/gmp-configure.log" 2>&1

    info "Compiling GMP (${JOBS} jobs)..."
    make -j"${JOBS}" > "${LOGDIR}/gmp-make.log" 2>&1

    info "Installing GMP locally..."
    make install > "${LOGDIR}/gmp-install.log" 2>&1

    success "GMP ${GMP_VERSION} installed to ${LOCAL_DEPS_PREFIX}"
}

build_mpfr() {
    header "Building MPFR ${MPFR_VERSION} (local, static)"

    if [[ -f "${LOCAL_DEPS_PREFIX}/lib/libmpfr.a" ]]; then
        success "MPFR already built, skipping"
        return
    fi

    local builddir="${BUILDDIR}/mpfr"
    rm -rf "$builddir"
    mkdir -p "$builddir"
    cd "$builddir"

    info "Configuring MPFR..."
    "${SRCDIR}/mpfr-${MPFR_VERSION}/configure" \
        --prefix="${LOCAL_DEPS_PREFIX}" \
        --with-gmp="${LOCAL_DEPS_PREFIX}" \
        --disable-shared \
        --enable-static \
        CC="gcc -std=gnu17" \
        > "${LOGDIR}/mpfr-configure.log" 2>&1

    info "Compiling MPFR (${JOBS} jobs)..."
    make -j"${JOBS}" > "${LOGDIR}/mpfr-make.log" 2>&1

    info "Installing MPFR locally..."
    make install > "${LOGDIR}/mpfr-install.log" 2>&1

    success "MPFR ${MPFR_VERSION} installed to ${LOCAL_DEPS_PREFIX}"
}

build_mpc() {
    header "Building MPC ${MPC_VERSION} (local, static)"

    if [[ -f "${LOCAL_DEPS_PREFIX}/lib/libmpc.a" ]]; then
        success "MPC already built, skipping"
        return
    fi

    local builddir="${BUILDDIR}/mpc"
    rm -rf "$builddir"
    mkdir -p "$builddir"
    cd "$builddir"

    info "Configuring MPC..."
    "${SRCDIR}/mpc-${MPC_VERSION}/configure" \
        --prefix="${LOCAL_DEPS_PREFIX}" \
        --with-gmp="${LOCAL_DEPS_PREFIX}" \
        --with-mpfr="${LOCAL_DEPS_PREFIX}" \
        --disable-shared \
        --enable-static \
        CC="gcc -std=gnu17" \
        > "${LOGDIR}/mpc-configure.log" 2>&1

    info "Compiling MPC (${JOBS} jobs)..."
    make -j"${JOBS}" > "${LOGDIR}/mpc-make.log" 2>&1

    info "Installing MPC locally..."
    make install > "${LOGDIR}/mpc-install.log" 2>&1

    success "MPC ${MPC_VERSION} installed to ${LOCAL_DEPS_PREFIX}"
}

build_bison() {
    header "Building Bison ${BISON_VERSION}"

    if [[ -f "${PREFIX}/bin/bison" ]]; then
        success "Bison already installed, skipping"
        return
    fi

    local builddir="${BUILDDIR}/bison"
    rm -rf "$builddir"
    mkdir -p "$builddir"
    cd "$builddir"

    info "Configuring Bison..."
    "${SRCDIR}/bison-${BISON_VERSION}/configure" \
        --prefix="${PREFIX}" \
        --disable-nls \
        > "${LOGDIR}/bison-configure.log" 2>&1

    info "Compiling Bison (${JOBS} jobs)..."
    make -j"${JOBS}" > "${LOGDIR}/bison-make.log" 2>&1

    info "Installing Bison..."
    make install > "${LOGDIR}/bison-install.log" 2>&1

    success "Bison ${BISON_VERSION} installed"
}

build_flex() {
    header "Building Flex ${FLEX_VERSION}"

    if [[ -f "${PREFIX}/bin/flex" ]]; then
        success "Flex already installed, skipping"
        return
    fi

    local builddir="${BUILDDIR}/flex"
    rm -rf "$builddir"
    mkdir -p "$builddir"
    cd "$builddir"

    info "Configuring Flex..."
    "${SRCDIR}/flex-${FLEX_VERSION}/configure" \
        --prefix="${PREFIX}" \
        --disable-nls \
        > "${LOGDIR}/flex-configure.log" 2>&1

    info "Compiling Flex (${JOBS} jobs)..."
    make -j"${JOBS}" > "${LOGDIR}/flex-make.log" 2>&1

    info "Installing Flex..."
    make install > "${LOGDIR}/flex-install.log" 2>&1

    success "Flex ${FLEX_VERSION} installed"
}

build_binutils() {
    header "Building binutils ${BINUTILS_VERSION} (static)"

    local builddir="${BUILDDIR}/binutils"
    if [[ -f "${PREFIX}/bin/${TARGET}-as" ]]; then
        success "binutils already installed, skipping"
        return
    fi

    rm -rf "$builddir"
    mkdir -p "$builddir"
    cd "$builddir"

    info "Configuring binutils..."
    "${SRCDIR}/binutils-${BINUTILS_VERSION}/configure" \
        --target="${TARGET}" \
        --prefix="${PREFIX}" \
        --with-sysroot \
        --disable-nls \
        --disable-werror \
        --disable-shared \
        --enable-static \
        --with-gmp="${LOCAL_DEPS_PREFIX}" \
        --with-mpfr="${LOCAL_DEPS_PREFIX}" \
        --with-mpc="${LOCAL_DEPS_PREFIX}" \
        > "${LOGDIR}/binutils-configure.log" 2>&1

    info "Compiling binutils (${JOBS} jobs)..."
    make -j"${JOBS}" > "${LOGDIR}/binutils-make.log" 2>&1

    info "Installing binutils..."
    make install > "${LOGDIR}/binutils-install.log" 2>&1

    success "binutils ${BINUTILS_VERSION} installed"
}

build_gcc() {
    header "Building GCC ${GCC_VERSION} (static, freestanding cross-compiler)"

    local builddir="${BUILDDIR}/gcc"
    if [[ -f "${PREFIX}/bin/${TARGET}-gcc" ]]; then
        success "GCC already installed, skipping"
        return
    fi

    rm -rf "$builddir"
    mkdir -p "$builddir"
    cd "$builddir"

    info "Configuring GCC..."
    "${SRCDIR}/gcc-${GCC_VERSION}/configure" \
        --target="${TARGET}" \
        --prefix="${PREFIX}" \
        --disable-nls \
        --enable-languages=c \
        --without-headers \
        --disable-shared \
        --enable-static \
        --disable-libssp \
        --disable-libquadmath \
        --disable-libgomp \
        --disable-libatomic \
        --disable-threads \
        --disable-multilib \
        --disable-decimal-float \
        --disable-libffi \
        --disable-libstdcxx \
        --with-gmp="${LOCAL_DEPS_PREFIX}" \
        --with-mpfr="${LOCAL_DEPS_PREFIX}" \
        --with-mpc="${LOCAL_DEPS_PREFIX}" \
        > "${LOGDIR}/gcc-configure.log" 2>&1

    info "Compiling GCC (${JOBS} jobs)... this will take a while ☕"
    make -j"${JOBS}" all-gcc > "${LOGDIR}/gcc-make.log" 2>&1
    make -j"${JOBS}" all-target-libgcc >> "${LOGDIR}/gcc-make.log" 2>&1

    info "Installing GCC..."
    make install-gcc > "${LOGDIR}/gcc-install.log" 2>&1
    make install-target-libgcc >> "${LOGDIR}/gcc-install.log" 2>&1

    success "GCC ${GCC_VERSION} installed"
}

build_gdb() {
    header "Building GDB ${GDB_VERSION} (static)"

    local builddir="${BUILDDIR}/gdb"
    if [[ -f "${PREFIX}/bin/${TARGET}-gdb" ]]; then
        success "GDB already installed, skipping"
        return
    fi

    rm -rf "$builddir"
    mkdir -p "$builddir"
    cd "$builddir"

    info "Configuring GDB..."
    "${SRCDIR}/gdb-${GDB_VERSION}/configure" \
        --target="${TARGET}" \
        --prefix="${PREFIX}" \
        --disable-nls \
        --disable-werror \
        --disable-shared \
        --enable-static \
        --with-system-readline=no \
        --with-guile=no \
        --with-python=no \
        --disable-source-highlight \
        --with-gmp="${LOCAL_DEPS_PREFIX}" \
        --with-mpfr="${LOCAL_DEPS_PREFIX}" \
        --with-mpc="${LOCAL_DEPS_PREFIX}" \
        > "${LOGDIR}/gdb-configure.log" 2>&1

    info "Compiling GDB (${JOBS} jobs)..."
    make -j"${JOBS}" > "${LOGDIR}/gdb-make.log" 2>&1

    info "Installing GDB..."
    make install > "${LOGDIR}/gdb-install.log" 2>&1

    success "GDB ${GDB_VERSION} installed"
}

build_xorriso() {
    header "Building xorriso ${XORRISO_VERSION} (local)"

    if [[ -f "${PREFIX}/bin/xorriso" ]]; then
        success "xorriso already installed, skipping"
        return
    fi

    local builddir="${BUILDDIR}/xorriso"
    rm -rf "$builddir"
    mkdir -p "$builddir"
    cd "$builddir"

    info "Configuring xorriso..."
    "${SRCDIR}/xorriso-${XORRISO_VERSION}/configure" \
        --prefix="${PREFIX}" \
        --disable-shared \
        --enable-static \
        CC="gcc -std=gnu17" \
        > "${LOGDIR}/xorriso-configure.log" 2>&1

    info "Compiling xorriso (${JOBS} jobs)..."
    make -j"${JOBS}" > "${LOGDIR}/xorriso-make.log" 2>&1

    info "Installing xorriso..."
    make install > "${LOGDIR}/xorriso-install.log" 2>&1

    success "xorriso ${XORRISO_VERSION} installed"
}

build_grub() {
    header "Building GRUB ${GRUB_VERSION} (i386-pc)"

    if [[ -f "${PREFIX}/bin/grub-mkrescue" ]]; then
        success "GRUB already installed, skipping"
        return
    fi

    local builddir="${BUILDDIR}/grub"
    rm -rf "$builddir"
    mkdir -p "$builddir"
    cd "$builddir"

    # GRUB needs to find our local objcopy for i386-elf target
    # and xorriso must be in PATH
    export PATH="${PREFIX}/bin:${PATH}"

    # Fix: extra_deps.lst is missing from the tarball release (normally
    # generated by ./bootstrap in the git repo). Create it if missing.
    if [[ ! -f "${SRCDIR}/grub-${GRUB_VERSION}/grub-core/extra_deps.lst" ]]; then
        info "Creating missing extra_deps.lst..."
        touch "${SRCDIR}/grub-${GRUB_VERSION}/grub-core/extra_deps.lst"
    fi

    info "Configuring GRUB..."
    "${SRCDIR}/grub-${GRUB_VERSION}/configure" \
        --prefix="${PREFIX}" \
        --target="${TARGET}" \
        --disable-werror \
        --disable-nls \
        --disable-grub-mkfont \
        --disable-grub-mount \
        --with-platform=pc \
        TARGET_CC="${PREFIX}/bin/${TARGET}-gcc" \
        TARGET_OBJCOPY="${PREFIX}/bin/${TARGET}-objcopy" \
        TARGET_STRIP="${PREFIX}/bin/${TARGET}-strip" \
        TARGET_NM="${PREFIX}/bin/${TARGET}-nm" \
        TARGET_RANLIB="${PREFIX}/bin/${TARGET}-ranlib" \
        CC="gcc -std=gnu17" \
        > "${LOGDIR}/grub-configure.log" 2>&1

    info "Compiling GRUB (${JOBS} jobs)..."
    make -j"${JOBS}" > "${LOGDIR}/grub-make.log" 2>&1

    info "Installing GRUB..."
    make install > "${LOGDIR}/grub-install.log" 2>&1

    success "GRUB ${GRUB_VERSION} installed"
}

# ── Verification ─────────────────────────────────────────────────────────────
verify_toolchain() {
    header "Verifying toolchain"

    local all_ok=true
    for tool in as ld objdump gcc gdb; do
        local bin="${PREFIX}/bin/${TARGET}-${tool}"
        if [[ -x "$bin" ]]; then
            local info_str
            info_str="$("$bin" --version 2>&1 | head -1)"
            success "${TARGET}-${tool}: ${info_str}"
        else
            error "${TARGET}-${tool}: NOT FOUND"
            all_ok=false
        fi
    done

    # Check standalone tools (not prefixed with target)
    for tool in bison flex grub-mkrescue xorriso; do
        local bin="${PREFIX}/bin/${tool}"
        if [[ -x "$bin" ]]; then
            local info_str
            info_str="$("$bin" --version 2>&1 | head -1)"
            success "${tool}: ${info_str}"
        else
            error "${tool}: NOT FOUND"
            all_ok=false
        fi
    done

    echo ""
    if $all_ok; then
        success "Toolchain is ready!"
    else
        error "Some tools are missing, check the logs in ${LOGDIR}/"
        exit 1
    fi
}

# ── Summary ──────────────────────────────────────────────────────────────────
print_summary() {
    header "Build complete! 🎉"
    echo -e "  ${BOLD}Toolchain location:${NC}  ${PREFIX}"
    echo -e "  ${BOLD}Target:${NC}              ${TARGET}"
    echo ""
    echo -e "  ${BOLD}Binaries:${NC}"
    echo "    ${PREFIX}/bin/${TARGET}-gcc"
    echo "    ${PREFIX}/bin/${TARGET}-as"
    echo "    ${PREFIX}/bin/${TARGET}-ld"
    echo "    ${PREFIX}/bin/${TARGET}-objdump"
    echo "    ${PREFIX}/bin/${TARGET}-gdb"
    echo "    ${PREFIX}/bin/bison"
    echo "    ${PREFIX}/bin/flex"
    echo "    ${PREFIX}/bin/grub-mkrescue"
    echo "    ${PREFIX}/bin/xorriso"
    echo ""
    echo -e "  ${BOLD}Add to your PATH:${NC}"
    echo "    export PATH=\"${PREFIX}/bin:\$PATH\""
    echo ""
    echo -e "  ${BOLD}To move to another machine:${NC}"
    echo "    tar -czf toolchain-${TARGET}.tar.gz -C \"$(dirname "${PREFIX}")\" \"$(basename "${PREFIX}")\""
    echo "    # Copy the archive to the other machine and extract it"
    echo "    # Then update PATH to point to the new location"
    echo ""
    echo -e "  ${BOLD}Build logs:${NC} ${LOGDIR}/"
    echo ""
}

# ── Main ─────────────────────────────────────────────────────────────────────
main() {
    header "Static i386-elf Toolchain Builder"
    echo "  Target:     ${TARGET}"
    echo "  Prefix:     ${PREFIX}"
    echo "  Build dir:  ${WORKDIR}"
    echo "  Jobs:       ${JOBS}"
    echo ""

    mkdir -p "${SRCDIR}" "${BUILDDIR}" "${LOGDIR}" "${PREFIX}"

    # Local prefix for GMP/MPFR/MPC (no sudo needed)
    LOCAL_DEPS_PREFIX="${WORKDIR}/local_deps"
    mkdir -p "${LOCAL_DEPS_PREFIX}"

    check_deps

    # ── Download sources ──
    header "Downloading sources"
    download "${SRCDIR}"  "${GMP_URL}"      "${GMP_MIRRORS[@]}"
    download "${SRCDIR}"  "${MPFR_URL}"     "${MPFR_MIRRORS[@]}"
    download "${SRCDIR}"  "${MPC_URL}"      "${MPC_MIRRORS[@]}"
    download "${SRCDIR}"  "${BISON_URL}"    "${BISON_MIRRORS[@]}"
    download "${SRCDIR}"  "${FLEX_URL}"     "${FLEX_MIRRORS[@]}"
    download "${SRCDIR}"  "${BINUTILS_URL}" "${BINUTILS_MIRRORS[@]}"
    download "${SRCDIR}"  "${GCC_URL}"      "${GCC_MIRRORS[@]}"
    download "${SRCDIR}"  "${GDB_URL}"      "${GDB_MIRRORS[@]}"
    download "${SRCDIR}"  "${XORRISO_URL}"  "${XORRISO_MIRRORS[@]}"
    download "${SRCDIR}"  "${GRUB_URL}"     "${GRUB_MIRRORS[@]}"

    # ── Extract sources ──
    header "Extracting sources"
    extract "${SRCDIR}/gmp-${GMP_VERSION}.tar.xz"           "${SRCDIR}" "gmp-${GMP_VERSION}"
    extract "${SRCDIR}/mpfr-${MPFR_VERSION}.tar.xz"         "${SRCDIR}" "mpfr-${MPFR_VERSION}"
    extract "${SRCDIR}/mpc-${MPC_VERSION}.tar.gz"            "${SRCDIR}" "mpc-${MPC_VERSION}"
    extract "${SRCDIR}/bison-${BISON_VERSION}.tar.xz"        "${SRCDIR}" "bison-${BISON_VERSION}"
    extract "${SRCDIR}/flex-${FLEX_VERSION}.tar.gz"          "${SRCDIR}" "flex-${FLEX_VERSION}"
    extract "${SRCDIR}/binutils-${BINUTILS_VERSION}.tar.gz"  "${SRCDIR}" "binutils-${BINUTILS_VERSION}"
    extract "${SRCDIR}/gcc-${GCC_VERSION}.tar.gz"            "${SRCDIR}" "gcc-${GCC_VERSION}"
    extract "${SRCDIR}/gdb-${GDB_VERSION}.tar.xz"            "${SRCDIR}" "gdb-${GDB_VERSION}"
    extract "${SRCDIR}/xorriso-${XORRISO_VERSION}.tar.gz"    "${SRCDIR}" "xorriso-${XORRISO_VERSION}"
    extract "${SRCDIR}/grub-${GRUB_VERSION}.tar.xz"          "${SRCDIR}" "grub-${GRUB_VERSION}"

    # ── Build local dependencies (no sudo!) ──
    build_gmp
    build_mpfr
    build_mpc

    # ── Build parser/lexer tools ──
    build_bison
    build_flex

    # ── Build toolchain ──
    build_binutils
    build_gcc
    build_gdb

    # ── Build GRUB tools ──
    build_xorriso
    build_grub

    # ── Verify ──
    verify_toolchain
    print_summary
}

main "$@"

