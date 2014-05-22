#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <iostream>

const char* jedn[] = { "", "jeden", "dwa", "trzy", "cztery", "pięć", "sześć", "siedem", "osiem", "dziewięć" };
const char* nasc[] = { "dziesięć", "jedenaście", "dwanaście", "trzynaście", "czternaście", "piętnaście", "szesnaście", "siedemnaście", "osiemnaście", "dziewiętnaście" };
const char* dzie[] = { "", "dziesięć", "dwadzieścia", "trzydzieści", "czterdzieści", "pięćdziesiąt", "sześćdziesiąt", "siedemdziesiąt", "osiemdziesiąt", "dziewięćdziesiąt" };
const char* setk[] = { "", "sto", "dwieście", "trzysta", "czterysta", "pięćset", "sześćset", "siedemset", "osiemset", "dziewięćset" };
const char* tysi[] = { "", "tysięcy", "tysiące", "tysiące", "tysiące", "tysięcy", "tysięcy", "tysięcy", "tysięcy", "tysięcy" };

const char* wiecej[] = { "tysiąc", "milion", "miliard", "bilion", "biliard", "trylion", "tryliard" };
const char* suff[] = { "", "ów", "y", "y", "y", "ów", "ów", "ów", "ów", "ów" };

const int maxGroups = 2 + sizeof(wiecej) / sizeof(const char*);    // jedna grupa w zapasie

void printNum(unsigned long long int num, FILE* file)
{
    if (!num) {
        fprintf(file, "zero\n");
        return;
    }

    unsigned short nums[maxGroups];
    unsigned int i = 0;

    for (i = maxGroups - 1; i + 1 > 0; --i) nums[i] = 0;
    ++i;

    while (num) {
        nums[i++] = num % 1000;
        num /= 1000;
    }

    for (--i; i + 1 > 0; --i) {
        unsigned int dig[3] = { nums[i] / 100, (nums[i] / 10) % 10, nums[i] % 10 };

        if (dig[0]) fprintf(file, "%s ", setk[dig[0]]);                                            // [-set]
        if (dig[1]) {
            if (dig[1] == 1) fprintf(file, "%s ", nasc[dig[2]]);                                // [-nascie]
            else {
                fprintf(file, dzie[dig[1]]);                                                    // [-dziesiat]
                if (dig[2]) fprintf(file, " %s", jedn[dig[2]]);                                    // [1-9]
                fprintf(file, " ");
            }
        } else if (dig[2] && (!i /* na koncu */ ||
                            nums[i] / 10 /* -set/-dziesiat */ ||
                            dig[2] > 1)) fprintf(file, "%s ", jedn[dig[2]]);                    // [1-9]

        if (i > 0 && nums[i]) {
            if (i > 1) {
                if (nums[i]) fprintf(file, wiecej[i - 1]);                                // [milion]
                if (nums[i] > 1) {
                    if (dig[2] && dig[1] != 1) fprintf(file, "%s ", suff[dig[2]]);        // [~ow] [2-4 ~y] [5-9 ~ow]
                    else fprintf(file, "%s ", suff[9]);                                    // [-nascie/-dziesiat/-set ~ow]
                }
            } else {
                if (nums[i] == 1) fprintf(file, "%s ", wiecej[0]);                        // [tysiąc]
                else if (dig[2] && dig[1] != 1) fprintf(file, "%s ", tysi[dig[2]]);        // [??1 tysięcy] [??2-4 tysiące] [??5-9 tysięcy]
                else fprintf(file, "%s ", tysi[9]);                                        // [-nascie/-set tysięcy]
            }
        }
    }

    fprintf(file, "\n");
}

int main()
{
    for (unsigned long long i = 0; i < 100; ++i)
        printNum(i, stdout);
    system("pause");
}

