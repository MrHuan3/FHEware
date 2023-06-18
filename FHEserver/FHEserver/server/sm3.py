# -*- encoding: utf-8 -*-
# File Name: sm3
# Author: 瓛
# @Time: 2023/1/12 09:53 1月

class SM3:

    def __init__(self):
        self.iv = 0x7380166f4914b2b9172442d7da8a0600a96f30bc163138aae38dee4db0fb0e4e
        self.MAX = 2 ** 32

    def str2bin(self, msg):
        l = len(msg)
        s_dec = 0
        for m in msg:
            s_dec = s_dec << 8
            s_dec += ord(m)
        msg_bin = bin(s_dec)[2:].zfill(l * 8)
        return msg_bin

    def int2bin(self, a, k):
        return bin(a)[2:].zfill(k)

    def int2hex(self, a, k):
        return hex(a)[2:].zfill(k)

    def bin2hex(self, a, k):
        return hex(int(a, 2))[2:].zfill(k)

    def msg_fill(self, msg_bin):
        l = len(msg_bin)
        k = 448 - (l + 1) % 512
        if k < 0:
            k += 512
        l_bin = self.int2bin(l, 64)
        msg_filled = msg_bin + '1' + '0' * k + l_bin
        return msg_filled

    def iteration_func(self, msg):
        n = len(msg) // 512
        b = []
        for i in range(n):
            b.append(msg[512 * i:512 * (i + 1)])
        v = [self.int2bin(self.iv, 256)]
        for i in range(n):
            v.append(self.cf(v[i], b[i]))
        return self.bin2hex(v[n], 64)
 
 
    def msg_extension(self, bi):
        w = []
        for j in range(16):
            w.append(int(bi[j * 32:(j + 1) * 32], 2))
        for j in range(16, 68):
            w_j = self.p1(w[j - 16] ^ w[j - 9] ^ self.rotate_left(w[j - 3], 15)) ^ self.rotate_left(w[j - 13], 7) ^ w[j - 6]
            w.append(w_j)
        w1 = []
        for j in range(64):
            w1.append(w[j] ^ w[j + 4])
        return w, w1
 
 
    def cf(self, vi, bi):
        w, w1 = self.msg_extension(bi)
        t = []
        for i in range(8):
            t.append(int(vi[i * 32:(i + 1) * 32], 2))
        a, b, c, d, e, f, g, h = t
        for j in range(64):
            ss1 = self.rotate_left((self.rotate_left(a, 12) + e + self.rotate_left(self.t_j(j), j)) % self.MAX, 7)
            ss2 = ss1 ^ self.rotate_left(a, 12)
            tt1 = (self.ff(a, b, c, j) + d + ss2 + w1[j]) % self.MAX
            tt2 = (self.gg(e, f, g, j) + h + ss1 + w[j]) % self.MAX
            d = c
            c = self.rotate_left(b, 9)
            b = a
            a = tt1
            h = g
            g = self.rotate_left(f, 19)
            f = e
            e = self.p0(tt2)
        vi_1 = self.int2bin(a, 32) + self.int2bin(b, 32) + self.int2bin(c, 32) + self.int2bin(d, 32) + self.int2bin(e, 32) + self.int2bin(f, 32) + self.int2bin(g, 32) + self.int2bin(h, 32)
        vi_1 = int(vi_1, 2) ^ int(vi, 2)
        return self.int2bin(vi_1, 256)

    def rotate_left(self, a, k):
        k = k % 32
        return ((a << k) & 0xFFFFFFFF) | ((a & 0xFFFFFFFF) >> (32 - k))

    def p0(self, x):
        return x ^ self.rotate_left(x, 9) ^ self.rotate_left(x, 17)

    def p1(self, x):
        return x ^ self.rotate_left(x, 15) ^ self.rotate_left(x, 23)

    def t_j(self, j):
        if j <= 15:
            return 0x79cc4519
        else:
            return 0x7a879d8a

    def ff(self, x, y, z, j):
        if j <= 15:
            return x ^ y ^ z
        else:
            return (x & y) | (x & z) | (y & z)

    def gg(self, x, y, z, j):
        if j <= 15:
            return x ^ y ^ z
        else:
            return (x & y) | ((x ^ 0xFFFFFFFF) & z)

    def generate_ID(self, msg):
        s_bin = self.str2bin(msg)
        s_fill = self.msg_fill(s_bin)
        s_sm3 = self.iteration_func(s_fill)
        return s_sm3.upper()[:10]
