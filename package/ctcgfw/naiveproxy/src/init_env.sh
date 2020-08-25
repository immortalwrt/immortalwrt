#!/bin/bash
# [CTCGFW] Project-OpenWrt
# Use it under GPLv3.
# --------------------------------------------------------
# Init build dependencies for naiveproxy

# Read args from shell
target_arch="$1"
target_board="$2"
cpu_type="$3"
cpu_subtype="$4"
toolchain_dir="$5"
dl_dir="$6"

# Set arch info
naive_arch="${target_arch}"
[ "${target_arch}" == "i386" ] && naive_arch="x86"
[ "${target_arch}" == "x86_64" ] && naive_arch="x64"
[ "${target_arch}" == "aarch64" ] && naive_arch="arm64"
ldso_path="/lib/$(find "${toolchain_dir}/" | grep -Eo "ld-musl-[a-z0-9_-]+\\.so\\.1")"

# OS detection
[ "$(uname)" != "Linux" -o "$(uname -m)" != "x86_64" ] && { echo -e "Support Linux AMD64 only."; exit 1; }

cd "$PWD/src"

# AFDO profile
[ ! -f "chrome/android/profiles/afdo.prof" ] && {
	AFDO_NAME="$(cat "chrome/android/profiles/newest.txt")"
	[ ! -f "${dl_dir}/${AFDO_NAME}" ] && curl -f --connect-timeout 20 --retry 5 --location --insecure "https://storage.googleapis.com/chromeos-prebuilt/afdo-job/llvm/${AFDO_NAME}" -o "${dl_dir}/${AFDO_NAME}"
	bzip2 -cd > "chrome/android/profiles/afdo.prof" < "${dl_dir}/${AFDO_NAME}"
}

# Download Clang
[ ! -d "third_party/llvm-build/Release+Asserts/bin" ] && {
	mkdir -p "third_party/llvm-build/Release+Asserts"
	CLANG_REVISION="$(python3 "tools/clang/scripts/update.py" --print-revision)"
	[ ! -f "${dl_dir}/clang-${CLANG_REVISION}.tgz" ] && curl -f --connect-timeout 20 --retry 5 --location --insecure "https://commondatastorage.googleapis.com/chromium-browser-clang/Linux_x64/clang-${CLANG_REVISION}.tgz" -o "${dl_dir}/clang-${CLANG_REVISION}.tgz"
	tar -xzf "${dl_dir}/clang-${CLANG_REVISION}.tgz" -C "third_party/llvm-build/Release+Asserts"
}

# Download GN tool
[ ! -f "gn/out/gn" ] && {
	mkdir -p "gn/out"
	GN_VERSION="$(grep "'gn_version':" "buildtools/DEPS" | cut -d"'" -f4)"
	[ ! -f "${dl_dir}/gn-${GN_VERSION}.zip" ] && curl -f --connect-timeout 20 --retry 5 --location --insecure "https://chrome-infra-packages.appspot.com/dl/gn/gn/linux-amd64/+/${GN_VERSION}" -o "${dl_dir}/gn-${GN_VERSION}.zip"
	unzip -o "${dl_dir}/gn-${GN_VERSION}.zip" -d "gn/out"
}

# Set ENV
export DEPOT_TOOLS_WIN_TOOLCHAIN=0
export naive_flags="
is_official_build=true
exclude_unwind_tables=true
enable_resource_whitelist_generation=false
symbol_level=0
is_clang=true
use_sysroot=false

use_allocator=\"none\"
use_allocator_shim=false

fatal_linker_warnings=false
treat_warnings_as_errors=false

fieldtrial_testing_like_official_build=true

enable_nacl=false
enable_print_preview=false
enable_remoting=false
use_alsa=false
use_cups=false
use_dbus=false
use_gio=false
use_platform_icu_alternatives=true
use_gtk=false
use_system_libdrm=false
use_gnome_keyring=false
use_libpci=false
use_pangocairo=false
use_aura=false
use_glib=false
use_pulseaudio=false
use_udev=false

disable_file_support=true
enable_websockets=false
disable_ftp_support=true
use_kerberos=false
enable_mdns=false
enable_reporting=false
include_transport_security_state_preload_list=false
rtc_use_pipewire=false

use_xkbcommon=false
use_ozone=true
ozone_auto_platforms=false
ozone_platform=\"headless\"
ozone_platform_headless=true

current_os=\"linux\"
current_cpu=\"${naive_arch}\"
sysroot=\"${toolchain_dir}\"
custom_toolchain=\"//build/toolchain/linux:clang_${naive_arch}_openwrt\"
ldso_path=\"${ldso_path}\""
[ "${target_arch}" == "arm" ] && {
	naive_flags="${naive_flags} arm_version=0 arm_cpu=\"${cpu_type}\""
	[ -n "${cpu_subtype}" ] && { echo "${cpu_subtype}" | grep -q "neon" && neon_flag="arm_use_neon=true" || neon_flag="arm_use_neon=false"; naive_flags="${naive_flags} arm_fpu=\"${cpu_subtype}\" arm_float_abi=\"hard\" ${neon_flag}"; } || naive_flags="${naive_flags} arm_float_abi=\"soft\" arm_use_neon=false"
}
[[ "mips mips64 mipsel mips64el" =~ (^|[[:space:]])"${target_arch}"($|[[:space:]]) ]] && {
	naive_flags="${naive_flags} use_gold=false is_cfi=false use_cfi_icall=false use_thin_lto=false mips_arch_variant=\"r2\""
	[[ "${target_arch}" =~ ^"mips"$|^"mipsel"$ ]] && naive_flags="mips_float_abi=\"soft\" mips_tune=\"${cpu_type}\""
}
