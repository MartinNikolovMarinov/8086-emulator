#include "t-index.h"

i32 emulateSimpleMovTest() {
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

i32 emulateMemoryToRegisterMovTest() {
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
     * mov sp, ax
     * mov bp, bx
     * mov si, cx
     * mov di, dx
     *
     * mov dx, sp
     * mov cx, bp
     * mov bx, si
     * mov ax, di
     *
    */
    core::Arr<u8> binaryData;

    binaryData
        .append(0xb8).append(0x01).append(0x00).append(0xbb).append(0x02).append(0x00).append(0xb9)
        .append(0x03).append(0x00).append(0xba).append(0x04).append(0x00).append(0x89).append(0xc4)
        .append(0x89).append(0xdd).append(0x89).append(0xce).append(0x89).append(0xd7).append(0x89)
        .append(0xe2).append(0x89).append(0xe9).append(0x89).append(0xf3).append(0x89).append(0xf8);


    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    asm8086::EmulationOpts options = {};
    EmulationContext ectx = asm8086::createEmulationCtx(core::move(ctx.instructions), options);

    asm8086::emulate(ectx);

    Assert( ectx.registers[i32(RegisterType::AX)].value == 4 );
    Assert( ectx.registers[i32(RegisterType::BX)].value == 3 );
    Assert( ectx.registers[i32(RegisterType::CX)].value == 2 );
    Assert( ectx.registers[i32(RegisterType::DX)].value == 1 );
    Assert( ectx.registers[i32(RegisterType::SP)].value == 1 );
    Assert( ectx.registers[i32(RegisterType::BP)].value == 2 );
    Assert( ectx.registers[i32(RegisterType::SI)].value == 3 );
    Assert( ectx.registers[i32(RegisterType::DI)].value == 4 );

    Assert( ectx.registers[i32(RegisterType::ES)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::CS)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::IP)].value == binaryData.len() );
    Assert( ectx.registers[i32(RegisterType::FLAGS)].value == 0 );

    return 0;
}

i32 emulateHalfRegisterMovTest() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * mov ax, 0x2222
     * mov bx, 0x4444
     * mov cx, 0x6666
     * mov dx, 0x8888
     *
     * mov ss, ax
     * mov ds, bx
     * mov es, cx
     *
     * mov al, 0x11
     * mov bh, 0x33
     * mov cl, 0x55
     * mov dh, 0x77
     *
     * mov ah, bl
     * mov cl, dh
     *
     * mov ss, ax
     * mov ds, bx
     * mov es, cx
     *
     * mov sp, ss
     * mov bp, ds
     * mov si, es
     * mov di, dx
     *
    */
    core::Arr<u8> binaryData;

    binaryData
        .append(0xb8).append(0x22).append(0x22).append(0xbb).append(0x44).append(0x44).append(0xb9)
        .append(0x66).append(0x66).append(0xba).append(0x88).append(0x88).append(0x8e).append(0xd0)
        .append(0x8e).append(0xdb).append(0x8e).append(0xc1).append(0xb0).append(0x11).append(0xb7)
        .append(0x33).append(0xb1).append(0x55).append(0xb6).append(0x77).append(0x88).append(0xdc)
        .append(0x88).append(0xf1).append(0x8e).append(0xd0).append(0x8e).append(0xdb).append(0x8e)
        .append(0xc1).append(0x8c).append(0xd4).append(0x8c).append(0xdd).append(0x8c).append(0xc6)
        .append(0x89).append(0xd7);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    asm8086::EmulationOpts options = {};
    EmulationContext ectx = asm8086::createEmulationCtx(core::move(ctx.instructions), options);

    asm8086::emulate(ectx);

    Assert( ectx.registers[i32(RegisterType::AX)].value == 0x4411 );
    Assert( ectx.registers[i32(RegisterType::BX)].value == 0x3344 );
    Assert( ectx.registers[i32(RegisterType::CX)].value == 0x6677 );
    Assert( ectx.registers[i32(RegisterType::DX)].value == 0x7788 );
    Assert( ectx.registers[i32(RegisterType::SP)].value == 0x4411 );
    Assert( ectx.registers[i32(RegisterType::BP)].value == 0x3344 );
    Assert( ectx.registers[i32(RegisterType::SI)].value == 0x6677 );
    Assert( ectx.registers[i32(RegisterType::DI)].value == 0x7788 );

    Assert( ectx.registers[i32(RegisterType::ES)].value == 0x6677 );
    Assert( ectx.registers[i32(RegisterType::SS)].value == 0x4411 );
    Assert( ectx.registers[i32(RegisterType::DS)].value == 0x3344 );
    Assert( ectx.registers[i32(RegisterType::CS)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::IP)].value == binaryData.len() );
    Assert( ectx.registers[i32(RegisterType::FLAGS)].value == 0 );

    return 0;
}

