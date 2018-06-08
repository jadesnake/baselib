var class2type = {},
    core_toString = class2type.toString,
    objArr = "Boolean Number String Function Array Date RegExp Object Error".split(" ");
    for(var i = 0; i < objArr.length ;i++){
        class2type["[object " + objArr[i] + "]"] = objArr[i].toLowerCase();
    }
var jQueryT = {
    isFunction: function (obj) {
        return jQueryT.type(obj) === "function";
    },
    isArray: Array.isArray || function (obj) {
        return jQueryT.type(obj) === "array";
    },
    isWindow: function (obj) {
        return obj != null && obj == obj.window;
    },
    type: function (obj) {
        if (obj == null) {
            return String(obj);
        }
        return typeof obj === "object" || typeof obj === "function" ?
            class2type[core_toString.call(obj)] || "object" :
            typeof obj;
    },
    isPlainObject: function (obj) {
        var key;
        if (!obj || jQueryT.type(obj) !== "object" || obj.nodeType || jQueryT.isWindow(obj)) {
            return false;
        }
        try {
            if (obj.constructor &&
                !core_hasOwn.call(obj, "constructor") &&
                !core_hasOwn.call(obj.constructor.prototype, "isPrototypeOf")) {
                return false;
            }
        } catch (e) {
            return false;
        }
        if (jQueryT.support.ownLast) {
            for (key in obj) {
                return core_hasOwn.call(obj, key);
            }
        }
        for (key in obj) { }

        return key === undefined || core_hasOwn.call(obj, key);
    }
}

jQueryT.extend = function () {
    var options, name, src, copy, copyIsArray, clone,
        target = arguments[0] || {},
        i = 1,
        length = arguments.length,
        deep = false;
    if (typeof target === "boolean") {
        deep = target;
        target = arguments[1] || {};
        i = 2;
    }
    if (typeof target !== "object" && !jQueryT.isFunction(target)) {
        target = {};
    }
    if (length === i) {
        target = this;
        --i;
    }
    for (; i < length; i++) {
        if ((options = arguments[i]) != null) {
            for (name in options) {
                src = target[name];
                copy = options[name];
                if (target === copy) {
                    continue;
                }
                if (deep && copy && (jQueryT.isPlainObject(copy) || (copyIsArray = jQueryT.isArray(copy)))) {
                    if (copyIsArray) {
                        copyIsArray = false;
                        clone = src && jQueryT.isArray(src) ? src : [];
                    } else {
                        clone = src && jQueryT.isPlainObject(src) ? src : {};
                    }
                    target[name] = jQueryT.extend(deep, clone, copy);
                } else if (copy !== undefined) {
                    target[name] = copy;
                }
            }
        }
    }
    return target;
};

jQueryT.bs = function (r) {
    function t(r, t) {
        var e = h.indexOf(r.charAt(t));
        if (-1 === e) throw "Cannot decode encrypt";
        return e
    }

    function e(r, t) {
        var e = r.charCodeAt(t);
        if (e > 255) throw "INVALID_CHARACTER_ERR: DOM Exception 5";
        return e
    }
    var n = "=",
        h = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    return {
        decode: function (r) {
            var e, h, a = 0,
                c = r.length,
                o = [];
            if (r = String(r), 0 === c) return r;
            if (c % 4 != 0) throw "Cannot decode base";
            for (r.charAt(c - 1) === n && (a = 1, r.charAt(c - 2) === n && (a = 2), c -= 4), e = 0; e < c; e += 4) h = t(r, e) << 18 | t(r, e + 1) << 12 | t(r, e + 2) << 6 | t(r, e + 3), o.push(String.fromCharCode(h >> 16, h >> 8 & 255, 255 & h));
            switch (a) {
                case 1:
                    h = t(r, e) << 18 | t(r, e + 1) << 12 | t(r, e + 2) << 6, o.push(String.fromCharCode(h >> 16, h >> 8 & 255));
                    break;
                case 2:
                    h = t(r, e) << 18 | t(r, e + 1) << 12, o.push(String.fromCharCode(h >> 16))
            }
            return o.join("")
        },
        encode: function (r) {
            if (1 !== arguments.length) throw "SyntaxError: exactly one argument required";
            var t, a, c = [],
                o = (r = String(r)).length - r.length % 3;
            if (0 === r.length) return r;
            for (t = 0; t < o; t += 3) a = e(r, t) << 16 | e(r, t + 1) << 8 | e(r, t + 2), c.push(h.charAt(a >> 18)), c.push(h.charAt(a >> 12 & 63)), c.push(h.charAt(a >> 6 & 63)), c.push(h.charAt(63 & a));
            switch (r.length - o) {
                case 1:
                    a = e(r, t) << 16, c.push(h.charAt(a >> 18) + h.charAt(a >> 12 & 63) + n + n);
                    break;
                case 2:
                    a = e(r, t) << 16 | e(r, t + 1) << 8, c.push(h.charAt(a >> 18) + h.charAt(a >> 12 & 63) + h.charAt(a >> 6 & 63) + n)
            }
            return c.join("")
        },
        VERSION: "1.0"
    }
}(jQueryT);

