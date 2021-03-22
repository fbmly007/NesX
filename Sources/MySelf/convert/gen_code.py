#!/usr/bin/python
# -*- coding: UTF-8 -*
import csv


# > 字典封装类
class DictWrap(object):
    def __init__(self, raw_dict):
        for key, value in raw_dict.items():
            setattr(self, key, value)


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
with open("test.c", "w") as file:
    # 写类型定义
    file.write("typedef unsigned char Byte;\n")
    file.write("typedef unsigned short Word;")

    file.write("\n\n")

    index = 0

    # 写助记符数组
    file.write("static Byte *s_Mnemonic[] =\n")
    file.write("{")
    for k in g_MnemonicMap.keys():
        v = g_MnemonicMap.get(k)
        if index % 8 == 0:
            file.write("\n    ")
        file.write("\"")
        file.write(v.name)
        file.write("\",")
        index = index + 1
    file.write("\n};\n")

    file.write("\n\n")

    # 写寻址方式
    file.write("enum addressingMode\n")
    file.write("{\n")
    file.write("    invalid \t= 0,\n")
    for k, v in g_AddressingMap.items():
        file.write("    ")
        file.write("%-12s" % v)
        file.write("= ")
        file.write(k)
        file.write(",\n")
    file.write("};")

    file.write("\n\n")

    # 写 Opcodes 结构体
    # code,mnemonic_id,addressing_id,[cycles]
    file.write("struct _opcode\n")
    file.write("{\n")
    file.write("    int code;\n")
    file.write("    int mnId;\n")
    file.write("    int addressingId;\n")
    file.write("    int reserved;\n")
    file.write("};\n")
    file.write("typedef struct _opcode opcode;")

    file.write("\n\n")

    # 写 Opcodes 数组
    file.write("static opcode s_Opcodes[256] = \n")
    file.write("{")
    for tmpIndex in range(0, 0x100):
        tmpKey = "0x%02X" % tmpIndex

        if tmpIndex % 8 == 0:
            file.write("\n    ")

        file.write("{")
        if tmpKey in g_OpcodesMap:
            v = g_OpcodesMap.get(tmpKey)
            file.write(v.code)
            file.write(",")
            file.write("%2s" % v.mnemonic_id)
            file.write(",")
            file.write("%2s" % v.addressing_id)
            file.write(", 0")
        else:
            file.write("%s, 0, 0, 0" % tmpKey)
        file.write("},\t")

    file.write("\n};\n")