i32 emulateCarryAndSignFlagInstructionsTest() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * mov bx, -4093
     * mov cx, 3841
     * sub bx, cx
     *
     * mov sp, 998
     * mov bp, 999
     * cmp bp, sp
     *
     * add bp, 1027
     * sub bp, 2026
     *
    */
    core::Arr<u8> binaryData;

    binaryData.append(0xbb).append(0x03).append(0xf0).append(0xb9).append(0x01).append(0x0f).append(0x29)
              .append(0xcb).append(0xbc).append(0xe6).append(0x03).append(0xbd).append(0xe7).append(0x03)
              .append(0x39).append(0xe5).append(0x81).append(0xc5).append(0x03).append(0x04).append(0x81)
              .append(0xed).append(0xea).append(0x07);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    asm8086::EmulationOpts options = {};
    EmulationContext ectx = asm8086::createEmulationCtx(core::move(ctx.instructions), options);

    asm8086::emulate(ectx);

    Assert( ectx.registers[i32(RegisterType::AX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::BX)].value == 0xE102 );
    Assert( ectx.registers[i32(RegisterType::CX)].value == 0x0F01 );
    Assert( ectx.registers[i32(RegisterType::DX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SP)].value == 0x03E6 );
    Assert( ectx.registers[i32(RegisterType::BP)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SI)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DI)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::ES)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::CS)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::IP)].value == binaryData.len() );
    asm8086::Flags expectedFlags = asm8086::Flags(asm8086::Flags::CPU_FLAG_PARITY_FLAG |
                                                  asm8086::Flags::CPU_FLAG_ZERO_FLAG);
    Assert( ectx.registers[i32(RegisterType::FLAGS)].value == expectedFlags );

    return 0;
}

i32 emulateMoreFlagsTest() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * add bx, 30000
     * add bx, 10000
     * sub bx, 5000
     * sub bx, 5000
     *
     * mov bx, 1
     * mov cx, 100
     * add bx, cx
     *
     * mov dx, 10
     * sub cx, dx
     *
     * add bx, 40000
     * add cx, -90
     *
     * mov sp, 99
     * mov bp, 98
     * cmp bp, sp
     *
    */
    core::Arr<u8> binaryData;

    binaryData
        .append(0x81).append(0xc3).append(0x30).append(0x75).append(0x81).append(0xc3).append(0x10)
        .append(0x27).append(0x81).append(0xeb).append(0x88).append(0x13).append(0x81).append(0xeb)
        .append(0x88).append(0x13).append(0xbb).append(0x01).append(0x00).append(0xb9).append(0x64)
        .append(0x00).append(0x01).append(0xcb).append(0xba).append(0x0a).append(0x00).append(0x29)
        .append(0xd1).append(0x81).append(0xc3).append(0x40).append(0x9c).append(0x83).append(0xc1)
        .append(0xa6).append(0xbc).append(0x63).append(0x00).append(0xbd).append(0x62).append(0x00)
        .append(0x39).append(0xe5);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    asm8086::EmulationOpts options = {};
    EmulationContext ectx = asm8086::createEmulationCtx(core::move(ctx.instructions), options);

    asm8086::emulate(ectx);

    Assert( ectx.registers[i32(RegisterType::AX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::BX)].value == 0x9CA5 );
    Assert( ectx.registers[i32(RegisterType::CX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DX)].value == 0x000A );
    Assert( ectx.registers[i32(RegisterType::SP)].value == 0x0063 );
    Assert( ectx.registers[i32(RegisterType::BP)].value == 0x0062 );
    Assert( ectx.registers[i32(RegisterType::SI)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DI)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::ES)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::CS)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::IP)].value == binaryData.len() );
    asm8086::Flags expectedFlags = asm8086::Flags(asm8086::Flags::CPU_FLAG_CARRY_FLAG |
                                                  asm8086::Flags::CPU_FLAG_AUX_CARRY_FLAG |
                                                  asm8086::Flags::CPU_FLAG_PARITY_FLAG |
                                                  asm8086::Flags::CPU_FLAG_SIGN_FLAG);
    Assert( ectx.registers[i32(RegisterType::FLAGS)].value == expectedFlags );

    return 0;
}