/*
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>



typedef __int64 int_t;

class BigInt
{
private:
    static const int_t MAX_DEC = 1000000000;
    static const size_t MAX_DIGITS = 9;
    bool mIsPositive;
    std::vector<int_t> mArr; // [0] - najmniej znaczace pozycje

    size_t elems() const { return mArr.size(); }
    void append(int_t a) { mArr.push_back(a); }
    int_t operator [](size_t i) const { return mArr[i]; }
    void trim()
    {
        for (size_t i = mArr.size() - 1; !mArr[i] && i != (size_t)-1; --i)
            mArr.pop_back();
    }

public:
    BigInt(): mIsPositive(true) {}
    BigInt(int_t n): mIsPositive(true)
    {
        mArr.push_back(n);
    }

    BigInt operator +() const { return *this; }
    BigInt operator -() const
    {
        BigInt ret = *this;
        ret.mIsPositive = !mIsPositive;
        return ret;
    }

    BigInt& operator ++()
    {
        *this += 1;
        return *this;
    }

    BigInt operator ++(int nvm)
    {
        BigInt ret = *this;
        *this += 1;
        return ret;
    }

    BigInt operator +(const BigInt& c) const
    {
        // robimy tak, zeby zawsze operowac na liczbach o tym samym znaku
        if (mIsPositive && !c.mIsPositive)
            return *this - (-c);
        else if (!mIsPositive && c.mIsPositive)
            return c - (-*this);

        BigInt ret;
        int_t maxElems = std::max(elems(), c.elems());
        int_t tmp = 0;

        for (size_t i = 0; i < maxElems; ++i)
        {
            if (i >= ret.mArr.size())
                ret.mArr.push_back(0LL);

            int_t n = 0;
            if (i < c.elems())
                n = c.mArr[i];

            tmp += mArr[i] + n;
            ret.mArr[i] = tmp % MAX_DEC;
            tmp /= MAX_DEC;
        }

        return ret;
    }

    BigInt operator -(const BigInt& c) const
    {
        // jesli znaki sa przeciwne, to jest to wlasciwie dodawanie
        if (mIsPositive ^ c.mIsPositive)
            return *this + (-c);

        BigInt ret;
        int_t maxElems = std::max(elems(), c.elems());
        int_t tmp = 0;

        for (size_t i = 0; i < maxElems; ++i)
        {
            tmp += (i < elems() ? mArr[i] : 0) - (i < c.elems() ? c[i] : 0);

            if (tmp < 0)
            {
                ret.mArr.push_back((tmp + MAX_DEC) % MAX_DEC);
                tmp -= MAX_DEC;
            }
            else
                ret.mArr.push_back(tmp % MAX_DEC);

            tmp /= MAX_DEC;
        }

        if (tmp)
            ret.mArr.push_back(tmp % MAX_DEC);

        ret.trim();
        ret.mIsPositive = *this >= c;

        return ret;
    }

    BigInt operator *(const BigInt& c) const
    {
        BigInt ret;

        for (size_t i = 0; i < elems(); ++i)
        {
            int_t next = 0;

            for (size_t j = 0; j < c.elems(); ++j)
            {
                next += mArr[i] * c.mArr[j];

                if (i + j >= ret.elems())
                    ret.mArr.push_back(0);

                ret.mArr[i + j] += next % MAX_DEC;
                next /= MAX_DEC;
            }

            if (next)
            {
                if (i + c.elems() >= ret.elems())
                    ret.mArr.push_back(next);
                else
                    ret.mArr[i + c.elems()] += next;
            }
        }

        return ret;
    }

    BigInt& operator +=(const BigInt& c)
    {
        int_t maxElems = std::max(elems(), c.elems());
        int_t tmp = 0;

        for (size_t i = 0; i < maxElems; ++i)
        {
            if (i >= mArr.size())
                mArr.push_back(0LL);

            int_t n = 0;
            if (i < c.elems())
                n = c.mArr[i];

            tmp += mArr[i] + n;
            mArr[i] = tmp % MAX_DEC;
            tmp /= MAX_DEC;
        }

        return *this;
    }

    BigInt& operator -=(const BigInt& c)
    {
        // jesli znaki sa przeciwne, to jest to wlasciwie dodawanie
        if (mIsPositive ^ c.mIsPositive)
            return *this += (-c);

        int_t maxElems = std::max(elems(), c.elems());
        int_t tmp = 0;
        size_t i;
        bool positive = *this >= c;

        if (elems() < maxElems)
            mArr.resize((size_t)maxElems, 0);

        for (i = 0; i < maxElems; ++i)
        {
            tmp += (i < elems() ? mArr[i] : 0) - (i < c.elems() ? c[i] : 0);

            if (tmp < 0)
            {
                mArr[i] = (tmp + MAX_DEC) % MAX_DEC;
                tmp -= MAX_DEC;
            }
            else
                mArr[i] = tmp % MAX_DEC;

            tmp /= MAX_DEC;
        }

        if (tmp)
            mArr[i] = tmp % MAX_DEC;

        trim();
        mIsPositive = positive;

        return *this;
    }

    BigInt& operator *=(const BigInt& c)
    {
        for (size_t i = 0; i < elems(); ++i)
        {
            int_t next = 0;

            for (size_t j = 0; j < c.elems(); ++j)
            {
                next += mArr[i] * c.mArr[j];

                if (i + j >= elems())
                    mArr.push_back(0);

                mArr[i + j] += next % MAX_DEC;
                next /= MAX_DEC;
            }

            if (next)
            {
                if (i + c.elems() >= elems())
                    mArr.push_back(next);
                else
                    mArr[i + c.elems()] += next;
            }
        }

        return *this;
    }

    bool operator >(const BigInt& a) const
    {
        if (elems() > a.elems()) return true;

        for (size_t i = elems() - 1; i != (size_t)-1; --i)
        {
            if (mArr[i] > a[i]) return true;
            if (mArr[i] < a[i]) return false;
        }

        return false;
    }

    bool operator >=(const BigInt& a) const
    {
        if (elems() > a.elems()) return true;

        for (size_t i = elems() - 1; i != (size_t)-1; --i)
        {
            if (mArr[i] > a[i]) return true;
            if (mArr[i] < a[i]) return false;
        }

        return true;
    }

    bool operator <(const BigInt& a) const
    {
        if (elems() < a.elems()) return true;

        for (size_t i = elems() - 1; i != (size_t)-1; --i)
        {
            if (mArr[i] < a[i]) return true;
            if (mArr[i] > a[i]) return false;
        }

        return false;
    }

    bool operator <=(const BigInt& a) const
    {
        if (elems() < a.elems()) return true;

        for (size_t i = elems() - 1; i != (size_t)-1; --i)
        {
            if (mArr[i] < a[i]) return true;
            if (mArr[i] > a[i]) return false;
        }

        return true;
    }

    bool operator ==(const BigInt& a) const
    {
        if (elems() != a.elems()) return false;

        for (size_t i = 0; i < elems(); ++i)
            if (mArr[i] != a[i]) return false;

        return true;
    }

    bool operator != (const BigInt& a) const
    {
        if (elems() != a.elems()) return true;

        for (size_t i = 0; i < elems(); ++i)
            if (mArr[i] != a[i]) return true;

        return false;
    }

friend std::ostream& operator <<(std::ostream& str, const BigInt& n);
friend std::istream& operator >>(std::istream& str, BigInt& n);
friend BigInt palindrom(const BigInt& n);
};

static int digitsCount(int_t n)
{
    if (!n) return 1;

    int i = 0;
    for (; n; ++i, n /= 10);
    return i;
}

std::ostream& operator <<(std::ostream& str, const BigInt& n)
{
    if (n.elems() == 0) str << "<null>";
    else
    {
        str << n[n.elems() - 1];
        for (size_t i = n.elems() - 2; i != (size_t)-1; --i)
        {
            for (int j = digitsCount(n[i]); j < BigInt::MAX_DIGITS; ++j)
                str << '0';
            str << n[i];
        }
    }

    return str;
}

std::istream& operator >>(std::istream& str, BigInt& n)
{
    n.mArr.clear();

    std::string tmp;
    std::cin >> tmp;

    while (tmp.size())
    {
        std::string last10 = tmp.size() > BigInt::MAX_DIGITS ? tmp.substr(tmp.size() - BigInt::MAX_DIGITS) : tmp;
        tmp = tmp.size() > BigInt::MAX_DIGITS ? tmp.substr(0, tmp.size() - BigInt::MAX_DIGITS) : "";

        n.append(_atoi64(last10.c_str()));
    }

    return str;
}




BigInt silnia(int_t n)
{
    if (n <= 1) return 1;
    return BigInt(n) * silnia(n-1);
}

BigInt palindrom(const BigInt& n)
{
    BigInt ret = 0;

    for (int_t i = 0; i < ret.elems(); ++i)
    {
        int_t part = n[i];
        for (int_t j = 0; ret.elems() > i && j < BigInt::MAX_DIGITS; ++j)
        {
            ret = BigInt(10) * ret + BigInt(part % 10);
            part /= 10;
        }
    }

    return ret;
}

int main()
{
    std::cout << palindrom(123456) << "\n" << palindrom(BigInt(111111111) * BigInt(111111111) * BigInt(1000000));

    PRINT_MEMLEAK_REPORT();
    getchar();
}
*/
