#ifndef senstick_device_definition_h
#define senstick_device_definition_h

// Bluetooth 製品登録情報
// Member Company	matilde inc
// Declaration ID	D025629
// Specification Name 4.1

#define MANUFACTURER_NAME          "Matilde Inc."   /* 製造者名 */

// 古いiOSアプリケーションからも、デバイスを発見できるように、 デバイス名は、SDKのバージョンにかかわらず、これまでと同じFUNIKIとする。
#define DEVICE_NAME                "NARA-ACT"    /* アドバタイジング、GAPのデバイス名 */
//#define HARDWARE_REVISION           0x0100       /* ハードウェアのリビジョン。ver 1.1 0xJJMN で表記。 Jはメイジャー、Mはマイナー、Nはサブマイナーバージョン番号を示す。*/
#define HARDWARE_REVISION_STRING    "rev 1.0"  /* ハードウェアのリビジョンを表す文字列。Device Information Serviceで使います */
#define	FIRMWARE_REVISION           0x0101     /* ファームウェアのリビジョン。先頭1バイトがメジャーバージョン、後ろ1バイトがマイナーバージョン 0xJJMN の表記。 */
#define FIRMWARE_REVISION_STRING    "rev 1.01"  /* ファームウェアのリビジョンを表す文字列。Device Information Serviceで使います */


#endif /* senstick_device_definition_h */
