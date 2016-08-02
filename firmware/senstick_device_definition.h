#ifndef senstick_device_definition_h
#define senstick_device_definition_h

// Bluetooth 製品登録情報
// Member Company	matilde inc
// Declaration ID	D025629
// Specification Name 4.1

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
#define	FIRMWARE_REVISION           0x0102
#define FIRMWARE_REVISION_STRING    "rev 1.02"

#endif /* senstick_device_definition_h */