i32 emulateBasicIpChanges() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * mov cx, 200
     * mov bx, cx
     * add cx, 1000
     * mov bx, 2000
     * sub cx, bx
     *
    */
    core::Arr<u8> binaryData;

    binaryData
        .append(0xb9).append(0xc8).append(0x00).append(0x89).append(0xcb).append(0x81).append(0xc1)
        .append(0xe8).append(0x03).append(0xbb).append(0xd0).append(0x07).append(0x29).append(0xd9);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    asm8086::EmulationOpts options = {};
    EmulationContext ectx = asm8086::createEmulationCtx(core::move(ctx.instructions), options);

    asm8086::emulate(ectx);

    Assert( ectx.registers[i32(RegisterType::AX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::BX)].value == 0x07D0 );
    Assert( ectx.registers[i32(RegisterType::CX)].value == 0xFCE0 );
    Assert( ectx.registers[i32(RegisterType::DX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SP)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::BP)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SI)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DI)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::ES)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::CS)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::IP)].value == binaryData.len() );
    asm8086::Flags expectedFlags = asm8086::Flags(asm8086::Flags::CPU_FLAG_CARRY_FLAG |
                                                  asm8086::Flags::CPU_FLAG_SIGN_FLAG);
    Assert( ectx.registers[i32(RegisterType::FLAGS)].value == expectedFlags );

    return 0;
}

i32 emulateALoopTest() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * mov cx, 3
     * mov bx, 1000
     * loop_start:
     * add bx, 10
     * sub cx, 1
     * jnz loop_start
     *
    */
    core::Arr<u8> binaryData;

    binaryData.append(0xb9).append(0x03).append(0x00).append(0xbb).append(0xe8).append(0x03).append(0x83)
              .append(0xc3).append(0x0a).append(0x83).append(0xe9).append(0x01).append(0x75).append(0xf8);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    asm8086::EmulationOpts options = {};
    EmulationContext ectx = asm8086::createEmulationCtx(core::move(ctx.instructions), options);

    asm8086::emulate(ectx);

    Assert( ectx.registers[i32(RegisterType::AX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::BX)].value == 0x0406 );
    Assert( ectx.registers[i32(RegisterType::CX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SP)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::BP)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SI)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DI)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::ES)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::CS)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::IP)].value == binaryData.len() );
    asm8086::Flags expectedFlags = asm8086::Flags(asm8086::Flags::CPU_FLAG_PARITY_FLAG |
                                                  asm8086::Flags::CPU_FLAG_ZERO_FLAG);
    Assert( ectx.registers[i32(RegisterType::FLAGS)].value == expectedFlags );

    return 0;
}

