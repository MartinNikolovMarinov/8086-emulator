#include "t-index.h"

void assertContextDecodedAsExpected(const DecodingContext& ctx, const asm8086::Instruction* expected, addr_size expectedLen) {
    // FIXME: Uncomment this check !
    // Assert(ctx.instructions.len() == expectedLen);

    for (addr_size i = 0; i < expectedLen; i++) {
        Assert(ctx.instructions[i].opcode == expected[i].opcode);
        Assert(ctx.instructions[i].d == expected[i].d);
        Assert(ctx.instructions[i].s == expected[i].s);
        Assert(ctx.instructions[i].w == expected[i].w);
        Assert(ctx.instructions[i].mod == expected[i].mod);
        Assert(ctx.instructions[i].reg == expected[i].reg);
        Assert(ctx.instructions[i].rm == expected[i].rm);
        Assert(ctx.instructions[i].disp[0] == expected[i].disp[0]);
        Assert(ctx.instructions[i].disp[1] == expected[i].disp[1]);
        Assert(ctx.instructions[i].data[0] == expected[i].data[0]);
        Assert(ctx.instructions[i].data[1] == expected[i].data[1]);
        Assert(ctx.instructions[i].type == expected[i].type);
        Assert(ctx.instructions[i].byteCount == expected[i].byteCount);
        Assert(ctx.instructions[i].operands == expected[i].operands);
    }
}

i32 decodeOneInstructionsTest() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     * mov cx, bx
     *
    */
    core::Arr<u8> binaryData;
    binaryData.append(0x89).append(0xD9);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    constexpr asm8086::Instruction expectedInstructions[] = {
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b011,                                     // reg
            0b001,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
    };
    constexpr addr_size expectedInstructionsLen = sizeof(expectedInstructions) / sizeof(asm8086::Instruction);

    assertContextDecodedAsExpected(ctx, expectedInstructions, expectedInstructionsLen);

    // Start encoding

    core::StrBuilder out;

    ctx.options = DEC_OP_IMMEDIATE_AS_SIGNED;
    {
        asm8086::encodeAsm8086(out, ctx);
        const char* expectedAsm =
            "bits 16\n"
            "\n"
            "mov cx, bx\n";

        Assert(out.eq(expectedAsm), "Encoding failed.");
    }

    out.clear();
    ctx.options = DEC_OP_IMMEDIATE_AS_UNSIGNED;
    {
        asm8086::encodeAsm8086(out, ctx);
        const char* expectedAsm =
            "bits 16\n"
            "\n"
            "mov cx, bx\n";

        Assert(out.eq(expectedAsm), "Encoding failed.");
    }

    out.clear();
    ctx.options = DEC_OP_IMMEDIATE_AS_HEX;
    {
        asm8086::encodeAsm8086(out, ctx);
        const char* expectedAsm =
            "bits 16\n"
            "\n"
            "mov cx, bx\n";

        Assert(out.eq(expectedAsm), "Encoding failed.");
    }

    return 0;
}

