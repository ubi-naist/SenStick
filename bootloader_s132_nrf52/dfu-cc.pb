
�
dfu-cc.protodfu"F
Hash*
	hash_type (2.dfu.HashTypeRhashType
hash (Rhash"�
InitCommand

fw_version (R	fwVersion

hw_version (R	hwVersion
sd_req (BRsdReq
type (2.dfu.FwTypeRtype
sd_size (RsdSize
bl_size (RblSize
app_size (RappSize
hash (2	.dfu.HashRhash 
is_debug	 (:falseRisDebug"(
ResetCommand
timeout (Rtimeout"~
Command$
op_code (2.dfu.OpCodeRopCode$
init (2.dfu.InitCommandRinit'
reset (2.dfu.ResetCommandRreset"�
SignedCommand&
command (2.dfu.CommandRcommand9
signature_type (2.dfu.SignatureTypeRsignatureType
	signature (R	signature"k
Packet&
command (2.dfu.CommandRcommand9
signed_command (2.dfu.SignedCommandRsignedCommand*
OpCode	
RESET 
INIT*T
FwType
APPLICATION 

SOFTDEVICE

BOOTLOADER
SOFTDEVICE_BOOTLOADER*D
HashType
NO_HASH 
CRC

SHA128

SHA256

SHA512*3
SignatureType
ECDSA_P256_SHA256 
ED25519