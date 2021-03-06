#include "stdafx.h"
#include "StringHelper.h"
#include "Poco/SHA1Engine.h"
#include "Poco/MD5Engine.h"
#include "Poco/DigestEngine.h"
#include "Poco/Buffer.h"
#include "Poco/UnicodeConverter.h"
#include <assert.h>

#ifndef NOWINDOWS
#include <Windows.h>
#endif

#define AND_COUNT 19
#define AND1 4
#define AND2 13
static int g_some_int[AND_COUNT] = { 67, 2389, 163, 839, 1489, 109, 7211,
8627, 1249, 11087, 10313, 7639, 26263, 27919, 9929, 7561, 3373, 11867, 12497 };


// for base64
static const char *codes =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char map64[256] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255,
    255, 254, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6,
    7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
    255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255
};

using  Poco::UnicodeConverter;
std::string W2A_UTF8(const std::wstring& strIn)
{
#ifdef NOWINDOWS
    std::string tmp;
    UnicodeConverter::toUTF8(strIn, tmp);
    return tmp;
#else
    DWORD dwNum = WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), -1, NULL, 0, NULL, NULL);
    Poco::Buffer<char> sTmp(dwNum + 1);
    WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), -1, sTmp.begin(), dwNum, NULL, NULL);
    return sTmp.begin();
#endif

}

std::wstring A2W_UTF8(const std::string& strIn)
{
#ifdef NOWINDOWS
    std::wstring tmp;
    UnicodeConverter::toUTF16(strIn, tmp);
    return tmp;
#else
    DWORD dwNum = MultiByteToWideChar(CP_UTF8, 0, strIn.c_str(), -1, NULL, 0);
    Poco::Buffer<wchar_t> sTmp(dwNum + 1);
    MultiByteToWideChar(CP_UTF8, 0, strIn.c_str(), -1, sTmp.begin(), dwNum);
    return sTmp.begin();
#endif
}

#ifndef NOWINDOWS

std::string W2A_ACP(const std::wstring& strIn)
{
    DWORD dwNum = WideCharToMultiByte(CP_ACP, 0, strIn.c_str(), -1, NULL, 0, NULL, NULL);
    Poco::Buffer<char> sTmp(dwNum + 1);
    WideCharToMultiByte(CP_ACP, 0, strIn.c_str(), -1, sTmp.begin(), dwNum, NULL, NULL);
    return sTmp.begin();
}

std::wstring A2W_ACP(const std::string& strIn)
{
    DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, strIn.c_str(), -1, NULL, 0);
    Poco::Buffer<wchar_t> sTmp(dwNum + 1);
    MultiByteToWideChar(CP_ACP, 0, strIn.c_str(), -1, sTmp.begin(), dwNum);
    return sTmp.begin();
}

std::string W2A_GB2312(const std::wstring& strIn)
{
    DWORD dwNum = WideCharToMultiByte(936, 0, strIn.c_str(), -1, NULL, 0, NULL, NULL);
    Poco::Buffer<char> sTmp(dwNum + 1);
    WideCharToMultiByte(936, 0, strIn.c_str(), -1, sTmp.begin(), dwNum, NULL, NULL);
    return sTmp.begin();
}

std::wstring A2W_GB2312(const std::string& strIn)
{
    DWORD dwNum = MultiByteToWideChar(936, 0, strIn.c_str(), -1, NULL, 0);
    Poco::Buffer<wchar_t> sTmp(dwNum + 1);
    MultiByteToWideChar(936, 0, strIn.c_str(), -1, sTmp.begin(), dwNum);
    return sTmp.begin();
}

#endif
std::string BuildMd5(const std::string& strIn)
{
    Poco::MD5Engine md5;
    md5.update(strIn);
    return Poco::DigestEngine::digestToHex(md5.digest());
//    return digestToUpCaseHex(md5.digest());
}

std::string BuildSha1(const std::string& strIn)
{
    Poco::SHA1Engine sha1;
    sha1.update(strIn);
    return Poco::DigestEngine::digestToHex(sha1.digest());
//    return digestToUpCaseHex(sha1.digest());
}

std::string BuildSha1Up(const std::string& strIn)
{
    Poco::SHA1Engine sha1;
    sha1.update(strIn);
    return digestToUpCaseHex(sha1.digest());
}

std::string digestToUpCaseHex(const Poco::DigestEngine::Digest& bytes)
{
    static const char digits[] = "0123456789ABCDEF";
    std::string result;
    result.reserve(bytes.size() * 2);
    for (Poco::DigestEngine::Digest::const_iterator it = bytes.begin(); it != bytes.end(); ++it)
    {
        unsigned char c = *it;
        result += digits[(c >> 4) & 0xF];
        result += digits[c & 0xF];
    }
    return result;
}

void ToTo(const char* in, char* out, unsigned long len)
{
    unsigned int and1 = g_some_int[(len + AND1) % AND_COUNT];
    unsigned int and2 = g_some_int[(len + AND2) % AND_COUNT];

    unsigned char and_char;

    size_t i = 0;
    for (; i < len; i++)
    {
        and_char = static_cast<unsigned char>((0x80 | (0xff & ((i + and1)*(len + and2)*(len + 12853)))));
        out[i] = in[i] ^ and_char;
    }
}

