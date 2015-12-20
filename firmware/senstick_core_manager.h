#ifndef senstick_core_manager_h
#define senstick_core_manager_h

// Senstickの、TWIおよびGPIOの統合動作を提供します。
// 例えば、センサーからデータを読みだして、フラッシュメモリに書き出す一連のシーケンスのように、周辺IOを束ねた逐次動作を提供します。

void init_senstick_core_manager(void);

// LEDの点灯/消灯
// TWIバスのリセット(電源落として、再起動)
// バッテリー電圧の取得


#endif /* senstick_core_manager_h */