!function (e) {
    var n = function () {
        var e = 1600,n = 758;
            if (e * n <= 12e4) return !0;
            var c = 0, r = 86;
            return c + e <= 0 || r + n <= 0 || c >= 1600 || r >= 900;
        },
        c = function (c) {
            wzwschallenge = e.encrypt("rhrewrchb"), wzwschallengex = e.bs.encode("rhrewrchb"), encoderchars = e.encrypt(wzwschallenge) + e.encrypt(wzwschallengex);
            var t, h, o, d, i, a;
            for (o = c.length, h = 0, t = ""; h < o;) {
                if (d = 255 & c.charCodeAt(h++), h == o) {
                    t += encoderchars.charAt(d >> 2), t += encoderchars.charAt((3 & d) << 4), t += "==";
                    break
                }
                if (i = c.charCodeAt(h++), h == o) {
                    t += encoderchars.charAt(d >> 2), t += encoderchars.charAt((3 & d) << 4 | (240 & i) >> 4), t += encoderchars.charAt((15 & i) << 2), t += "=";
                    break
                }
                a = c.charCodeAt(h++), t += encoderchars.charAt(d >> 2), t += encoderchars.charAt((3 & d) << 4 | (240 & i) >> 4), t += encoderchars.charAt((15 & i) << 2 | (192 & a) >> 6), t += encoderchars.charAt(63 & a)
            }
            var w = 0;
            return n() || (w = r(wzwschallenge, wzwschallengex)), t + w
        },
        r = function (e, n) {
            var c = e + n,
                r = 0,
                t = 0;
            for (t = 0; t < c.length; t++) r += c.charCodeAt(t);
            return r *= 245, r += 963863
        };
    e.extend({
        gen: function (n, c) {
            var r, t, h, o = (o = n += "").length,
                d = e.encrypt(n),
                i = e.encrypt(n) + c;
            for ((t = new Array)[0] = "ff8080815ed2f53b015f27c2b7b9783e", t[1] = "402880bd5c76166f015c9041698e5099", t[2] = "402880bd5c76166f015c903ee811504e", r = 0; r < t.length; ++r) h = t[r];
            var a = 1600, w = 900;
            return a * w <= 12e4 ? e.encrypt(d + i + h).toUpperCase() : e.encrypt(d + i + h + o).toUpperCase()
        },
        moveTo: function (n) {
            return e.encrypt(c(n))
        }
    })
}(jQueryT);

