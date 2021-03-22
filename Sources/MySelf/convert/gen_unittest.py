#!/usr/bin/python
# -*- coding: UTF-8 -*
import csv


# > 字典封装类
class DictWrap(object):
    def __init__(self, raw_dict):
        for key, value in raw_dict.items():
            setattr(self, key, value)


def get_register_check_string(reg_name, attribute):
    statement = ""
    if attribute == "-":
        statement = "    ASSERT_EQ(OLD_SR.%s, m_pCPU->SR.%s);\n" % (reg_name, reg_name)
    elif attribute == "0" or attribute == "1":
        statement = "    ASSERT_EQ(m_pCPU->SR.%s, %s);\n" % (reg_name, attribute)
    return statement


def get_cycle_check_string(cycles_str):
    real_cycles = int(cycles_str.replace("*", ""))
    star_count = cycles_str.count("*")
    if star_count == 0:
        return "ASSERT_EQ(m_pCPU->m_nCyclesLeft, %du);\n" % real_cycles
    elif star_count == 1:
        return "ASSERT_EQ(m_pCPU->m_nCyclesLeft, %du + (int)m_pCPU->m_bCross);" % real_cycles
    elif star_count == 2:
        return "ASSERT_EQ(m_pCPU->m_nCyclesLeft, %du + (int)m_pCPU->m_bCross);" % (real_cycles + 1)
    else:
        raise Exception("无效的配置!", 1)


# 数据
g_AddressingMap = {}
g_MnemonicMap = {}
g_OpcodesMap = {}

# 预读 addressing.csv
with open("addressing.csv", "r") as file:
    csv_reader = csv.reader(file, delimiter=',')
    # 跳过 header
    next(csv_reader)

    # 读内容
    for row in csv_reader:
        g_AddressingMap[row[0]] = str(row[1]).replace("(", "") \
            .replace(",", "") \
            .replace(")", "")

# 预读 mnemonic.csv
with open("mnemonic.csv", "r") as file:
    csv_reader = csv.reader(file, delimiter=",")
    # 跳过 header
    fields = next(csv_reader)

    # 解析内容
    for row in csv_reader:
        g_MnemonicMap[row[0]] = DictWrap(dict(zip(fields, row)))

# 开始解析 opcodes.csv
with open("opcodes.csv", "r") as file:
    csv_reader = csv.reader(file, delimiter=",")
    # 跳过 header
    fields = next(csv_reader)

    # 解析内容
    for row in csv_reader:
        g_OpcodesMap[row[0]] = DictWrap(dict(zip(fields, row)))

# 开始生成相关代码
with open("test.cpp", "w") as file:
    for tmpIndex in range(0, 256):
        tmpKey = "0x%02X" % tmpIndex

        # 遍历生成单元测试代码
        if tmpKey in g_OpcodesMap:
            v = g_OpcodesMap.get(tmpKey)
            mnemonicItem = g_MnemonicMap.get(v.mnemonic_id)
            addressingItem = g_AddressingMap.get(v.addressing_id)

            test_name = "%s_%s" % (mnemonicItem.name, addressingItem)

            # 写开头注释
            file.write("// %s %s\n" % (tmpKey, test_name))

            # 写单元测试声明
            file.write("TEST_F(CPU_Opcode_Fixture, %s)\n" % test_name)
            file.write("{\n")
            file.write("    // %s bytes, %s cycles\n" % (v.bytes, v.cycles))
            file.write("    // ( %-2s %-2s %-2s %-2s %-2s %-2s)\n" % ("N", "Z", "C", "I", "D", "V"))
            file.write("    //   %-2s %-2s %-2s %-2s %-2s %-2s\n" % (mnemonicItem.N, mnemonicItem.Z,
                                                                     mnemonicItem.C, mnemonicItem.I,
                                                                     mnemonicItem.D, mnemonicItem.V))
            file.write("    DEF_BACKUP;\n")
            file.write("    m_pMapper->SetFakeData(\"\\x%02X%s\", %s);\n" % (tmpIndex,
                                                                             "\\x00" * (int(v.bytes)-1),
                                                                             v.bytes))
            file.write("    m_pCPU->OnTick();\n")
            file.write("    %s\n" % get_cycle_check_string(v.cycles))

            # 状态寄存器
            file.write("    // Check SR\n")
            file.write("%s" % get_register_check_string("N", mnemonicItem.N))
            file.write("%s" % get_register_check_string("Z", mnemonicItem.Z))
            file.write("%s" % get_register_check_string("C", mnemonicItem.C))
            file.write("%s" % get_register_check_string("I", mnemonicItem.I))
            file.write("%s" % get_register_check_string("D", mnemonicItem.D))
            file.write("%s" % get_register_check_string("V", mnemonicItem.V))

            file.write("}\n")
        else:
            # 未实现的指令
            # 写开头注释
            file.write("// %s\n" % tmpKey)
            file.write("TEST_F(CPU_Opcode_Fixture, UNKNOWN_%02X)\n" % tmpIndex)
            file.write("{\n")
            file.write("}\n")

        file.write("\n")
