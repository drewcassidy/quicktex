class Color:
    def __init__(self, r=0, g=0, b=0, a=0xFF):
        self.r = r
        self.g = g
        self.b = b
        self.a = a

    def __add__(self, a):
        return Color(self.r + a.r, self.g + a.g, self.b + a.b, self.a + a.a)

    def __mul__(self, c):
        return Color(self.r * c, self.g * c, self.b * c, self.a * c)

    def __rmul__(self, c):
        return Color(self.r * c, self.g * c, self.b * c, self.a * c)

    def __iter__(self):
        return iter([self.r, self.g, self.b, self.a])

    def __repr__(self):
        return f'r: {self.r} g: {self.g} b: {self.b} a: {self.a}'

    def __str__(self):
        return self.to_hex()

    def error(self, other):
        assert isinstance(other, Color)
        return ((self.r - other.r) ** 2) + ((self.g - other.g) ** 2) + ((self.b - other.b) ** 2)

    @classmethod
    def from_565(cls, int_value):
        r = float((int_value & 0xF800) >> 11) / 0x1F
        g = float((int_value & 0x07E0) >> 5) / 0x3F
        b = float(int_value & 0x001F) / 0x1F

        return cls(r, g, b)

    def to_565(self):
        r = int(self.r * 0x1F)
        g = int(self.g * 0x3F)
        b = int(self.b * 0x1F)

        return (r << 11) | (g << 5) | b

    @classmethod
    def from_rgb24(cls, int_value):
        r = float((int_value & 0xFF0000) >> 16) / 0xFF
        g = float((int_value & 0x00FF00) >> 8) / 0xFF
        b = float(int_value & 0x0000FF) / 0xFF

        return cls(r, g, b)

    def to_rgb24(self):
        r = int(self.r * 0xFF)
        g = int(self.g * 0xFF)
        b = int(self.b * 0xFF)

        return (r << 16) | (g << 8) | b

    @classmethod
    def from_rgba32(cls, int_value):
        r = float((int_value & 0xFF000000) >> 24) / 0xFF
        g = float((int_value & 0x00FF0000) >> 16) / 0xFF
        b = float((int_value & 0x0000FF00) >> 8) / 0xFF
        a = float(int_value & 0x000000FF) / 0xFF

        return cls(r, g, b, a)

    def to_rgba32(self):
        r = int(self.r * 0xFF)
        g = int(self.g * 0xFF)
        b = int(self.b * 0xFF)
        a = int(self.a * 0xFF)

        return (r << 24) | (g << 16) | (b << 8) | a

    def to_hex(self):
        if self.a < 1:
            return hex(self.to_rgba32())
        else:
            return hex(self.to_rgb24())