!function ($) {
    var key, replaceLf = function (e, c) {
            return key = "402880bd5c76166f015c903ee811504e", e << c | e >>> 32 - c
        },
        F = function (e, c, n) {
            return key = "402880bd5c76166", e & c | ~e & n
        },
        G = function (e, c, n) {
            return key = "402880bd5c76166", e & n | c & ~n
        },
        H = function (e, c, n) {
            return key = "3ee811504e", e ^ c ^ n
        },
        I = function (e, c, n) {
            return c ^ (e | ~n)
        },
        FF = function (e, c, n, r, t, $, o) {
            return e = edUnsig(e, edUnsig(edUnsig(F(c, n, r), t), o)), edUnsig(replaceLf(e, $), c)
        },
        GG = function (e, c, n, r, t, $, o) {
            return e = edUnsig(e, edUnsig(edUnsig(G(c, n, r), t), o)), edUnsig(replaceLf(e, $), c)
        },
        HH = function (e, c, n, r, t, $, o) {
            return e = edUnsig(e, edUnsig(edUnsig(H(c, n, r), t), o)), edUnsig(replaceLf(e, $), c)
        },
        II = function (e, c, n, r, t, $, o) {
            return e = edUnsig(e, edUnsig(edUnsig(I(c, n, r), t), o)), edUnsig(replaceLf(e, $), c)
        },
        edUnsig = function (e, c) {
            var n, r, $, o;
            return t = 2147483648 & e, $ = 2147483648 & c, n = 1073741824 & e, r = 1073741824 & c, o = (1073741823 & e) + (1073741823 & c), n & r ? 2147483648 ^ o ^ lX8 ^ $ : n | r ? 1073741824 & o ? 3221225472 ^ o ^ lX8 ^ $ : 1073741824 ^ o ^ lX8 ^ $ : o ^ lX8 ^ $
        };
    $.extend({
        ck: function (e, c, n, r, t, o) {
            var a, p = G(c, e, n),
                d = $.encrypt(e),
                i = $.encrypt(r + t),
                y = replaceLf(e, c);
            p = 2147483648 & e, p += 2147483648 & c, p += a, p += a = 1073741824 & p, y = p = $.encrypt(e) + $.bs.encode($.encrypt(c)) + n;
            var g = $.gen(p, y),
                u = $.encrypt(d) + i,
                s = $.gen(g + $.gen(e, y) + u, i);
            return alert("public :" + s), $.prijm(e, c, n, r, t, o, s)
        },
        checkTaxno: function (a, b, c, d, e) {
            var c2 = "";
            return c2 = $.encrypt($.xx(a) + b), c2 = $.encrypt($.gen(c2, $.yy(a)) + $.moveTo(b) + $.bs.encode($.yy($.gen(b, a))) + $.gen($.bs.encode(c2), $.xx($.moveTo(c2.length + b + e))) + e).toUpperCase(), cc2 = eval(d), cc2 = $.encrypt(cc2), cc2
        },
        checkOneInv: function (a, b, c, d, e, f, g) {
            var c2 = "";
            return c2 = $.gen($.moveTo($.encrypt($.xx($.gen(a, b)))), $.yy($.moveTo($.xx(e + (a + c + d)))) + $.bs.encode($.encrypt($.xx($.bs.encode($.xx(c) + b + $.bs.encode(d + e)))))).toUpperCase(), cc = eval(g), cc = $.encrypt(cc), cc
        },
        checkInvConf: function (a, b, c, d, e) {
            var c2 = "";
            return c2 = $.gen($.gen(a.toUpperCase(), $.yy($.encrypt($.encrypt(a + c.length)))), c.toUpperCase() + $.bs.encode(a + $.encrypt(a + b)) + $.xx($.moveTo(c))).toUpperCase(), cc = eval(e), cc = $.encrypt(cc), cc
        },
        checkDeduDown: function (a, b, c, d, e, f) {
            var c2 = "";
            return c2 = $.gen($.gen(a.toUpperCase(), $.yy($.encrypt($.encrypt(a + c.length)))), c.toUpperCase() + $.bs.encode(a + $.encrypt(a + b)) + $.xx($.moveTo(c))).toUpperCase(), cc = eval(e), cc = $.encrypt(cc), cc
        }
    })
}(jQueryT);