i32 emulateCompliatedJumpInstructionsTest() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * mov ax, 10
     * mov bx, 10
     * mov cx, 10
     *
     * label_0:
     * cmp bx, cx
     * je label_1
     *
     * add ax, 1
     * jp label_2
     *
     * label_1:
     * sub bx, 5
     * jb label_3
     *
     * label_2:
     * sub cx, 2
     *
     * label_3:
     * loopnz label_0
     *
    */
    core::Arr<u8> binaryData;

    binaryData
        .append(0xb8).append(0x0a).append(0x00).append(0xbb).append(0x0a).append(0x00).append(0xb9)
        .append(0x0a).append(0x00).append(0x39).append(0xcb).append(0x74).append(0x05).append(0x83)
        .append(0xc0).append(0x01).append(0x7a).append(0x05).append(0x83).append(0xeb).append(0x05)
        .append(0x72).append(0x03).append(0x83).append(0xe9).append(0x02).append(0xe0).append(0xed);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    asm8086::EmulationOpts options = {};
    EmulationContext ectx = asm8086::createEmulationCtx(core::move(ctx.instructions), options);

    asm8086::emulate(ectx);

    Assert( ectx.registers[i32(RegisterType::AX)].value == 0x000D );
    Assert( ectx.registers[i32(RegisterType::BX)].value == 0XFFFB );
    Assert( ectx.registers[i32(RegisterType::CX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SP)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::BP)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SI)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DI)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::ES)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::CS)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::IP)].value == binaryData.len() );
    asm8086::Flags expectedFlags = asm8086::Flags(asm8086::Flags::CPU_FLAG_CARRY_FLAG |
                                                  asm8086::Flags::CPU_FLAG_AUX_CARRY_FLAG |
                                                  asm8086::Flags::CPU_FLAG_SIGN_FLAG);
    Assert( ectx.registers[i32(RegisterType::FLAGS)].value == expectedFlags );

    return 0;
}

i32 emulateBasicMemoryAddressingTest() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * mov word [1000], 1
     * mov word [1002], 2
     * mov word [1004], 3
     * mov word [1006], 4
     *
     * mov bx, 1000
     * mov word [bx + 4], 10
     *
     * mov bx, word [1000]
     * mov cx, word [1002]
     * mov dx, word [1004]
     * mov bp, word [1006]
     *
    */
    core::Arr<u8> binaryData;

    // 0xc7, 0x06, 0xe8, 0x03, 0x01, 0x00, 0xc7, 0x06, 0xea, 0x03, 0x02, 0x00,
    // 0xc7, 0x06, 0xec, 0x03, 0x03, 0x00, 0xc7, 0x06, 0xee, 0x03, 0x04, 0x00,
    // 0xbb, 0xe8, 0x03, 0xc7, 0x47, 0x04, 0x0a, 0x00, 0x8b, 0x1e, 0xe8, 0x03,
    // 0x8b, 0x0e, 0xea, 0x03, 0x8b, 0x16, 0xec, 0x03, 0x8b, 0x2e, 0xee, 0x03

    binaryData
        .append(0xc7).append(0x06).append(0xe8).append(0x03).append(0x01).append(0x00).append(0xc7)
        .append(0x06).append(0xea).append(0x03).append(0x02).append(0x00).append(0xc7).append(0x06)
        .append(0xec).append(0x03).append(0x03).append(0x00).append(0xc7).append(0x06).append(0xee)
        .append(0x03).append(0x04).append(0x00).append(0xbb).append(0xe8).append(0x03).append(0xc7)
        .append(0x47).append(0x04).append(0x0a).append(0x00).append(0x8b).append(0x1e).append(0xe8)
        .append(0x03).append(0x8b).append(0x0e).append(0xea).append(0x03).append(0x8b).append(0x16)
        .append(0xec).append(0x03).append(0x8b).append(0x2e).append(0xee).append(0x03);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    asm8086::EmulationOpts options = {};
    EmulationContext ectx = asm8086::createEmulationCtx(core::move(ctx.instructions), options);

    asm8086::emulate(ectx);

    Assert( ectx.registers[i32(RegisterType::AX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::BX)].value == 0x0001 );
    Assert( ectx.registers[i32(RegisterType::CX)].value == 0x0002 );
    Assert( ectx.registers[i32(RegisterType::DX)].value == 0x000A );
    Assert( ectx.registers[i32(RegisterType::SP)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::BP)].value == 0x0004 );
    Assert( ectx.registers[i32(RegisterType::SI)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DI)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::ES)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::CS)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::IP)].value == binaryData.len() );
    Assert( ectx.registers[i32(RegisterType::FLAGS)].value == 0 );

    return 0;
}

