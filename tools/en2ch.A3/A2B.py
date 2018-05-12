#!/usr/bin/python
# coding:utf-8
# Author:  summer rain(xiatian@ict.ac.cn)

"""
Purpose:
windos£º python A2B.py file1 file2 file3....
linux: ./A2B.py file1 file2 file3...
"""

import sys

class ET:
    def SubStr(self, lt, del_self = False):
        """
            return all its substring.
            lt: is a string or a list.
            del_self: if True not return itself.
        """
        size = len(lt)
        r = []
        for lent in xrange(1, size + 1):
            for i in xrange(size):            
                if i + lent > size:
                    break
                r.append(lt[i: i + lent])

        return r[: -1] if del_self else r

    def IsChineseChar(self, uchar):
        """check a unicode char is chinese"""
        if uchar >= u'\u4e00' and uchar <= u'\u9fa5':
            return True
        else:
            return False

    def CharB2Q(self, uchar):
        """char of B2Q"""
        inside_code = ord(uchar)
        if inside_code < 0x0020 or inside_code > 0x7e:  
            return uchar
        if inside_code == 0x0020:                      
            inside_code=0x3000
        else:
            inside_code+=0xfee0
            
        return unichr(inside_code)

    def CharQ2B(self, uchar):
        """char of Q2B"""
        inside_code = ord(uchar)
        if inside_code == 0x3000:
            inside_code = 0x0020
        else:
            inside_code -= 0xfee0
        if inside_code < 0x0020 or inside_code > 0x7e: 
            return uchar
        
        return unichr(inside_code)

    def Q2B(self, ustring):
        """transfor a unicode string from Q 2 B"""
        return "".join([self.CharQ2B(uchar) for uchar in ustring])

    def CountChineseChar(self, ustring):
        count = 0
        for c in ustring:
            if self.IsChineseChar(c):
                count += 1
        return count

    def CountSubStr(self, utext, ustr):
        """
        count all the substr of ustr in utext sorted by freqency.
        eg. utext = "012342323", ustr = "123",
        return [["23", 2], "123", 1]
        """
        freq = {}
        for sub in self.SubStr(ustr):
            c = utext.count(sub)
            if c > 0:
                freq[sub] = c
        item = sorted(freq.keys(), key = lambda x: len(x), reverse = True)
        for s in item:
            if freq.get(s, 0) == 0:
                continue
            for sub in self.SubStr(s, True):
                freq[sub] -= freq[s]
                if freq[sub] == 0:
                    del freq[sub]
                
        return sorted(freq.items(), key = lambda x: x[1], reverse = True)

if __name__ == "__main__":
    try:
        import psyco
        psyco.full()
        print "optimization on"
    except:
        print "optimization off"        
    
    if len(sys.argv) <= 2:
        print "format error! usage: cmd file1 ..filen code"
        exit()

    fse = sys.argv[1: -1]
    code = sys.argv[-1]

    et = ET()
    for fs in fse:
        print "dealing %s" %fs,
        infile = file(fs, "rU")
        oufile = file(fs + ".A2B", "w")
        for line in infile:
            print >> oufile, et.Q2B(line.decode(code)).encode(code),
        oufile.close()
        print "OK"
    print "all is OK"

