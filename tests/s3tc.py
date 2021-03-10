import struct
import math
import operator
from functools import reduce
from color import Color


def bit_slice(value, size, count):
    mask = (2 ** size) - 1
    return [(value >> offset) & mask for offset in range(0, size * count, size)]


def bit_merge(values, size):
    offsets = range(0, len(values) * size, size)
    return reduce(operator.__or__, map(operator.lshift, values, offsets))


def triple_slice(triplet):
    values = bit_slice(bit_merge(triplet, 8), 3, 8)
    return [values[0:4], values[4:8]]


def triple_merge(rows):
    values = rows[0] + rows[1]
    return bit_slice(bit_merge(values, 3), 8, 3)


class BC1Block:
    size = 8

    def __init__(self):
        self.color0 = Color()
        self.color1 = Color()
        self.selectors = [[0] * 4] * 4

    def __repr__(self):
        return repr(self.__dict__)

    def __str__(self):
        return f'color0: {str(self.color0)} color1: {str(self.color1)}, indices:{self.selectors}'

    @staticmethod
    def frombytes(data):
        block = struct.unpack_from('<2H4B', data)
        result = BC1Block()

        result.color0 = Color.from_565(block[0])
        result.color1 = Color.from_565(block[1])
        result.selectors = [bit_slice(row, 2, 4) for row in block[2:6]]
        return result

    def tobytes(self):
        return struct.pack('<2H4B',
                           self.color0.to_565(), self.color1.to_565(),
                           *(bit_merge(row, 2) for row in self.selectors))

    def is_3color(self):
        return self.color0.to_565() <= self.color1.to_565()

    def is_3color_black(self):
        return self.is_3color() and any(3 in row for row in self.selectors)


class BC4Block:
    size = 8

    def __init__(self):
        self.alpha0 = 1
        self.alpha1 = 1
        self.selectors = [[0] * 4] * 4

    def __repr__(self):
        return repr(self.__dict__)

    @staticmethod
    def frombytes(data):
        block = struct.unpack_from('<2B6B', data)
        result = BC4Block()

        result.alpha0 = block[0] / 0xFF
        result.alpha1 = block[1] / 0xFF
        result.selectors = triple_slice(block[2:5]) + triple_slice(block[5:8])
        return result

    def tobytes(self):
        return struct.pack('<2B6B',
                           int(self.alpha0 * 0xFF), int(self.alpha1 * 0xFF),
                           *triple_merge(self.selectors[0:2]),
                           *triple_merge(self.selectors[2:4]))