i32 emulateLoopWithMemoryAddressingTest() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * mov dx, 6
     * mov bp, 1000
     *
     * mov si, 0
     * init_loop_start:
     *     mov word [bp + si], si
     *     add si, 2
     *     cmp si, dx
     *     jnz init_loop_start
     *
     * mov bx, 0
     * mov si, 0
     * add_loop_start:
     *     mov cx, word [bp + si]
     *     add bx, cx
     *     add si, 2
     *     cmp si, dx
     *     jnz add_loop_start
     *
    */
   core::Arr<u8> binaryData;

    binaryData
        .append(0xba).append(0x06).append(0x00).append(0xbd).append(0xe8).append(0x03).append(0xbe)
        .append(0x00).append(0x00).append(0x89).append(0x32).append(0x83).append(0xc6).append(0x02)
        .append(0x39).append(0xd6).append(0x75).append(0xf7).append(0xbb).append(0x00).append(0x00)
        .append(0xbe).append(0x00).append(0x00).append(0x8b).append(0x0a).append(0x01).append(0xcb)
        .append(0x83).append(0xc6).append(0x02).append(0x39).append(0xd6).append(0x75).append(0xf5);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    asm8086::EmulationOpts options = {};
    EmulationContext ectx = asm8086::createEmulationCtx(core::move(ctx.instructions), options);

    asm8086::emulate(ectx);

    Assert( ectx.registers[i32(RegisterType::AX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::BX)].value == 0x0006 );
    Assert( ectx.registers[i32(RegisterType::CX)].value == 0x0004 );
    Assert( ectx.registers[i32(RegisterType::DX)].value == 0x0006 );
    Assert( ectx.registers[i32(RegisterType::SP)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::BP)].value == 0x03e8 );
    Assert( ectx.registers[i32(RegisterType::SI)].value == 0x0006 );
    Assert( ectx.registers[i32(RegisterType::DI)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::ES)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::CS)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::IP)].value == binaryData.len() );
    asm8086::Flags expectedFlags = asm8086::Flags(asm8086::Flags::CPU_FLAG_PARITY_FLAG |
                                                  asm8086::Flags::CPU_FLAG_ZERO_FLAG);
    Assert( ectx.registers[i32(RegisterType::FLAGS)].value == expectedFlags );

    return 0;
}

