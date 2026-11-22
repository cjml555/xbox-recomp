#!/usr/bin/env bash
# Despliega los datos del juego MCLA al directorio que el runtime monta como game:
# (game_data_root = <filesDir>/assets, ver Runtime::SetupVfs en rexglue-sdk y
#  jni_bridge.cpp: FindXexFile -> xex_dir -> Runtime(xex_dir,...)).
# El ISO se usa SOLO localmente para reparar archivos incompletos; NUNCA se
# sube nada a ningún repositorio ni servicio remoto.
set -euo pipefail

PACKAGE="${PACKAGE:-com.mcla.recomp}"
ADB="${ADB:-/usr/bin/adb}"
DATA_DIR="${DATA_DIR:-/home/jhona/MCLA}"
ISO_PATH="${ISO_PATH:-$DATA_DIR/Midnight Club - Los Angeles - Complete Edition (USA, Europe).iso}"
STAGING_DIR="${STAGING_DIR:-$DATA_DIR/iso_extract}"
DEVICE_DIR="files/assets"
TMP_BASE="/data/local/tmp/mcla_deploy_$$"
XISO="${XISO:-$(command -v extract-xiso || true)}"
[[ -z "$XISO" && -x /home/jhona/.hermes/cache/scratch/extract-xiso/build/extract-xiso ]] && \
    XISO=/home/jhona/.hermes/cache/scratch/extract-xiso/build/extract-xiso

# Subconjunto opcional: FILES="xarchive_audlo.rpf xarchive_music.rpf" ./scripts/deploy_game_data.sh
if [[ -n "${FILES:-}" ]]; then
    REQUIRED=($FILES)
else
    REQUIRED=(
        default.xex
        xarchive_audio.rpf
        xarchive_audlo.rpf
        xarchive_cache.rpf
        xarchive_music.rpf
        nxeart
        attract576_16x9.bik attract576_4x3.bik attract720.bik
        intro576_16x9.bik intro576_4x3.bik intro720.bik
        root_directory_padding.pad
    )
fi

log() { printf '[deploy] %s\n' "$*"; }
die() { printf '[deploy] ERROR: %s\n' "$*" >&2; exit 1; }

ensure_tool() {
    [[ -n "$XISO" && -x "$XISO" ]] && return 0
    local srcdir="$STAGING_DIR/.extract-xiso-src"
    if [[ ! -x "$srcdir/build/extract-xiso" ]]; then
        log "Compilando extract-xiso desde XboxDev/extract-xiso..."
        mkdir -p "$STAGING_DIR"
        git clone --depth 1 https://github.com/XboxDev/extract-xiso.git "$srcdir" >/dev/null 2>&1 || \
            [[ -d "$srcdir" ]] || die "No se pudo clonar extract-xiso"
        cmake -S "$srcdir" -B "$srcdir/build" -DCMAKE_BUILD_TYPE=Release >/dev/null
        cmake --build "$srcdir/build" -j"$(nproc)" >/dev/null
    fi
    XISO="$srcdir/build/extract-xiso"
}

# 1) Reparar ficheros locales incompletos comparando con el directorio del ISO.
declare -A EXPECTED=()
if [[ -f "$ISO_PATH" ]]; then
    ensure_tool
    log "Leyendo tabla de directorios del ISO..."
    while IFS= read -r line; do
        if [[ "$line" =~ ^/(.+[^/])\ \(([0-9]+)\ bytes\)$ ]]; then
            EXPECTED["${BASH_REMATCH[1]}"]="${BASH_REMATCH[2]}"
        fi
    done < <("$XISO" -l "$ISO_PATH" 2>/dev/null)
    (( ${#EXPECTED[@]} > 0 )) || die "No se pudo listar el ISO: $ISO_PATH"

    # Extraer solo una vez (marcador). Este ISO está truncado: el directorio
    # anuncia 6.57 GB pero el image file tiene 5.10 GB; extract-xiso entrega
    # 0 bytes para music.rpf/nxeart/.bik. Tomamos la versión más completa
    # realmente extraíble y avisamos, en vez de fallar.
    if [[ ! -f "$STAGING_DIR/.extract_complete" ]]; then
        log "Extrayendo ISO a $STAGING_DIR (solo lectura local)..."
        mkdir -p "$STAGING_DIR"
        "$XISO" -x -d "$STAGING_DIR" "$ISO_PATH" || true
        touch "$STAGING_DIR/.extract_complete"
    fi

    for f in "${REQUIRED[@]}"; do
        want="${EXPECTED[$f]:-0}"
        have=0; staged=0
        [[ -f "$DATA_DIR/$f" ]] && have=$(stat -c %s "$DATA_DIR/$f")
        [[ -f "$STAGING_DIR/$f" ]] && staged=$(stat -c %s "$STAGING_DIR/$f")
        if (( staged > have )); then
            log "Reparando $f desde ISO ($staged bytes; antes $have)"
            cp -f "$STAGING_DIR/$f" "$DATA_DIR/$f"
        fi
        have=$(stat -c %s "$DATA_DIR/$f")
        if (( want > have )); then
            log "AVISO: ISO truncado para $f: directorio=$want, disponible=$have bytes"
        fi
    done
else
    log "ISO no encontrado ($ISO_PATH); usando archivos locales tal cual"
fi

for f in "${REQUIRED[@]}"; do
    [[ -f "$DATA_DIR/$f" ]] || die "Falta el archivo local $DATA_DIR/$f"
done

# 2) Despliegue idempotente al device. adb push -> /data/local/tmp -> run-as cp.
trap '$ADB shell "rm -f $TMP_BASE" >/dev/null 2>&1 || true' EXIT
$ADB get-state >/dev/null || die "adb no ve ningún device"

deployed=0
for f in "${REQUIRED[@]}"; do
    local_size=$(stat -c %s "$DATA_DIR/$f")
    remote_size=$($ADB shell "run-as $PACKAGE sh -c 'if [ -f $DEVICE_DIR/$f ]; then stat -c %s $DEVICE_DIR/$f; else echo MISSING; fi'" 2>/dev/null | tr -d '\r' | tail -1)
    [[ "$remote_size" =~ ^[0-9]+$ ]] || remote_size=MISSING
    if [[ "$remote_size" == "$local_size" ]]; then
        log "OK (ya desplegado): $f ($local_size bytes)"
        continue
    fi
    log "Pushing $f ($local_size bytes; device=$remote_size)..."
    $ADB push "$DATA_DIR/$f" "$TMP_BASE/$f" >/dev/null
    $ADB shell "run-as $PACKAGE sh -c 'mkdir -p $DEVICE_DIR && cp $TMP_BASE/$f $DEVICE_DIR/$f && chmod 600 $DEVICE_DIR/$f'" >/dev/null
    verify=$($ADB shell "run-as $PACKAGE stat -c %s $DEVICE_DIR/$f" | tr -d '\r')
    [[ "$verify" == "$local_size" ]] || die "Verificación fallida para $f ($verify != $local_size)"
    $ADB shell "rm -f $TMP_BASE/$f" >/dev/null
    log "Desplegado y verificado: $f"
    deployed=$((deployed+1))
done

log "Despliegue completado ($deployed archivos nuevos). Contenido de game: en el device:"
$ADB shell "run-as $PACKAGE ls -l $DEVICE_DIR"