i32 decodeMultipleInstructionsTest() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * mov cx, bx
     * mov ch, ah
     * mov dx, bx
     * mov si, bx
     * mov bx, di
     * mov al, cl
     * mov ch, ch
     * mov bx, ax
     * mov bx, si
     * mov sp, di
     * mov bp, ax
     *
    */
    core::Arr<u8> binaryData;
    binaryData.append(0x89).append(0xd9)
              .append(0x88).append(0xe5)
              .append(0x89).append(0xda)
              .append(0x89).append(0xde)
              .append(0x89).append(0xfb)
              .append(0x88).append(0xc8)
              .append(0x88).append(0xed)
              .append(0x89).append(0xc3)
              .append(0x89).append(0xf3)
              .append(0x89).append(0xfc)
              .append(0x89).append(0xc5);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    constexpr asm8086::Instruction expectedInstructions[] = {
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b011,                                     // reg
            0b001,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            0,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b100,                                     // reg
            0b101,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b011,                                     // reg
            0b010,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b011,                                     // reg
            0b110,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b111,                                     // reg
            0b011,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            0,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b001,                                     // reg
            0b000,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            0,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b101,                                     // reg
            0b101,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b000,                                     // reg
            0b011,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b110,                                     // reg
            0b011,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b111,                                     // reg
            0b100,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b000,                                     // reg
            0b101,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
    };
    constexpr addr_size expectedInstructionsLen = sizeof(expectedInstructions) / sizeof(asm8086::Instruction);

    assertContextDecodedAsExpected(ctx, expectedInstructions, expectedInstructionsLen);

    // Start encoding

    core::StrBuilder out;

    ctx.options = DEC_OP_IMMEDIATE_AS_SIGNED;
    {
        asm8086::encodeAsm8086(out, ctx);
        const char* expectedAsm =
            "bits 16\n"
            "\n"
            "mov cx, bx\n"
            "mov ch, ah\n"
            "mov dx, bx\n"
            "mov si, bx\n"
            "mov bx, di\n"
            "mov al, cl\n"
            "mov ch, ch\n"
            "mov bx, ax\n"
            "mov bx, si\n"
            "mov sp, di\n"
            "mov bp, ax\n";

        Assert(out.eq(expectedAsm), "Encoding failed.");
    }

    out.clear();
    ctx.options = DEC_OP_IMMEDIATE_AS_UNSIGNED;
    {
        asm8086::encodeAsm8086(out, ctx);
        const char* expectedAsm =
            "bits 16\n"
            "\n"
            "mov cx, bx\n"
            "mov ch, ah\n"
            "mov dx, bx\n"
            "mov si, bx\n"
            "mov bx, di\n"
            "mov al, cl\n"
            "mov ch, ch\n"
            "mov bx, ax\n"
            "mov bx, si\n"
            "mov sp, di\n"
            "mov bp, ax\n";

        Assert(out.eq(expectedAsm), "Encoding failed.");
    }

    out.clear();
    ctx.options = DEC_OP_IMMEDIATE_AS_HEX;
    {
        asm8086::encodeAsm8086(out, ctx);
        const char* expectedAsm =
            "bits 16\n"
            "\n"
            "mov cx, bx\n"
            "mov ch, ah\n"
            "mov dx, bx\n"
            "mov si, bx\n"
            "mov bx, di\n"
            "mov al, cl\n"
            "mov ch, ch\n"
            "mov bx, ax\n"
            "mov bx, si\n"
            "mov sp, di\n"
            "mov bp, ax\n";

        Assert(out.eq(expectedAsm), "Encoding failed.");
    }

    return 0;
}

