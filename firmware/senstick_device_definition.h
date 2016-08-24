#ifndef senstick_device_definition_h
#define senstick_device_definition_h

#define MANUFACTURER_NAME          "Matilde Inc."   /* 製造者名 */

//アドバタイジング、GAPのデバイス名
#ifdef PANASONIC
#define DEVICE_NAME                "PanaStick"
#else
#define DEVICE_NAME                "NARA-ACT"
#endif

//#define HARDWARE_REVISION           0x0100       /* ハードウェアのリビジョン。ver 1.1 0xJJMN で表記。 Jはメイジャー、Mはマイナー、Nはサブマイナーバージョン番号を示す。*/
#define HARDWARE_REVISION_STRING    "rev 1.0"  /* ハードウェアのリビジョンを表す文字列。Device Information Serviceで使います */

// FIRMWARE_REVISIONは、ファームウェアのリビジョン。先頭1バイトがメジャーバージョン、後ろ1バイトがマイナーバージョン 0xJJMN の表記。
// FIRMWARE_REVISION_STRINGは、ファームウェアのリビジョンを表す文字列。Device Information Serviceで使います
#define	FIRMWARE_REVISION           0x0103
#define FIRMWARE_REVISION_STRING    "rev 1.03"

#endif /* senstick_device_definition_h */