i32 emulateChallengeMemoryAddressing() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * mov dx, 6
     * mov bp, 1000
     *
     * mov si, 0
     * init_loop_start:
     *     mov word [bp + si], si
     *     add si, 2
     *     cmp si, dx
     *     jnz init_loop_start
     *
     * mov bx, 0
     * mov si, dx
     * sub bp, 2
     * add_loop_start:
     *     add bx, word [bp + si]
     *     sub si, 2
     *     jnz add_loop_start
     *
    */
    core::Arr<u8> binaryData;

    // 0xba, 0x06, 0x00, 0xbd, 0xe8, 0x03, 0xbe,
    // 0x00, 0x00, 0x89, 0x32, 0x83, 0xc6, 0x02,
    // 0x39, 0xd6, 0x75, 0xf7, 0xbb, 0x00, 0x00,
    // 0x89, 0xd6, 0x83, 0xed, 0x02, 0x03, 0x1a,
    // 0x83, 0xee, 0x02, 0x75, 0xf9

    binaryData
        .append(0xba).append(0x06).append(0x00).append(0xbd).append(0xe8).append(0x03).append(0xbe)
        .append(0x00).append(0x00).append(0x89).append(0x32).append(0x83).append(0xc6).append(0x02)
        .append(0x39).append(0xd6).append(0x75).append(0xf7).append(0xbb).append(0x00).append(0x00)
        .append(0x89).append(0xd6).append(0x83).append(0xed).append(0x02).append(0x03).append(0x1a)
        .append(0x83).append(0xee).append(0x02).append(0x75).append(0xf9);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    asm8086::EmulationOpts options = {};
    EmulationContext ectx = asm8086::createEmulationCtx(core::move(ctx.instructions), options);

    asm8086::emulate(ectx);

    Assert( ectx.registers[i32(RegisterType::AX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::BX)].value == 0x0006 );
    Assert( ectx.registers[i32(RegisterType::CX)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DX)].value == 0x0006 );
    Assert( ectx.registers[i32(RegisterType::SP)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::BP)].value == 0x03E6 );
    Assert( ectx.registers[i32(RegisterType::SI)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DI)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::ES)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::SS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::DS)].value == 0 );
    Assert( ectx.registers[i32(RegisterType::CS)].value == 0 );

    Assert( ectx.registers[i32(RegisterType::IP)].value == binaryData.len() );
    asm8086::Flags expectedFlags = asm8086::Flags(asm8086::Flags::CPU_FLAG_PARITY_FLAG |
                                                  asm8086::Flags::CPU_FLAG_ZERO_FLAG);
    Assert( ectx.registers[i32(RegisterType::FLAGS)].value == expectedFlags );

    return 0;
}

i32 emulateImageGenerationTest() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * ; Start image after one row, to avoid overwriting our code!
     * mov bp, 64*4
     *
     * mov dx, 0
     * y_loop_start:
     *
     *     mov cx, 0
     *     x_loop_start:
     *         ; Fill pixel
     *         mov word [bp + 0], cx ; Red
     *         mov word [bp + 2], dx ; Blue
     *         mov byte [bp + 3], 255 ; Alpha
     *
     *         ; Advance pixel location
     *         add bp, 4
     *
     *         ; Advance X coordinate and loop
     *         add cx, 1
     *         cmp cx, 64
     *         jnz x_loop_start
     *
     *     ; Advance Y coordinate and loop
     *     add dx, 1
     *     cmp dx, 64
     *     jnz y_loop_start
     *
    */
    core::Arr<u8> binaryData;

    binaryData
        .append(0xbd).append(0x00).append(0x01).append(0xba).append(0x00).append(0x00).append(0xb9)
        .append(0x00).append(0x00).append(0x89).append(0x4e).append(0x00).append(0x89).append(0x56)
        .append(0x02).append(0xc6).append(0x46).append(0x03).append(0xff).append(0x83).append(0xc5)
        .append(0x04).append(0x83).append(0xc1).append(0x01).append(0x83).append(0xf9).append(0x40)
        .append(0x75).append(0xeb).append(0x83).append(0xc2).append(0x01).append(0x83).append(0xfa)
        .append(0x40).append(0x75).append(0xe0);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    asm8086::EmulationOpts options = {};
    EmulationContext ectx = asm8086::createEmulationCtx(core::move(ctx.instructions), options);

    asm8086::emulate(ectx);

    // Just assert there was no infinite loop when generating.
    Assert( ectx.registers[i32(RegisterType::IP)].value == binaryData.len() );

    return 0;
}