i32 decodeComplicatedMoveInstructionsTest() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * ; Register-to-register
     * mov si, bx
     * mov dh, al
     *
     * ; 8-bit immediate-to-register
     * mov cx, 12
     * mov cx, -12
     *
     * ; 16-bit immediate-to-register
     * mov dx, 3948
     * mov dx, -3948
     *
     * ; 16-bit max immediate-to-register
     * mov ax, 32769 ; This one is technically invalid.
     *
     * ; Source address calculation
     * mov al, [bx + si]
     * mov bx, [bp + di]
     * mov dx, [bp]
     *
     * ; Source address calculation plus 8-bit displacement
     * mov ah, [bx + si + 4]
     *
     * ; Source address calculation plus 16-bit displacement
     * mov al, [bx + si + 4999]
     * mov al, [bx + si + 32769] ; This one is technically invalid.
     *
     * ; Dest address calculation
     * mov [bx + di], cx
     * mov [bp + si], cl
     * mov [bp], ch
     *
    */
    core::Arr<u8> binaryData;
    binaryData.append(0x89).append(0xde)
              .append(0x88).append(0xc6)
              .append(0xb9).append(0x0c).append(0x00)
              .append(0xb9).append(0xf4).append(0xff)
              .append(0xba).append(0x6c).append(0x0f)
              .append(0xba).append(0x94).append(0xf0)
              .append(0xb8).append(0x01).append(0x80)
              .append(0x8a).append(0x00)
              .append(0x8b).append(0x1b)
              .append(0x8b).append(0x56).append(0x00)
              .append(0x8a).append(0x60).append(0x04)
              .append(0x8a).append(0x80).append(0x87).append(0x13)
              .append(0x8a).append(0x80).append(0x01).append(0x80)
              .append(0x89).append(0x09)
              .append(0x88).append(0x0a)
              .append(0x88).append(0x6e).append(0x00);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    constexpr asm8086::Instruction expectedInstructions[] = {
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b011,                                     // reg
            0b110,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            0,                                         // w
            Mod::REGISTER_TO_REGISTER_NO_DISPLACEMENT, // mod
            0b000,                                     // reg
            0b110,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Register,               // operands
        },
        {
            Opcode::MOV_IMM_TO_REG,                    // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::NONE_SENTINEL,                        // mod
            0b001,                                     // reg
            0b001,                                     // rm
            {0, 0},                                    // disp
            {12, 0},                                   // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Register_Immediate,              // operands
        },
        {
            Opcode::MOV_IMM_TO_REG,                    // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::NONE_SENTINEL,                        // mod
            0b001,                                     // reg
            0b001,                                     // rm
            {0, 0},                                    // disp
            {0xF4, 0xFF},                              // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Register_Immediate,              // operands
        },
        {
            Opcode::MOV_IMM_TO_REG,                    // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::NONE_SENTINEL,                        // mod
            0b010,                                     // reg
            0b010,                                     // rm
            {0, 0},                                    // disp
            {0x6C, 0xF},                               // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Register_Immediate,              // operands
        },
        {
            Opcode::MOV_IMM_TO_REG,                    // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::NONE_SENTINEL,                        // mod
            0b010,                                     // reg
            0b010,                                     // rm
            {0, 0},                                    // disp
            {0x94, 0xF0},                              // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Register_Immediate,              // operands
        },
        {
            Opcode::MOV_IMM_TO_REG,                    // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::NONE_SENTINEL,                        // mod
            0b000,                                     // reg
            0b000,                                     // rm
            {0, 0},                                    // disp
            {0x01, 0x80},                              // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Register_Immediate,              // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            1,                                         // d
            0,                                         // s
            0,                                         // w
            Mod::MEMORY_NO_DISPLACEMENT,               // mod
            0b000,                                     // reg
            0b000,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Memory_Register,                 // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            1,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::MEMORY_NO_DISPLACEMENT,               // mod
            0b011,                                     // reg
            0b011,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Memory_Register,                 // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            1,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::MEMORY_8_BIT_DISPLACEMENT,            // mod
            0b010,                                     // reg
            0b110,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Memory_Register,                 // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            1,                                         // d
            0,                                         // s
            0,                                         // w
            Mod::MEMORY_8_BIT_DISPLACEMENT,            // mod
            0b100,                                     // reg
            0b000,                                     // rm
            {0x4, 0},                                  // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Memory_Register,                 // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            1,                                         // d
            0,                                         // s
            0,                                         // w
            Mod::MEMORY_16_BIT_DISPLACEMENT,           // mod
            0b000,                                     // reg
            0b000,                                     // rm
            {0x87, 0x13},                              // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            4,                                         // byteCount
            Operands::Memory_Register,                 // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            1,                                         // d
            0,                                         // s
            0,                                         // w
            Mod::MEMORY_16_BIT_DISPLACEMENT,           // mod
            0b000,                                     // reg
            0b000,                                     // rm
            {0x01, 0x80},                              // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            4,                                         // byteCount
            Operands::Memory_Register,                 // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::MEMORY_NO_DISPLACEMENT,               // mod
            0b001,                                     // reg
            0b001,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Memory,                 // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            0,                                         // w
            Mod::MEMORY_NO_DISPLACEMENT,               // mod
            0b001,                                     // reg
            0b010,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            2,                                         // byteCount
            Operands::Register_Memory,                 // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            0,                                         // w
            Mod::MEMORY_8_BIT_DISPLACEMENT,            // mod
            0b101,                                     // reg
            0b110,                                     // rm
            {0, 0},                                    // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Register_Memory,                 // operands
        },
    };
    constexpr addr_size expectedInstructionsLen = sizeof(expectedInstructions) / sizeof(asm8086::Instruction);

    assertContextDecodedAsExpected(ctx, expectedInstructions, expectedInstructionsLen);

    // Start encoding

    core::StrBuilder out;

    ctx.options = DEC_OP_IMMEDIATE_AS_SIGNED;
    {
        asm8086::encodeAsm8086(out, ctx);

        const char* expectedAsm =
            "bits 16\n"
            "\n"
            "mov si, bx\n"
            "mov dh, al\n"
            "mov cx, 12\n"
            "mov cx, -12\n"
            "mov dx, 3948\n"
            "mov dx, -3948\n"
            "mov ax, -32767\n"
            "mov al, [bx + si]\n"
            "mov bx, [bp + di]\n"
            "mov dx, [bp]\n"
            "mov ah, [bx + si + 4]\n"
            "mov al, [bx + si + 4999]\n"
            "mov al, [bx + si - 32767]\n"
            "mov [bx + di], cx\n"
            "mov [bp + si], cl\n"
            "mov [bp], ch\n";

        Assert(out.eq(expectedAsm), "Encoding failed.");
    }

    out.clear();
    ctx.options = DEC_OP_IMMEDIATE_AS_UNSIGNED;
    {
        asm8086::encodeAsm8086(out, ctx);

        const char* expectedAsm =
            "bits 16\n"
            "\n"
            "mov si, bx\n"
            "mov dh, al\n"
            "mov cx, 12\n"
            "mov cx, 65524\n"
            "mov dx, 3948\n"
            "mov dx, 61588\n"
            "mov ax, 32769\n"
            "mov al, [bx + si]\n"
            "mov bx, [bp + di]\n"
            "mov dx, [bp]\n"
            "mov ah, [bx + si + 4]\n"
            "mov al, [bx + si + 4999]\n"
            "mov al, [bx + si + 32769]\n"
            "mov [bx + di], cx\n"
            "mov [bp + si], cl\n"
            "mov [bp], ch\n";

        Assert(out.eq(expectedAsm), "Encoding failed.");
    }

    out.clear();
    ctx.options = DEC_OP_IMMEDIATE_AS_HEX;
    {
        asm8086::encodeAsm8086(out, ctx);

        const char* expectedAsm =
            "bits 16\n"
            "\n"
            "mov si, bx\n"
            "mov dh, al\n"
            "mov cx, 0x000C\n"
            "mov cx, 0xFFF4\n"
            "mov dx, 0x0F6C\n"
            "mov dx, 0xF094\n"
            "mov ax, 0x8001\n"
            "mov al, [bx + si]\n"
            "mov bx, [bp + di]\n"
            "mov dx, [bp]\n"
            "mov ah, [bx + si + 0x0004]\n"
            "mov al, [bx + si + 0x1387]\n"
            "mov al, [bx + si + 0x8001]\n"
            "mov [bx + di], cx\n"
            "mov [bp + si], cl\n"
            "mov [bp], ch\n";

        Assert(out.eq(expectedAsm), "Encoding failed.");
    }

    return 0;
}

