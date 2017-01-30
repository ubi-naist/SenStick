#ifndef senstick_device_definition_h
#define senstick_device_definition_h

#define MANUFACTURER_NAME          "Matilde Inc."   /* 製造者名 */

//アドバタイジング、GAPのデバイス名
#ifdef PANASONIC
#define DEVICE_NAME                "PanaStick"
#else
#define DEVICE_NAME                "NARA-ACT"
#endif

// ハードウェアバージョン
#ifdef NRF52
#define HARDWARE_REVISION_STRING    "rev 2.0"  // ハードウェアのリビジョンを表す文字列。Device Information Serviceで使います。
#else // NRF51
#define HARDWARE_REVISION_STRING    "rev 1.0"  // ハードウェアのリビジョンを表す文字列。Device Information Serviceで使います。
#endif

// ファームウェアは、機能が同じであるならば、同じ番号を用いる。
// FIRMWARE_REVISIONは、ファームウェアのリビジョン。先頭1バイトがメジャーバージョン、後ろ1バイトがマイナーバージョン 0xJJMN の表記。
// FIRMWARE_REVISION_STRINGは、ファームウェアのリビジョンを表す文字列。Device Information Serviceで使います
#define	FIRMWARE_REVISION           0x0107
#define FIRMWARE_REVISION_STRING    "rev 1.07"

#endif /* senstick_device_definition_h */
