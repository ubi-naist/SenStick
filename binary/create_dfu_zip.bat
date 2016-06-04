fromelf --bin --output o.bin %1
nrfutil dfu genpkg dfu.zip --application o.bin --application-version 0x0100 --dev-revision 1 --dev-type 1 --sd-req 0x64
del o.bin
