#!/usr/bin/python
# -*- coding: UTF-8 -*
import re
import csv
from enum import Enum, unique


@unique
class ParseState(Enum):
    InstructHeader = 0,
    TipsAndPseudoCode = 1,
    SR = 2,
    Separator = 3,
    OpCode = 4


# 初始化状态
g_State = ParseState.InstructHeader

# 临时解析状态
g_Instruction = None
g_Description = None
g_Tips = None
g_PseudoCode = None

# csv需要的属性记录
g_MnemonicId = 0
g_AddressingId = 0
g_AddressingIdMap = {}

with open("mnemonic.csv", 'w', newline="") as fMnemonic, \
        open("addressing.csv", "w", newline="") as fAddressing, \
        open("opcodes.csv", "w", newline="") as fOpCodes:

    # 写csv头
    csv_mnemonic = csv.DictWriter(fMnemonic, ["id", "name", "description", "tips", "pseudo", "N", "Z", "C", "I",
                                              "D", "V"])
    csv_mnemonic.writeheader()

    csv_addressing = csv.DictWriter(fAddressing, ["id", "addressing_mode"])
    csv_addressing.writeheader()

    csv_opcodes = csv.DictWriter(fOpCodes, ["code", "mnemonic_id", "addressing_id", "bytes", "cycles", "assembler"])
    csv_opcodes.writeheader()

    # 开始解析
    for line in open("6502.txt"):
        if len(line.strip()) == 0:
            if g_State is ParseState.OpCode:
                g_State = ParseState.InstructHeader
                g_Instruction = None
                g_Description = None
                g_Tips = None
                g_PseudoCode = None
            continue

        # 解析指令头
        if g_State is ParseState.InstructHeader:
            matchObj = re.match(r'^([A-Z]{3})\s*(.*)', line, re.M | re.I)
            if matchObj:
                g_MnemonicId = g_MnemonicId + 1
                g_Instruction = matchObj.group(1)
                g_Description = matchObj.group(2)
                g_State = ParseState.TipsAndPseudoCode

        # 解析详细说明
        elif g_State is ParseState.TipsAndPseudoCode:
            if -1 == line.rfind("N Z C I D V"):
                matchObj = re.match(r'.*', line, re.M | re.I)
                if matchObj:
                    if g_Tips is not None:
                        g_Tips += "\n"
                    else:
                        g_Tips = ""
                    g_Tips += matchObj.group(0).strip()
            else:
                g_PseudoCode = line.replace("N Z C I D V", "").strip()
                g_State = ParseState.SR

        # 解析状态寄存器
        elif g_State is ParseState.SR:
            matchObj = re.match(r'\s*([+-01SM67]{1,2}) ([+-01SM67]{1,2}) ([+-01SM67]{1,2}) ([+-01SM67]{1,'
                                r'2}) ([+-01SM67]{1,2}) ([+-01SM67]{1,2})', line, re.M | re.I)
            if matchObj:
                # 解析完SR, 可以写助记符csv的行了
                csv_mnemonic.writerow({
                    csv_mnemonic.fieldnames[0]: g_MnemonicId,
                    csv_mnemonic.fieldnames[1]: g_Instruction,
                    csv_mnemonic.fieldnames[2]: g_Description,
                    csv_mnemonic.fieldnames[3]: g_Tips,
                    csv_mnemonic.fieldnames[4]: g_PseudoCode,
                    csv_mnemonic.fieldnames[5]: matchObj.group(1).strip(),
                    csv_mnemonic.fieldnames[6]: matchObj.group(2).strip(),
                    csv_mnemonic.fieldnames[7]: matchObj.group(3).strip(),
                    csv_mnemonic.fieldnames[8]: matchObj.group(4).strip(),
                    csv_mnemonic.fieldnames[9]: matchObj.group(5).strip(),
                    csv_mnemonic.fieldnames[10]: matchObj.group(6).strip()
                })

                g_State = ParseState.Separator

        # 检测分隔线
        elif g_State is ParseState.Separator:
            matchObj = re.match(r'\s*-{44}', line, re.M | re.I)
            if matchObj:
                g_State = ParseState.OpCode

        # 解析OpCode
        elif g_State is ParseState.OpCode:

            matchObj = re.match(r'\s*(\S*)\s*(\S* \S*)\s*([0-9A-Fa-f]{2})\s*(\S*)\s*(\S*)', line,
                                re.M | re.I)

            if matchObj:
                tmpAddressingMode = matchObj.group(1).strip()
                addressingIdUsed = 0
                if tmpAddressingMode in g_AddressingIdMap:
                    addressingIdUsed = g_AddressingIdMap[tmpAddressingMode]
                else:
                    g_AddressingId = g_AddressingId + 1
                    csv_addressing.writerow({csv_addressing.fieldnames[0]: g_AddressingId,
                                             csv_addressing.fieldnames[1]: tmpAddressingMode})
                    g_AddressingIdMap[tmpAddressingMode] = g_AddressingId
                    addressingIdUsed = g_AddressingId

                csv_opcodes.writerow({
                    csv_opcodes.fieldnames[0]: "0x%s" % matchObj.group(3).strip(),
                    csv_opcodes.fieldnames[1]: g_MnemonicId,
                    csv_opcodes.fieldnames[2]: addressingIdUsed,
                    csv_opcodes.fieldnames[3]: matchObj.group(4).strip(),
                    csv_opcodes.fieldnames[4]: matchObj.group(5).strip(),
                    csv_opcodes.fieldnames[5]: matchObj.group(2).strip()
                })