i32 emulateImageGenerationWithBoarderTest() {
    /**
     * This binary data represents the following assembly code:
     * bits 16
     *
     * ; Start image after one row, to avoid overwriting our code!
     * mov bp, 64*4
     *
     * ; Draw the solid rectangle red/blue/alpha
     * mov dx, 64
     * y_loop_start:
     *
     *     mov cx, 64
     *     x_loop_start:
     *         mov byte [bp + 0], cl  ; Red
     *         mov byte [bp + 1], 0   ; Green
     *         mov byte [bp + 2], dl  ; Blue
     *         mov byte [bp + 3], 255 ; Alpha
     *         add bp, 4
     *
     *         loop x_loop_start
     *
     *     sub dx, 1
     *     jnz y_loop_start
     *
     * ; Add the line rectangle green
     * mov bp, 64*4 + 4*64 + 4
     * mov bx, bp
     * mov cx, 62
     * outline_loop_start:
     *
     *     mov byte [bp + 1], 255           ; Top line
     *     mov byte [bp + 61*64*4 + 1], 255 ; Bottom line
     *     mov byte [bx + 1], 255           ; Left line
     *     mov byte [bx + 61*4 + 1], 255    ; Right  line
     *
     *     add bp, 4
     *     add bx, 4*64
     *
     *     loop outline_loop_start
     *
    */
    core::Arr<u8> binaryData;

    // 0xbd, 0x00, 0x01, 0xba, 0x40, 0x00, 0xb9,
    // 0x40, 0x00, 0x88, 0x4e, 0x00, 0xc6, 0x46,
    // 0x01, 0x00, 0x88, 0x56, 0x02, 0xc6, 0x46,
    // 0x03, 0xff, 0x83, 0xc5, 0x04, 0xe2, 0xed,
    // 0x83, 0xea, 0x01, 0x75, 0xe5, 0xbd, 0x04,
    // 0x02, 0x89, 0xeb, 0xb9, 0x3e, 0x00, 0xc6,
    // 0x46, 0x01, 0xff, 0xc6, 0x86, 0x01, 0x3d,
    // 0xff, 0xc6, 0x47, 0x01, 0xff, 0xc6, 0x87,
    // 0xf5, 0x00, 0xff, 0x83, 0xc5, 0x04, 0x81,
    // 0xc3, 0x00, 0x01, 0xe2, 0xe5

    binaryData
        .append(0xbd).append(0x00).append(0x01).append(0xba).append(0x40).append(0x00).append(0xb9)
        .append(0x40).append(0x00).append(0x88).append(0x4e).append(0x00).append(0xc6).append(0x46)
        .append(0x01).append(0x00).append(0x88).append(0x56).append(0x02).append(0xc6).append(0x46)
        .append(0x03).append(0xff).append(0x83).append(0xc5).append(0x04).append(0xe2).append(0xed)
        .append(0x83).append(0xea).append(0x01).append(0x75).append(0xe5).append(0xbd).append(0x04)
        .append(0x02).append(0x89).append(0xeb).append(0xb9).append(0x3e).append(0x00).append(0xc6)
        .append(0x46).append(0x01).append(0xff).append(0xc6).append(0x86).append(0x01).append(0x3d)
        .append(0xff).append(0xc6).append(0x47).append(0x01).append(0xff).append(0xc6).append(0x87)
        .append(0xf5).append(0x00).append(0xff).append(0x83).append(0xc5).append(0x04).append(0x81)
        .append(0xc3).append(0x00).append(0x01).append(0xe2).append(0xe5);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    asm8086::EmulationOpts options = {};
    EmulationContext ectx = asm8086::createEmulationCtx(core::move(ctx.instructions), options);

    asm8086::emulate(ectx);

    // Just assert there was no infinite loop when generating.
    Assert( ectx.registers[i32(RegisterType::IP)].value == binaryData.len() );

    return 0;
}

i32 runEmulatorTestsSuite() {
    RunTest(emulateSimpleMovTest);
    RunTest(emulateMemoryToRegisterMovTest);
    RunTest(emulateHalfRegisterMovTest);
    RunTest(emulateCarryAndSignFlagInstructionsTest);
    RunTest(emulateMoreFlagsTest);
    RunTest(emulateBasicIpChanges);
    RunTest(emulateALoopTest);
    RunTest(emulateCompliatedJumpInstructionsTest);
    RunTest(emulateBasicMemoryAddressingTest);
    RunTest(emulateLoopWithMemoryAddressingTest);
    RunTest(emulateChallengeMemoryAddressing);
    RunTest(emulateImageGenerationTest);
    RunTest(emulateImageGenerationWithBoarderTest);

    return 0;
}
