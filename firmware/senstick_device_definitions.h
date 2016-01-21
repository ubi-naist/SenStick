#ifndef senstick_device_definitions_h
#define senstick_device_definitions_h

// Bluetooth 製品登録情報
// Member Company	matilde inc
// Declaration ID	D025629
// Specification Name 4.1

#define MANUFACTURER_NAME          "Matilde Inc."   /* 製造者名 */

// ビルドを表す番号。フィンガープリントの一部にして使われます。
// この値はビルドの順序を表しません。値が異なることだけが、意味を持ちます。 0x01 の次が 0x0f になっていても、かまいません。
// デバッグ時にEEPROMを工場出荷状態からテストしたいときには、この値を変更して使います。
#define BUILD_NUMBER               0x0001

// 古いiOSアプリケーションからも、デバイスを発見できるように、 デバイス名は、SDKのバージョンにかかわらず、これまでと同じFUNIKIとする。
#define DEVICE_NAME                "NARA-ACT"    /* アドバタイジング、GAPのデバイス名 */
//#define HARDWARE_REVISION           0x0100       /* ハードウェアのリビジョン。ver 1.1 0xJJMN で表記。 Jはメイジャー、Mはマイナー、Nはサブマイナーバージョン番号を示す。*/
#define HARDWARE_REVISION_STRING    "rev 1.0"  /* ハードウェアのリビジョンを表す文字列。Device Information Serviceで使います */
//#define	FIRMWARE_REVISION           0x0100       /* ファームウェアのリビジョン。ver0.6 0xJJMN の表記。 */
#define FIRMWARE_REVISION_STRING    "rev 1.0"  /* ファームウェアのリビジョンを表す文字列。Device Information Serviceで使います */


#endif /* senstick_device_definitions_h */