i32 decodeChallengeMoveInstructonsTest() {
    /**
     * This binary data represents the following assembly code:
     *
     * bits 16
     *
     * ; Signed displacements
     * mov ax, [bx + di - 37]
     * mov [si - 300], cx
     * mov dx, [bx - 32]
     *
     * ; Explicit sizes
     * mov byte [bp + di], 7
     * mov word [di + 901], 347
     *
     * ; Direct address
     * mov bp, [5]
     * mov bx, [3458]
     *
     * ; Memory-to-accumulator test
     * mov ax, [2555]
     * mov ax, [16]
     *
     * ; Accumulator-to-memory test
     * mov [2554], ax
     * mov [15], ax
     *
    */
    core::Arr<u8> binaryData;
    binaryData.append(0x8b).append(0x41).append(0xdb)
              .append(0x89).append(0x8c).append(0xd4).append(0xfe)
              .append(0x8b).append(0x57).append(0xe0)
              .append(0xc6).append(0x03).append(0x07)
              .append(0xc7).append(0x85).append(0x85).append(0x03).append(0x5b).append(0x01)
              .append(0x8b).append(0x2e).append(0x05).append(0x00)
              .append(0x8b).append(0x1e).append(0x82).append(0x0d)
              .append(0xa1).append(0xfb).append(0x09)
              .append(0xa1).append(0x10).append(0x00)
              .append(0xa3).append(0xfa).append(0x09)
              .append(0xa3).append(0x0f).append(0x00);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    constexpr asm8086::Instruction expectedInstructions[] = {
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            1,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::MEMORY_8_BIT_DISPLACEMENT,            // mod
            0b000,                                     // reg
            0b001,                                     // rm
            {0xDB, 0},                                 // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Memory_Register,                 // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::MEMORY_16_BIT_DISPLACEMENT,           // mod
            0b001,                                     // reg
            0b100,                                     // rm
            {0xD4, 0xFE},                              // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            4,                                         // byteCount
            Operands::Register_Memory,                 // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            1,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::MEMORY_8_BIT_DISPLACEMENT,            // mod
            0b010,                                     // reg
            0b111,                                     // rm
            {0xE0, 0},                                 // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Memory_Register,                 // operands
        },
        {
            Opcode::MOV_IMM_TO_REG_OR_MEM,             // opcode
            0,                                         // d
            0,                                         // s
            0,                                         // w
            Mod::MEMORY_NO_DISPLACEMENT,               // mod
            0b000,                                     // reg
            0b011,                                     // rm
            {0, 0},                                    // disp
            {7, 0},                                    // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Memory_Immediate,                // operands
        },
        {
            Opcode::MOV_IMM_TO_REG_OR_MEM,             // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::MEMORY_16_BIT_DISPLACEMENT,           // mod
            0,                                         // reg
            0b101,                                     // rm
            {0x85, 0x03},                              // disp
            {0x5B, 0x01},                              // data
            InstType::MOV,                             // type
            6,                                         // byteCount
            Operands::Memory_Immediate,                // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            1,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::MEMORY_NO_DISPLACEMENT,               // mod
            0b101,                                     // reg
            0b110,                                     // rm
            {0x05, 0},                                 // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            4,                                         // byteCount
            Operands::Memory_Register,                 // operands
        },
        {
            Opcode::MOV_REG_OR_MEM_TO_OR_FROM_REG,     // opcode
            1,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::MEMORY_NO_DISPLACEMENT,               // mod
            0b011,                                     // reg
            0b110,                                     // rm
            {0x82, 0xD},                               // disp
            {0, 0},                                    // data
            InstType::MOV,                             // type
            4,                                         // byteCount
            Operands::Memory_Register,                 // operands
        },
        {
            Opcode::MOV_MEM_TO_ACC,                    // opcode
            0,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::NONE_SENTINEL,                        // mod
            0,                                         // reg
            0b000,                                     // rm
            {0, 0},                                    // disp
            {0xFB, 0x9},                               // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Memory_Accumulator,              // operands
        },
        {
            Opcode::MOV_MEM_TO_ACC,                    // opcode
            0,                                         // d
            0,                                         // s
            1,                                         //

            Mod::NONE_SENTINEL,                        // mod
            0,                                         // reg
            0b000,                                     // rm
            {0, 0},                                    // disp
            {0x10, 0},                                 // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Memory_Accumulator,              // operands
        },
        {
            Opcode::MOV_ACC_TO_MEM,                    // opcode
            1,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::NONE_SENTINEL,                        // mod
            0,                                         // reg
            0b000,                                     // rm
            {0, 0},                                    // disp
            {0xFA, 0x9},                               // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Accumulator_Memory,              // operands
        },
        {
            Opcode::MOV_ACC_TO_MEM,                    // opcode
            1,                                         // d
            0,                                         // s
            1,                                         // w
            Mod::NONE_SENTINEL,                        // mod
            0,                                         // reg
            0b000,                                     // rm
            {0, 0},                                    // disp
            {0xF, 0},                                  // data
            InstType::MOV,                             // type
            3,                                         // byteCount
            Operands::Accumulator_Memory,              // operands
        },
    };
    constexpr addr_size expectedInstructionsLen = sizeof(expectedInstructions) / sizeof(asm8086::Instruction);

    assertContextDecodedAsExpected(ctx, expectedInstructions, expectedInstructionsLen);

    // Start encoding

    core::StrBuilder out;

    ctx.options = DEC_OP_IMMEDIATE_AS_SIGNED;
    {
        asm8086::encodeAsm8086(out, ctx);

        const char* expectedAsm =
            "bits 16\n"
            "\n"
            "mov ax, [bx + di - 37]\n"
            "mov [si - 300], cx\n"
            "mov dx, [bx - 32]\n"
            "mov byte [bp + di], 7\n"
            "mov word [di + 901], 347\n"
            "mov bp, [5]\n"
            "mov bx, [3458]\n"
            "mov ax, [2555]\n"
            "mov ax, [16]\n"
            "mov [2554], ax\n"
            "mov [15], ax\n";

        Assert(out.eq(expectedAsm), "Encoding failed.");
    }

    out.clear();
    ctx.options = DEC_OP_IMMEDIATE_AS_UNSIGNED;
    {
        asm8086::encodeAsm8086(out, ctx);

        const char* expectedAsm =
            "bits 16\n"
            "\n"
            "mov ax, [bx + di + 219]\n"
            "mov [si + 65236], cx\n"
            "mov dx, [bx + 224]\n"
            "mov byte [bp + di], 7\n"
            "mov word [di + 901], 347\n"
            "mov bp, [5]\n"
            "mov bx, [3458]\n"
            "mov ax, [2555]\n"
            "mov ax, [16]\n"
            "mov [2554], ax\n"
            "mov [15], ax\n";

        Assert(out.eq(expectedAsm), "Encoding failed.");
    }

    out.clear();
    ctx.options = DEC_OP_IMMEDIATE_AS_HEX;
    {
        asm8086::encodeAsm8086(out, ctx);

        const char* expectedAsm =
            "bits 16\n"
            "\n"
            "mov ax, [bx + di + 0x00DB]\n"
            "mov [si + 0xFED4], cx\n"
            "mov dx, [bx + 0x00E0]\n"
            "mov byte [bp + di], 0x0007\n"
            "mov word [di + 0x0385], 0x015B\n"
            "mov bp, [0x0005]\n"
            "mov bx, [0x0D82]\n"
            "mov ax, [0x09FB]\n"
            "mov ax, [0x0010]\n"
            "mov [0x09FA], ax\n"
            "mov [0x000F], ax\n";

        Assert(out.eq(expectedAsm), "Encoding failed.");
    }

    return 0;
}

