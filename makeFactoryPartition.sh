#!/bin/bash

if (( "$#" == 1 )); then
  FACTORY_PARTITION_PATH=$1
else  
  FACTORY_PARTITION_PATH=${FACTORY_PARTITION:-"./factory_partition"}
fi

partition_file=$(find $FACTORY_PARTITION_PATH -name "*-partition.bin" -print -quit 2>/dev/null)
if [[ ${#partition_file} != 0 ]]; then
    echo Generated $partition_file already exist. Please cleanup and retry.
    exit 1
fi

if [[ "$FACTORY_PARTITION_PATH" == "./factory_partition" ]]; then
    source ./factoryData.sh
else
    source $FACTORY_PARTITION_PATH/factoryData.sh
fi

read -rp "Enter count of manufacturing partition binaries to generate: " count

if ! [[ "$count" =~ ^[1-9][0-9]*$ ]]; then
    echo "Error: input must be a number greater than zero."
    exit 1
fi

MATTER_CRED_PATH=$ESP_MATTER_PATH/connectedhomeip/connectedhomeip/credentials/test
mkdir -p $FACTORY_PARTITION_PATH/cert
$ESP_MATTER_PATH/connectedhomeip/connectedhomeip/out/host/chip-cert \
  gen-att-cert \
  --type i \
  --subject-cn "Matter Test PAI" \
  --subject-vid "${VENDOR_ID}" \
  --subject-pid "${PRODUCT_ID}" \
  --valid-from "2026-01-01 12:00:00" \
  --lifetime "4294967295" \
  --ca-key $MATTER_CRED_PATH/attestation/Chip-Test-PAA-NoVID-Key.pem \
  --ca-cert $MATTER_CRED_PATH/attestation/Chip-Test-PAA-NoVID-Cert.pem \
  --out-key $FACTORY_PARTITION_PATH/cert/Chip-Test-PAI-${VENDOR_ID}-${PRODUCT_ID}-key.pem \
  --out $FACTORY_PARTITION_PATH/cert/Chip-Test-PAI-${VENDOR_ID}-${PRODUCT_ID}-cert.pem

$ESP_MATTER_PATH/connectedhomeip/connectedhomeip/out/host/chip-cert \
  gen-cd \
  --key $MATTER_CRED_PATH/certification-declaration/Chip-Test-CD-Signing-Key.pem \
  --cert $MATTER_CRED_PATH/certification-declaration/Chip-Test-CD-Signing-Cert.pem \
  --out $FACTORY_PARTITION_PATH/cert/Chip-Test-CD-${VENDOR_ID}-${PRODUCT_ID}.der \
  --format-version "1" \
  --vendor-id "${VENDOR_ID}" \
  --product-id "${PRODUCT_ID}" \
  --device-type-id ${DEVICE_TYPE_ID} \
  --certificate-id "ZIG20141ZB330001-24" \
  --security-level "0" \
  --security-info "0" \
  --version-number "9876" \
  --certification-type "0"

esp-matter-mfg-tool \
  --outdir $FACTORY_PARTITION_PATH \
  --cn-prefix $CN_PREFIX \
  --count $count \
  --target "$IDF_TARGET" \
  --vendor-id 0x$VENDOR_ID \
  --vendor-name "$VENDOR_NAME" \
  --product-id 0x$PRODUCT_ID \
  --product-name "$PRODUCT_NAME" \
  --hw-ver $HW_VER \
  --hw-ver-str "$HW_VER_STR" \
  --product-label "$PRODUCT_LABEL" \
  --product-url "$PRODUCT_URL" \
  --pai \
  --key "$FACTORY_PARTITION_PATH/cert/Chip-Test-PAI-$VENDOR_ID-$PRODUCT_ID-Key.pem" \
  --cert "$FACTORY_PARTITION_PATH/cert/Chip-Test-PAI-$VENDOR_ID-$PRODUCT_ID-Cert.pem" \
  --cert-dclrn "$FACTORY_PARTITION_PATH/cert/Chip-Test-CD-$VENDOR_ID-$PRODUCT_ID.der"
