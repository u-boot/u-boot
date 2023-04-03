#!/bin/sh

if [ -z "$1" ]; then
	echo "Usage: $0 KEY"
	exit 1
fi

TEMP_X509=$(mktemp XXXXXXXX.temp)

REVISION=${2:-0}
SHA_VAL=$(openssl dgst -sha512 -hex tispl.bin | sed -e "s/^.*= //g")
BIN_SIZE=$(stat -c %s tispl.bin)

cat <<EOF >$TEMP_X509
[ req ]
distinguished_name     = req_distinguished_name
x509_extensions        = v3_ca
prompt                 = no
dirstring_type         = nobmp

[ req_distinguished_name ]
CN                     = IOT2050 Firmware Signature

[ v3_ca ]
basicConstraints       = CA:true
1.3.6.1.4.1.294.1.3    = ASN1:SEQUENCE:swrv
1.3.6.1.4.1.294.1.34   = ASN1:SEQUENCE:sysfw_image_integrity

[ swrv ]
swrv = INTEGER:$REVISION

[ sysfw_image_integrity ]
shaType                = OID:2.16.840.1.101.3.4.2.3
shaValue               = FORMAT:HEX,OCT:$SHA_VAL
imageSize              = INTEGER:$BIN_SIZE
EOF

CERT_X509=$(mktemp XXXXXXXX.crt)

openssl req -new -x509 -key $1 -nodes -outform DER -out $CERT_X509 -config $TEMP_X509 -sha512
cat $CERT_X509 tispl.bin > tispl.bin_signed
# currently broken in upstream
#source/tools/binman/binman replace -i flash.bin -f tispl.bin_signed blob@0x180000
dd if=tispl.bin_signed of=flash.bin bs=$((0x1000)) seek=$((0x180000/0x1000)) conv=notrunc

rm $TEMP_X509 $CERT_X509

tools/mkimage -G $1 -r -o sha256,rsa4096 -F fit@0x380000.fit
# currently broken in upstream
#source/tools/binman/binman replace -i flash.bin -f fit@0x380000.fit fit@0x380000
dd if=fit@0x380000.fit of=flash.bin bs=$((0x1000)) seek=$((0x380000/0x1000)) conv=notrunc