i32 decodeAddSubCmpJumpInstructionsTest() {

    // This is example is located in data/05_add_sub_cmp_jump.asm
    core::Arr<u8> binaryData;

    binaryData
        .append(0x03).append(0x18).append(0x03).append(0x5e).append(0x00).append(0x83)
        .append(0xc6).append(0x02).append(0x83).append(0xc5).append(0x02).append(0x83)
        .append(0xc1).append(0x08).append(0x03).append(0x5e).append(0x00).append(0x03)
        .append(0x4f).append(0x02).append(0x02).append(0x7a).append(0x04).append(0x03)
        .append(0x7b).append(0x06).append(0x01).append(0x18).append(0x01).append(0x5e)
        .append(0x00).append(0x01).append(0x5e).append(0x00).append(0x01).append(0x4f)
        .append(0x02).append(0x00).append(0x7a).append(0x04).append(0x01).append(0x7b)
        .append(0x06).append(0x80).append(0x07).append(0x22).append(0x83).append(0x82)
        .append(0xe8).append(0x03).append(0x1d).append(0x03).append(0x46).append(0x00)
        .append(0x02).append(0x00).append(0x01).append(0xd8).append(0x00).append(0xe0)
        .append(0x05).append(0xe8).append(0x03).append(0x04).append(0xe2).append(0x04)
        .append(0x09).append(0x2b).append(0x18).append(0x2b).append(0x5e).append(0x00)
        .append(0x83).append(0xee).append(0x02).append(0x83).append(0xed).append(0x02)
        .append(0x83).append(0xe9).append(0x08).append(0x2b).append(0x5e).append(0x00)
        .append(0x2b).append(0x4f).append(0x02).append(0x2a).append(0x7a).append(0x04)
        .append(0x2b).append(0x7b).append(0x06).append(0x29).append(0x18).append(0x29)
        .append(0x5e).append(0x00).append(0x29).append(0x5e).append(0x00).append(0x29)
        .append(0x4f).append(0x02).append(0x28).append(0x7a).append(0x04).append(0x29)
        .append(0x7b).append(0x06).append(0x80).append(0x2f).append(0x22).append(0x83)
        .append(0x29).append(0x1d).append(0x2b).append(0x46).append(0x00).append(0x2a)
        .append(0x00).append(0x29).append(0xd8).append(0x28).append(0xe0).append(0x2d)
        .append(0xe8).append(0x03).append(0x2c).append(0xe2).append(0x2c).append(0x09)
        .append(0x3b).append(0x18).append(0x3b).append(0x5e).append(0x00).append(0x83)
        .append(0xfe).append(0x02).append(0x83).append(0xfd).append(0x02).append(0x83)
        .append(0xf9).append(0x08).append(0x3b).append(0x5e).append(0x00).append(0x3b)
        .append(0x4f).append(0x02).append(0x3a).append(0x7a).append(0x04).append(0x3b)
        .append(0x7b).append(0x06).append(0x39).append(0x18).append(0x39).append(0x5e)
        .append(0x00).append(0x39).append(0x5e).append(0x00).append(0x39).append(0x4f)
        .append(0x02).append(0x38).append(0x7a).append(0x04).append(0x39).append(0x7b)
        .append(0x06).append(0x80).append(0x3f).append(0x22).append(0x83).append(0x3e)
        .append(0xe2).append(0x12).append(0x1d).append(0x3b).append(0x46).append(0x00)
        .append(0x3a).append(0x00).append(0x39).append(0xd8).append(0x38).append(0xe0)
        .append(0x3d).append(0xe8).append(0x03).append(0x3c).append(0xe2).append(0x3c)
        .append(0x09).append(0x75).append(0x06).append(0x75).append(0x0b).append(0x75)
        .append(0x26).append(0x75).append(0x2f).append(0x83).append(0x3e).append(0xe2)
        .append(0x12).append(0x1d).append(0x75).append(0xf9).append(0x38).append(0x7a)
        .append(0x04).append(0x75).append(0xf4).append(0x38).append(0x7a).append(0x04)
        .append(0x39).append(0xd8).append(0x38).append(0xe0).append(0x75).append(0xf2)
        .append(0x83).append(0x3e).append(0xe2).append(0x12).append(0x1d).append(0x75)
        .append(0x08).append(0x38).append(0x7a).append(0x04).append(0x75).append(0xe6)
        .append(0x38).append(0x7a).append(0x04).append(0x75).append(0xfe).append(0x83)
        .append(0x3e).append(0xe2).append(0x12).append(0x1d).append(0x75).append(0xda)
        .append(0x75).append(0xd1).append(0x75).append(0x00).append(0x74).append(0x00)
        .append(0x7c).append(0xfe).append(0x7e).append(0xfc).append(0x72).append(0xfa)
        .append(0x76).append(0xf8).append(0x7a).append(0xf6).append(0x70).append(0xf4)
        .append(0x78).append(0xf2).append(0x75).append(0xf0).append(0x7d).append(0xee)
        .append(0x7f).append(0xec).append(0x73).append(0xea).append(0x77).append(0xe8)
        .append(0x7b).append(0xe6).append(0x71).append(0xe4).append(0x79).append(0xe2)
        .append(0xe2).append(0xe0).append(0xe1).append(0xde).append(0xe0).append(0xdc)
        .append(0xe3).append(0xda);

    DecodingContext ctx;
    decodeAsm8086(binaryData, ctx);

    // Start encoding

    core::StrBuilder out;
    {
        asm8086::encodeAsm8086(out, ctx);

        const char* expectedAsm =
        "bits 16\n"
        "\n"
        "add bx, [bx + si]\n"
        "add bx, [bp]\n"
        "add si, 2\n"
        "add bp, 2\n"
        "add cx, 8\n"
        "add bx, [bp]\n"
        "add cx, [bx + 2]\n"
        "add bh, [bp + si + 4]\n"
        "add di, [bp + di + 6]\n"
        "add [bx + si], bx\n"
        "add [bp], bx\n"
        "add [bp], bx\n"
        "add [bx + 2], cx\n"
        "add [bp + si + 4], bh\n"
        "add [bp + di + 6], di\n"
        "add byte [bx], 34\n"
        "add word [bp + si + 1000], 29\n"
        "add ax, [bp]\n"
        "add al, [bx + si]\n"
        "add ax, bx\n"
        "add al, ah\n"
        "add ax, 1000\n"
        "add al, -30\n"
        "add al, 9\n"
        "sub bx, [bx + si]\n"
        "sub bx, [bp]\n"
        "sub si, 2\n"
        "sub bp, 2\n"
        "sub cx, 8\n"
        "sub bx, [bp]\n"
        "sub cx, [bx + 2]\n"
        "sub bh, [bp + si + 4]\n"
        "sub di, [bp + di + 6]\n"
        "sub [bx + si], bx\n"
        "sub [bp], bx\n"
        "sub [bp], bx\n"
        "sub [bx + 2], cx\n"
        "sub [bp + si + 4], bh\n"
        "sub [bp + di + 6], di\n"
        "sub byte [bx], 34\n"
        "sub word [bx + di], 29\n"
        "sub ax, [bp]\n"
        "sub al, [bx + si]\n"
        "sub ax, bx\n"
        "sub al, ah\n"
        "sub ax, 1000\n"
        "sub al, -30\n"
        "sub al, 9\n"
        "cmp bx, [bx + si]\n"
        "cmp bx, [bp]\n"
        "cmp si, 2\n"
        "cmp bp, 2\n"
        "cmp cx, 8\n"
        "cmp bx, [bp]\n"
        "cmp cx, [bx + 2]\n"
        "cmp bh, [bp + si + 4]\n"
        "cmp di, [bp + di + 6]\n"
        "cmp [bx + si], bx\n"
        "cmp [bp], bx\n"
        "cmp [bp], bx\n"
        "cmp [bx + 2], cx\n"
        "cmp [bp + si + 4], bh\n"
        "cmp [bp + di + 6], di\n"
        "cmp byte [bx], 34\n"
        "cmp word [4834], 29\n"
        "cmp ax, [bp]\n"
        "cmp al, [bx + si]\n"
        "cmp ax, bx\n"
        "cmp al, ah\n"
        "cmp ax, 1000\n"
        "cmp al, -30\n"
        "cmp al, 9\n"
        "jne label_0\n"
        "jne label_1\n"
        "jne label_2\n"
        "jne label_3\n"
        "label_0:\n"
        "cmp word [4834], 29\n"
        "jne label_0\n"
        "label_1:\n"
        "cmp [bp + si + 4], bh\n"
        "jne label_0\n"
        "cmp [bp + si + 4], bh\n"
        "cmp ax, bx\n"
        "cmp al, ah\n"
        "jne label_1\n"
        "cmp word [4834], 29\n"
        "jne label_2\n"
        "cmp [bp + si + 4], bh\n"
        "jne label_1\n"
        "cmp [bp + si + 4], bh\n"
        "label_2:\n"
        "jne label_2\n"
        "cmp word [4834], 29\n"
        "jne label_1\n"
        "jne label_0\n"
        "label_3:\n"
        "jne label_4\n"
        "label_4:\n"
        "je label_5\n"
        "label_5:\n"
        "jl label_5\n"
        "jle label_5\n"
        "jb label_5\n"
        "jbe label_5\n"
        "jp label_5\n"
        "jo label_5\n"
        "js label_5\n"
        "jne label_5\n"
        "jnl label_5\n"
        "jnle label_5\n"
        "jnb label_5\n"
        "jnbe label_5\n"
        "jnp label_5\n"
        "jno label_5\n"
        "jns label_5\n"
        "loop label_5\n"
        "loope label_5\n"
        "loopne label_5\n"
        "jcxz label_5\n";

        Assert(out.eq(expectedAsm), "Encoding failed.");
    }

    return 0;
}

i32 runDecoderTestsSuite() {
    RunTest(decodeOneInstructionsTest);
    RunTest(decodeMultipleInstructionsTest);
    RunTest(decodeComplicatedMoveInstructionsTest);
    RunTest(decodeChallengeMoveInstructonsTest);
    RunTest(decodeAddSubCmpJumpInstructionsTest);

    return 0;
}
