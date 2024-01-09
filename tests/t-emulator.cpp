#include "t-index.h"

i32 emulateSimpleMov() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * mov ax, 1
     * mov bx, 2
     * mov cx, 3
     * mov dx, 4
     *
     * mov sp, 5
     * mov bp, 6
     * mov si, 7
     * mov di, 8
     *
    */
    core::Arr<u8> binaryData;
    binaryData
        .append(0xb8).append(0x01).append(0x00).append(0xbb).append(0x02).append(0x00).append(0xb9)
        .append(0x03).append(0x00).append(0xba).append(0x04).append(0x00).append(0xbc).append(0x05)
        .append(0x00).append(0xbd).append(0x06).append(0x00).append(0xbe).append(0x07).append(0x00)
        .append(0xbf).append(0x08).append(0x00);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    asm8086::EmulationOpts options = {};
    EmulationContext ectx = asm8086::createEmulationCtx(core::move(ctx.instructions), options);

    asm8086::emulate(ectx);

    Assert( ectx.registers[i32(RegisterType::AX)].value == 1 );
    Assert( ectx.registers[i32(RegisterType::BX)].value == 2 );
    Assert( ectx.registers[i32(RegisterType::CX)].value == 3 );
    Assert( ectx.registers[i32(RegisterType::DX)].value == 4 );
    Assert( ectx.registers[i32(RegisterType::SP)].value == 5 );
    Assert( ectx.registers[i32(RegisterType::BP)].value == 6 );
    Assert( ectx.registers[i32(RegisterType::SI)].value == 7 );
    Assert( ectx.registers[i32(RegisterType::DI)].value == 8 );

    Assert( ectx.registers[i32(RegisterType::ES)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::CS)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::IP)].value == binaryData.len() );
    Assert( ectx.registers[i32(RegisterType::FLAGS)].value == 0 );

    return 0;
}

i32 runEmulatorTestsSuite() {
    RunTest(emulateSimpleMov);

    return 0;
}
