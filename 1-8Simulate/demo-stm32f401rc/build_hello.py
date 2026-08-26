"""汇编 STM32F401RCT6 最小 UART 程序，向 USART2 输出固定字符串。"""

from pathlib import Path

from keystone import KS_ARCH_ARM, KS_MODE_THUMB, Ks

DEMO = Path(__file__).resolve().parent

# USART2_CR1 开启 UE/TE 后，向 DR 写字符。Renode 的 UART 模型通常不要求完整 RCC 初始化。
ASM = r"""
    ldr r0, =0x4000440C
    ldr r1, =0x2008
    str r1, [r0]
    ldr r0, =0x40004404
    adr r2, msg
send:
    ldrb r1, [r2]
    cmp r1, #0
    beq hang
    str r1, [r0]
    adds r2, #1
    b send
hang:
    b hang
    .align 2
msg:
    .asciz "F401RCT6 OK"
"""


def main() -> None:
    ks = Ks(KS_ARCH_ARM, KS_MODE_THUMB)
    encoding, _ = ks.asm(ASM)
    if encoding is None:
        raise SystemExit("汇编失败")
    code = bytes(encoding)
    # 向量表：MSP 指向 64KB SRAM 顶端；复位地址为 Thumb
    reset = 0x08000008 | 1
    vector = (0x20010000).to_bytes(4, "little") + reset.to_bytes(4, "little")
    (DEMO / "hello.bin").write_bytes(vector + code)
    print(f"wrote {DEMO / 'hello.bin'} ({8 + len(code)} bytes)")


if __name__ == "__main__":
    main()