std::streamsize ToTo(std::istream& istr, std::ostream& ostr, std::streamsize len)
{
    char c;
    unsigned char and_char;

    unsigned int and1 = g_some_int[(len + AND1) % AND_COUNT];
    unsigned int and2 = g_some_int[(len + AND2) % AND_COUNT];

    std::streamsize ret = 0;
    istr.get(c);
    while (istr && ostr)
    {
        and_char = static_cast<unsigned char>((0x80 | (0xff & ((ret + and1)*(len + and2)*(len + 12853)))));
        ++ret;

        ostr.put(c ^ and_char);
        istr.get(c);
    }
    return len;
}

void split(const std::string& s, const std::string& delim, std::vector< std::string >& ret)
{
    size_t last = 0;
    size_t index = s.find_first_of(delim, last);
    while (index != std::string::npos) {
        ret.push_back(s.substr(last, index - last));
        last = index + 1;
        index = s.find_first_of(delim, last);
    }
    if (index - last>0) {
        ret.push_back(s.substr(last, index - last));
    }
}

/* Converts a hex characte5r to its integer value */
char from_hex(char ch) {
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
    static char hex[] = "0123456789ABCDEF";
    return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned std::string after use */
std::string url_encode(const char *str) {
    const char *pstr = str;
    Poco::Buffer<char> buf(strlen(str) * 3 + 1);
    std::memset(buf.begin(), '\0', buf.size());
    char *pbuf = buf.begin();
    while (*pstr) {
        if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
            *pbuf++ = *pstr;
        else if (*pstr == ' ')
            *pbuf++ = '+';
        else
            *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
        pstr++;
    }
    *pbuf = '\0';
    return buf.begin();
}



/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned std::string after use */
std::string url_decode(const char *str) {
    const char *pstr = str;
    Poco::Buffer<char> buf(strlen(str) + 1);
    std::memset(buf.begin(), '\0', buf.size());
    char* pbuf = buf.begin();
    while (*pstr) {
        if (*pstr == '%') {
            if (pstr[1] && pstr[2]) {
                *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
                pstr += 2;
            }
        }
        else if (*pstr == '+') {
            *pbuf++ = ' ';
        }
        else {
            *pbuf++ = *pstr;
        }
        pstr++;
    }
    *pbuf = '\0';
    return buf.begin();
}


int base64_decode(const unsigned char *in, unsigned int inlen,
    unsigned char *out, unsigned int *outlen)
{
    unsigned int t, x, y, z;
    unsigned char c;
    int           g;
    if (!in || !outlen)
        return -1;

    g = 3;
    for (x = y = z = t = 0; x < inlen; x++) {
        c = map64[in[x] & 0xFF];
        if (c == 255) continue;
        /* the final = symbols are read and used to trim the remaining bytes */
        if (c == 254) {
            c = 0;
            /* prevent g < 0 which would potentially allow an overflow later */
            if (--g < 0) {
                return -2;
            }
        }
        else if (g != 3) {
            /* we only allow = to be at the end */
            return -3;
        }

        t = (t << 6) | c;

        if (++y == 4) {
            if (z + g > *outlen) {
                return -4;
            }
            out[z++] = (unsigned char) ((t >> 16) & 255);
            if (g > 1) out[z++] = (unsigned char) ((t >> 8) & 255);
            if (g > 2) out[z++] = (unsigned char) (t & 255);
            y = t = 0;
        }
    }
    if (y != 0) {
        return -5;
    }
    *outlen = z;
    return 0;
}

/**
base64 Encode a buffer (NUL terminated)
@param in      The input buffer to encode
@param inlen   The length of the input buffer
@param out     [out] The destination of the base64 encoded data
@param outlen  [in/out] The max size and resulting size
@return 0 if successful
*/
int base64_encode(const unsigned char *in, unsigned int inlen,
    unsigned char *out, unsigned int *outlen)
{
    unsigned int i, len2, leven;
    unsigned char *p;
    if (!in || !outlen)
        return -1;

    /* valid output size ? */
    len2 = 4 * ((inlen + 2) / 3);
    if (*outlen < len2 + 1) {
        *outlen = len2 + 1;
        return -1;
    }
    p = out;
    leven = 3 * (inlen / 3);
    for (i = 0; i < leven; i += 3) {
        *p++ = codes[(in[0] >> 2) & 0x3F];
        *p++ = codes[(((in[0] & 3) << 4) + (in[1] >> 4)) & 0x3F];
        *p++ = codes[(((in[1] & 0xf) << 2) + (in[2] >> 6)) & 0x3F];
        *p++ = codes[in[2] & 0x3F];
        in += 3;
    }
    /* Pad it if necessary...  */
    if (i < inlen) {
        unsigned a = in[0];
        unsigned b = (i + 1 < inlen) ? in[1] : 0;

        *p++ = codes[(a >> 2) & 0x3F];
        *p++ = codes[(((a & 3) << 4) + (b >> 4)) & 0x3F];
        *p++ = (i + 1 < inlen) ? codes[(((b & 0xf) << 2)) & 0x3F] : '=';
        *p++ = '=';
    }

    /* append a NULL byte */
    *p = '\0';

    /* return ok */
    *outlen = (unsigned int) (p - out);
    return 0;
}

