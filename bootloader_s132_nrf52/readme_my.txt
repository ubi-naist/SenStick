Usage:
1. If proto file changes:

You will need to download the protoc compiler from:
https://developers.google.com/protocol-buffers/docs/downloads

    A. From examples\dfu\bootloader_secure\, run:

       protoc.exe -odfu-cc.pb dfu-cc.proto
プロトコルのバージョン指定がないとワーニングがでるけど、まず無視。
Brew install protobuf
Pythonで使うためのパス作りと設定のメッセージが出るから指示通りで。

       This generates a 'dfu-cc.pb' file, which will be used by the Python script

    B. From examples\dfu\bootloader_secure\, run python script:

       python ..\..\..\external\nano-pb\generator\nanopb_generator.py dfu-cc.pb -f dfu-cc.options
ここだと、../nRF5_SDK_12//external/nano-pb/generator/nanopb_generator.py
       This produces 2 files: dfu-cc.pb.c and dfu-cc.pb.h, which will overwrite the old ones if present.

2. File:

   - pb.h

   Should be used to use PB.

3. Additionally, the following may be used to decode/encode messages:

   - pb_common.h
   - pb_common.c
   (to decode:)
   - pb_decode.h
   - pb_decode.c
   (to encode:)
   - pb_encode.h
   - pb_endode.c