!function (r) {
    var e = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
        t = new Array((-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), 62, (-1), (-1), (-1), 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, (-1), (-1), (-1), (-1), (-1), (-1), (-1), 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, (-1), (-1), (-1), (-1), (-1), (-1), 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, (-1), (-1), (-1), (-1), (-1)),
        a = function (r) {
            var t, a, n, o, c, h;
            for (n = r.length, a = 0, t = ""; a < n;) {
                if (o = 255 & r.charCodeAt(a++), a == n) {
                    t += e.charAt(o >> 2), t += e.charAt((3 & o) << 4), t += "==";
                    break
                }
                if (c = r.charCodeAt(a++), a == n) {
                    t += e.charAt(o >> 2), t += e.charAt((3 & o) << 4 | (240 & c) >> 4), t += e.charAt((15 & c) << 2), t += "=";
                    break
                }
                h = r.charCodeAt(a++), t += e.charAt(o >> 2), t += e.charAt((3 & o) << 4 | (240 & c) >> 4), t += e.charAt((15 & c) << 2 | (192 & h) >> 6), t += e.charAt(63 & h)
            }
            return t
        },
        n = function (r) {
            var e, a, n, o, c, h, f;
            for (h = r.length, c = 0, f = ""; c < h;) {
                do e = t[255 & r.charCodeAt(c++)]; while (c < h && -1 == e);
                if (-1 == e) break;
                do a = t[255 & r.charCodeAt(c++)]; while (c < h && -1 == a);
                if (-1 == a) break;
                f += String.fromCharCode(e << 2 | (48 & a) >> 4);
                do {
                    if (61 == (n = 255 & r.charCodeAt(c++))) return f;
                    n = t[n]
                } while (c < h && -1 == n);
                if (-1 == n) break;
                f += String.fromCharCode((15 & a) << 4 | (60 & n) >> 2);
                do {
                    if (61 == (o = 255 & r.charCodeAt(c++))) return f;
                    o = t[o]
                } while (c < h && -1 == o);
                if (-1 == o) break;
                f += String.fromCharCode((3 & n) << 6 | o)
            }
            return f
        },
        o = function (r) {
            var e, t, a, n;
            for (e = "", a = r.length, t = 0; t < a; t++)(n = r.charCodeAt(t)) >= 1 && n <= 127 ? e += r.charAt(t) : n > 2047 ? (e += String.fromCharCode(224 | n >> 12 & 15), e += String.fromCharCode(128 | n >> 6 & 63), e += String.fromCharCode(128 | n >> 0 & 63)) : (e += String.fromCharCode(192 | n >> 6 & 31), e += String.fromCharCode(128 | n >> 0 & 63));
            return e
        },
        c = function (r) {
            var e, t, a, n, o, c;
            for (e = "", a = r.length, t = 0; t < a;) switch ((n = r.charCodeAt(t++)) >> 4) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                    e += r.charAt(t - 1);
                    break;
                case 12:
                case 13:
                    o = r.charCodeAt(t++), e += String.fromCharCode((31 & n) << 6 | 63 & o);
                    break;
                case 14:
                    o = r.charCodeAt(t++), c = r.charCodeAt(t++), e += String.fromCharCode((15 & n) << 12 | (63 & o) << 6 | (63 & c) << 0)
            }
            return e
        },
        h = function (r) {
            var e, t, a, n;
            for (e = "", a = r.length, t = 0; t < a;)(n = r.charCodeAt(t++).toString(16)).length < 2 && (n = "0" + n), e += "\\x" + n + " ", t > 0 && t % 8 == 0 && (e += "\r\n");
            return e
        },
        f = function (r) {
            for (var e = "", t = (r = r.replace(/=/g, "")).length; t >= 0; t--) e += r.charAt(t);
            return e
        };
    r.extend({
        ben: function (r) {
            return a(o(r))
        },
        dde: function (r) {
            return h(n(r))
        },
        dde16: function (r) {
            return c(n(r))
        },
        ctr: function (r) {
            return f(r)
        }
    })
}(jQueryT);
//m.q.d.min.js
!function (n) {
    var r = function (n, r) {
        return n << r | n >>> 32 - r;
    }, t = function (n, r) {
        var t, e, u, o, i;
        return u = 2147483648 & n, o = 2147483648 & r, t = 1073741824 & n, e = 1073741824 & r, i = (1073741823 & n) + (1073741823 & r), t & e ? 2147483648 ^ i ^ u ^ o : t | e ? 1073741824 & i ? 3221225472 ^ i ^ u ^ o : 1073741824 ^ i ^ u ^ o : i ^ u ^ o;
    }, e = function (n, r, t) {
        return n & r | ~n & t;
    }, u = function (n, r, t) {
        return n & t | r & ~t;
    }, o = function (n, r, t) {
        return n ^ r ^ t;
    }, i = function (n, r, t) {
        return r ^ (n | ~t);
    }, f = function (n, u, o, i, f, c, g) {
        return n = t(n, t(t(e(u, o, i), f), g)), t(r(n, c), u);
    }, c = function (n, e, o, i, f, c, g) {
        return n = t(n, t(t(u(e, o, i), f), g)), t(r(n, c), e);
    }, g = function (n, e, u, i, f, c, g) {
        return n = t(n, t(t(o(e, u, i), f), g)), t(r(n, c), e);
    }, a = function (n, e, u, o, f, c, g) {
        return n = t(n, t(t(i(e, u, o), f), g)), t(r(n, c), e);
    }, h = function (n) {
        for (var r, t = n.length, e = t + 8, u = 16 * ((e - e % 64) / 64 + 1), o = Array(u - 1), i = 0, f = 0; f < t;)
            i = f % 4 * 8, o[r = (f - f % 4) / 4] = o[r] | n.charCodeAt(f) << i, f++;
        return r = (f - f % 4) / 4, i = f % 4 * 8, o[r] = o[r] | 128 << i, o[u - 2] = t << 3, o[u - 1] = t >>> 29, o;
    }, l = function (n) {
        var r, t = "", e = "";
        for (r = 0; r <= 3; r++)
            t += (e = "0" + (n >>> 8 * r & 255).toString(16)).substr(e.length - 2, 2);
        return t;
    }, C = function (n) {
        n = n.replace(/\x0d\x0a/g, "\n");
        for (var r = "", t = 0; t < n.length; t++) {
            var e = n.charCodeAt(t);
            e < 128 ? r += String.fromCharCode(e) : e > 127 && e < 2048 ? (r += String.fromCharCode(e >> 6 | 192), r += String.fromCharCode(63 & e | 128)) : (r += String.fromCharCode(e >> 12 | 224), r += String.fromCharCode(e >> 6 & 63 | 128), r += String.fromCharCode(63 & e | 128));
        }
        return r;
    };
    jQueryT.extend({ xx: function (n) {
        return (n += "").length >= 12 ? n.substring(0, 10) : n.substring(0, 2);
    }, yy: function (n) {
        return (n += "").length + 5 * n.length;
    }, xcx: function (n) {
        return (n += "").length >= 20 ? n.substring(0, 12) : n.length >= 8 ? n.substring(0, 5) : n;
    }, ll: function (n) {
        return (n += "").length + 3 * n.length;
    }, slave: function (n) {
        return (n += "").length + 6 * n.length;
    }, encrypt: function (n) {
        var r, e, u, o, i, s, d, v, x, S = Array();
        for (n = C(n), S = h(n), s = 1732584193, d = 4023233417, v = 2562383102, x = 271733878, r = 0; r < S.length; r += 16)
            e = s, u = d, o = v, i = x, s = f(s, d, v, x, S[r + 0], 7, 3614090360), x = f(x, s, d, v, S[r + 1], 12, 3905402710), v = f(v, x, s, d, S[r + 2], 17, 606105819), d = f(d, v, x, s, S[r + 3], 22, 3250441966), s = f(s, d, v, x, S[r + 4], 7, 4118548399), x = f(x, s, d, v, S[r + 5], 12, 1200080426), v = f(v, x, s, d, S[r + 6], 17, 2821735955), d = f(d, v, x, s, S[r + 7], 22, 4249261313), s = f(s, d, v, x, S[r + 8], 7, 1770035416), x = f(x, s, d, v, S[r + 9], 12, 2336552879), v = f(v, x, s, d, S[r + 10], 17, 4294925233), d = f(d, v, x, s, S[r + 11], 22, 2304563134), s = f(s, d, v, x, S[r + 12], 7, 1804603682), x = f(x, s, d, v, S[r + 13], 12, 4254626195), v = f(v, x, s, d, S[r + 14], 17, 2792965006), d = f(d, v, x, s, S[r + 15], 22, 1236535329), s = c(s, d, v, x, S[r + 1], 5, 4129170786), x = c(x, s, d, v, S[r + 6], 9, 3225465664), v = c(v, x, s, d, S[r + 11], 14, 643717713), d = c(d, v, x, s, S[r + 0], 20, 3921069994), s = c(s, d, v, x, S[r + 5], 5, 3593408605), x = c(x, s, d, v, S[r + 10], 9, 38016083), v = c(v, x, s, d, S[r + 15], 14, 3634488961), d = c(d, v, x, s, S[r + 4], 20, 3889429448), s = c(s, d, v, x, S[r + 9], 5, 568446438), x = c(x, s, d, v, S[r + 14], 9, 3275163606), v = c(v, x, s, d, S[r + 3], 14, 4107603335), d = c(d, v, x, s, S[r + 8], 20, 1163531501), s = c(s, d, v, x, S[r + 13], 5, 2850285829), x = c(x, s, d, v, S[r + 2], 9, 4243563512), v = c(v, x, s, d, S[r + 7], 14, 1735328473), d = c(d, v, x, s, S[r + 12], 20, 2368359562), s = g(s, d, v, x, S[r + 5], 4, 4294588738), x = g(x, s, d, v, S[r + 8], 11, 2272392833), v = g(v, x, s, d, S[r + 11], 16, 1839030562), d = g(d, v, x, s, S[r + 14], 23, 4259657740), s = g(s, d, v, x, S[r + 1], 4, 2763975236), x = g(x, s, d, v, S[r + 4], 11, 1272893353), v = g(v, x, s, d, S[r + 7], 16, 4139469664), d = g(d, v, x, s, S[r + 10], 23, 3200236656), s = g(s, d, v, x, S[r + 13], 4, 681279174), x = g(x, s, d, v, S[r + 0], 11, 3936430074), v = g(v, x, s, d, S[r + 3], 16, 3572445317), d = g(d, v, x, s, S[r + 6], 23, 76029189), s = g(s, d, v, x, S[r + 9], 4, 3654602809), x = g(x, s, d, v, S[r + 12], 11, 3873151461), v = g(v, x, s, d, S[r + 15], 16, 530742520), d = g(d, v, x, s, S[r + 2], 23, 3299628645), s = a(s, d, v, x, S[r + 0], 6, 4096336452), x = a(x, s, d, v, S[r + 7], 10, 1126891415), v = a(v, x, s, d, S[r + 14], 15, 2878612391), d = a(d, v, x, s, S[r + 5], 21, 4237533241), s = a(s, d, v, x, S[r + 12], 6, 1700485571), x = a(x, s, d, v, S[r + 3], 10, 2399980690), v = a(v, x, s, d, S[r + 10], 15, 4293915773), d = a(d, v, x, s, S[r + 1], 21, 2240044497), s = a(s, d, v, x, S[r + 8], 6, 1873313359), x = a(x, s, d, v, S[r + 15], 10, 4264355552), v = a(v, x, s, d, S[r + 6], 15, 2734768916), d = a(d, v, x, s, S[r + 13], 21, 1309151649), s = a(s, d, v, x, S[r + 4], 6, 4149444226), x = a(x, s, d, v, S[r + 11], 10, 3174756917), v = a(v, x, s, d, S[r + 2], 15, 718787259), d = a(d, v, x, s, S[r + 9], 21, 3951481745), s = t(s, e), d = t(d, u), v = t(v, o), x = t(x, i);
        return (l(s) + l(d) + l(v) + l(x)).toLowerCase();
    } });
}();
//m.q.z.min.js
!function (r) {
    var n = function (r) {
        return o(t(f(r), 8 * r.length));
    }, t = function (r, n) {
        r[n >> 5] |= 128 << 24 - n % 32, r[15 + (n + 64 >> 9 << 4)] = n;
        for (var t = Array(80), f = 1732584193, o = -271733879, i = -1732584194, h = 271733878, v = -1009589776, y = 0; y < r.length; y += 16) {
            for (var A = f, g = o, l = i, b = h, d = v, p = 0; p < 80; p++) {
                t[p] = p < 16 ? r[y + p] : a(t[p - 3] ^ t[p - 8] ^ t[p - 14] ^ t[p - 16], 1);
                var j = c(c(a(f, 5), e(p, o, i, h)), c(c(v, t[p]), u(p)));
                v = h, h = i, i = a(o, 30), o = f, f = j;
            }
            f = c(f, A), o = c(o, g), i = c(i, l), h = c(h, b), v = c(v, d);
        }
        return Array(f, o, i, h, v);
    }, e = function (r, n, t, e) {
        return r < 20 ? n & t | ~n & e : r < 40 ? n ^ t ^ e : r < 60 ? n & t | n & e | t & e : n ^ t ^ e;
    }, u = function (r) {
        return r < 20 ? 1518500249 : r < 40 ? 1859775393 : r < 60 ? -1894007588 : -899497514;
    }, c = function (r, n) {
        var t = (65535 & r) + (65535 & n);
        return (r >> 16) + (n >> 16) + (t >> 16) << 16 | 65535 & t;
    }, a = function (r, n) {
        return r << n | r >>> 32 - n;
    }, f = function (r) {
        for (var n = Array(), t = 0; t < 8 * r.length; t += 8)
            n[t >> 5] |= (255 & r.charCodeAt(t / 8)) << 24 - t % 32;
        return n;
    }, o = function (r) {
        for (var n = "0123456789abcdef", t = "", e = 0; e < 4 * r.length; e++)
            t += n.charAt(r[e >> 2] >> 8 * (3 - e % 4) + 4 & 15) + n.charAt(r[e >> 2] >> 8 * (3 - e % 4) & 15);
        return t;
    };
    r.extend({ encrypta: function (r) {
        return n(r);
    }, encryptab: function (t) {
        return n(r.ben(t));
    } });
}(jQueryT);
