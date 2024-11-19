#include <stdio.h>
#include "bf20.h"

/*
    Do NOT include any C libraries or header files
    except those above
    use the defines in d20.h
    if neccessary, you can add some macros below
*/

//                                   28  24  20  16  12  8   4   0
#define         MASK_SIGN       0b00000000000010000000000000000000
#define         MASK_EXP        0b00000000000001111111000000000000
#define         MASK_FRAC       0b00000000000000000000111111111111

#define         EXP_BITS        7
#define         FRAC_BITS       12
#define         BIAS            63

#define         L2R_MAX_INDX    31

// right to left
#define         MASK_FIRST_BIT  0b00000000000000000000000000000001
#define         MASK_LAST_BIT   0b10000000000000000000000000000000

#define         INT_MAX         (2147483647)
#define         INT_MIN         (-2147483647 - 1)

#define         BF20_PINF       0b00000000000001111111000000000000
#define         BF20_MINF       0b00000000000011111111000000000000

#define         BF20_PNAN       0b00000000000001111111000000000001
#define         BF20_MNAN       0b00000000000011111111000000000001

#define         BF20_PZERO      0b00000000000000000000000000000000
#define         BF20_MZERO      0b00000000000010000000000000000000

typedef unsigned int uint;

void print_bits(uint in)
{
    for (int i = 0; i < 32; i++)
    {
        printf("%d", (in & MASK_LAST_BIT) ? 1 : 0);
        in = in << 1;
    }
    printf("\n");
}

uint set_bit(uint number, int indx, int val)
{
    if (indx < 0)
    {
        return number;
    }
    if (val == 0)
    {
        return number & ~(1 << indx);
    }
    else
    {
        return number | (1 << indx);
    }
}

uint chk_bit(uint number, int indx)
{
    if (indx < 0)
    {
        return 0;
    }
    else
    {
        return (number >> indx) & 1;
    }
}

uint maskgen_firstn(int n)
{
    if (n > 0)
    {
        if (n <= 31)
        {
            return (1 << n) - 1;
        }
        else
        {
            return ~0;
        }
    }

    // negative input gives mask that does nothing
    return (~0);
}

uint maskgen_atindx(int indx)
{
    if (indx >= 0)
    {
        return (1 << indx);
    }

    // negative input gives mask that does nothing
    return (~0);
}

uint chk_first_n_bits(uint number, int n)
{
    if (n > 0)
    {
        return number & maskgen_firstn(n);
    }

    return 0;
}

uint round_to_even(uint number, int g_indx)
{
    // reject if g_indx is less than 0 (it means that we dont need to round anything bc r = 0)
    if (g_indx < 0)
    {
        return number;
    }

    uint g = chk_bit(number, g_indx);
    uint r = chk_bit(number, g_indx - 1);
    uint s = chk_first_n_bits(number, (g_indx - 2) + 1); // + 1 to convert an index to a count
    // printf("%d, %d, %d\n", g, r, s);

    // if round bit is zero then gg no increment
    // x0x (N)
    if (r == 0)
    {
        number &= ~maskgen_firstn(g_indx); // clear all less significant bits than g
    }
    // round bit is set
    // we dont have to increment if the number is even (round down)
    // 010 (N)
    else if (g == 0 && r >= 1 && s == 0)
    {
        number &= ~maskgen_firstn(g_indx); // clear all less significant bits than g
    }
    // round bit in set
    // we have to increment if the number is odd (round up)
    // 110 (Y)
    else if (g >= 1 && r >= 1 && s == 0)
    {
        number += (1 << g_indx);
        number &= ~maskgen_firstn(g_indx); // clear all less significant bits than g
    }
    // round bit set
    // else we are in x11 so we are closer to round up target which means we round up
    // x11 (Y)
    else
    {
        number += (1 << g_indx);
        number &= ~maskgen_firstn(g_indx); // clear all less significant bits than g
    }

    return number;
}

uint shift_bit_pattern(uint number, int shift)
{
    if (shift > 0)
    {
        return number >> shift;
    }
    else if (shift < 0)
    {
        return number << -shift;
    }
    else
    {
        return number;
    }
}

uint chk_all_in_mask(uint number, uint mask, uint val)
{
    if (val)
    {
        return (number & mask) == mask;
    }
    else
    {
        return (number & mask) == 0;
    }
}

uint is_p_inf(bf20 in)
{
    return (!(MASK_SIGN & in) && chk_all_in_mask(in, MASK_EXP, 1) && chk_all_in_mask(in, MASK_FRAC, 0));
}

uint is_m_inf(bf20 in)
{
    return ((MASK_SIGN & in) && chk_all_in_mask(in, MASK_EXP, 1) && chk_all_in_mask(in, MASK_FRAC, 0));
}

uint is_inf(bf20 in)
{
    return is_p_inf(in) || is_m_inf(in);
}

uint is_nan(bf20 in)
{
    // printf("nan: %d, %d\n", chk_all_in_mask(in, MASK_EXP, 1), !chk_all_in_mask(in, MASK_FRAC, 0));
    return (chk_all_in_mask(in, MASK_EXP, 1) && !chk_all_in_mask(in, MASK_FRAC, 0));
}

uint is_p_zero(bf20 in)
{
    return in == 0;
}

uint is_m_zero(bf20 in)
{
    return in == MASK_SIGN;
}

uint is_zero(bf20 in)
{
    return in == MASK_SIGN || in == 0;
}

uint extract_sign(bf20 in)
{
    return (in & MASK_SIGN) >> (EXP_BITS + FRAC_BITS);
}

uint extract_exp(bf20 in)
{
    return (in & MASK_EXP) >> (FRAC_BITS);
}

uint extract_frac(bf20 in)
{
    return in & MASK_FRAC;
}

void swap(uint* a, uint* b)
{
    uint tmp = *a;
    *a = *b;
    *b = tmp;
}

bf20 int2bf20(int in)
{
    bf20 result = 0;
    uint abs_in = 0, rnd_in = 0, exp = 0, frac = 0;

    // handle zero quickly
    if (in == 0) return BF20_PZERO;

    // calc abs
    if (in < 0)
    {
        // write sign
        result |= MASK_SIGN;
        abs_in = -in;
    }
    else
    {
        abs_in = in;
    }

    // __builtin_clz returns # of zeroes before first 1
    int highest_bit = L2R_MAX_INDX - __builtin_clz(abs_in);

    rnd_in = round_to_even(abs_in, highest_bit - FRAC_BITS + 1);

    // printf("in:\n");
    // print_bits(in);
    // printf("rounded:\n");
    // print_bits(rnd_in);
    // printf("high_bit=%d\n", highest_bit);

    // recalculate highest bit (it might have shifted because of rounding)
    highest_bit = L2R_MAX_INDX - __builtin_clz(rnd_in);
    // printf("E=%d\n", highest_bit);

    // calculate exponent
    exp = highest_bit + BIAS;

    // move exp into place
    // we dont have to worry abt value truncation because range of int is smaller than bf20
    exp = exp << FRAC_BITS;

    // write exp
    result |= exp;

    // no rounding is required now because if it was rounded before and we didnt increment
    // then rnd_in still contains the rounded result
    // if we did increment and carry changed MSB, still, the result is rounded because
    // every bit after  and including the former g bit is set to zero
    // so we dont need to round nor set anything

    // align the fractional part with index 13 (minding the implied 1)
    frac = MASK_FRAC & shift_bit_pattern(rnd_in, highest_bit - (FRAC_BITS));

    // write frac
    result |= frac;

    // printf("frac:\n");
    // print_bits(frac);
    // printf("exp:\n");
    // print_bits(exp);
    // printf("result:\n");
    // print_bits(result);

    return result;
}

int bf202int(bf20 in)
{
    int result = 0;
    uint tmp = 0;
    int E = 0;
    uint exp = 0, frac = 0, sign = 0;

    // print_bits(in);

    // handle special cases first
    if (is_p_inf(in))
    {
        return INT_MAX;
    }
    else if (is_m_inf(in))
    {
        return INT_MIN;
    }
    else if (is_nan(in))
    {
        return INT_MIN;
    }

    // extract segments
    sign = extract_sign(in);
    exp = extract_exp(in);
    frac = extract_frac(in);

    // calculate unbiased exponent
    E = exp - BIAS;
    // printf("E=%d\n", E);

    // normalized
    if (exp != 0)
    {
        frac |= (1 << FRAC_BITS);  // add implicit leading one
    }
    // else denormalized zero is already there -> extremely small number -> return zero
    else
    {
        return 0;
    }

    int g_indx = 12;

    if (E < 0)
    {
        if (E == -1)
        {
            g_indx++;
        }
        else // (E <= -2)
        {
            // r bit is zero so always round down to zero
            return 0;
        }
    }
    else // (E >= 0)
    {
        // E > 31 means that we shifted the dot right more than 31 times
        // so 1.xxxx became 1xxxx....xxx(33bits) which doesnt fit in an int32
        if (E > 31)
        {
            // if sign its negative inf
            if (sign)
            {
                return INT_MIN;
            }
            // if no sign positive inf
            else
            {
                return INT_MAX;
            }
        }
        else // (0 <= E <= 31)
        {
            g_indx -= E;
            // no need to worry abt overflow because if E is close to 31 we will have
            // at least one padding bit to "intercept" the rounding increment
            // if E puts the floating point in the middle of the mantissa the overflow
            // bit will be correctly positioned anyways
            frac = round_to_even(frac, g_indx);
            frac = shift_bit_pattern(frac, g_indx);
            if (sign)
            {
                result = -frac;
            }
            else
            {
                result = frac;
            }
            return result;
        }
    }
}

bf20 float2bf20(float in)
{
    bf20 result = 0;

    union converter
    {
        float f;
        uint u;
    } c;

    c.f = in;

    uint sign = 0, exp = 0, frac = 0;

    // float specific conversions
    uint fl_sign = (c.u >> 31) & 0x1; // 1 bit mask
    uint fl_exp = (c.u >> 23) & 0xFF; // 8 bit mask
    uint fl_frac = c.u & 0x7FFFFF; // 23 bit mask

    // print_bits(c.u);
    // print_bits(fl_sign);
    // print_bits(fl_exp);
    // print_bits(fl_frac);

    if (fl_exp == 0xFF)
    {
        // exp is 11...11 frac is 00...00 -> inf
        if (fl_frac == 0)
        {
            return fl_sign ? BF20_MINF : BF20_PINF;
        }
        // exp is 11...11 frac is non zero -> nan
        else
        {
            // conserve sign
            return fl_sign ? BF20_MNAN : BF20_PNAN;
        }
    }

    // prepare sign for assembly
    sign = fl_sign ? MASK_SIGN : 0;

    int E = (int)fl_exp - 127; // float32 has bias of 127
    exp = E + BIAS;

    // exp is bigger than the max that bf20 can hold -> inf
    if (exp >= 0b00000000000000000000000001111111)
    {
        return fl_sign ? BF20_MINF : BF20_PINF;
    }

    // we need to "cut into" the float and take 12 frac bits. but also we have
    // to round what we take to the 12th bit
    //   seeeeeeeefffffffffffffffffffffff
    // 0b11000100111100011000000000000000

    // offset from right that we need in order to take 12 most significant bits
    int g_indx = 11;
    // print_bits(frac);
    frac = round_to_even(fl_frac, g_indx);
    // print_bits(frac);
    uint carry_overflow = chk_bit(frac, 23);

    //           23
    // 0b01000000011111111111110000000000

    // if we overflowed because of rounding we have to mask off the bit
    // frac is ready for assembly
    frac = MASK_FRAC & shift_bit_pattern(frac, 11);

    if (carry_overflow)
    {
        // exp is bigger than the max that bf20 can hold -> inf
        if (exp >= 0b00000000000000000000000001111111)
        {
            return fl_sign ? BF20_MINF : BF20_PINF;
        }
    }

    // preapare exp for assembly
    exp = exp << FRAC_BITS;

    // printf("carry_of:%d\n", carry_overflow);
    // print_bits(frac);
    // print_bits(exp);

    result |= sign;
    result |= exp;
    result |= frac;

    return result;
}

float bf202float(bf20 in)
{
    uint result = 0;

    union converter
    {
        float f;
        uint u;
    } c;

    c.u = 0;

    uint sign = 0, exp = 0, frac = 0;
    uint fl_sign = 0, fl_exp = 0, fl_frac = 0;

    sign = extract_sign(in);
    exp = extract_exp(in);
    frac = extract_frac(in);

    // if is -inf return -inf
    if (is_m_inf(in))
    {
        c.u = 0b11111111100000000000000000000000;
        return c.f;
    }
    // if is +inf return +inf
    else if (is_p_inf(in))
    {
        c.u = 0b01111111100000000000000000000000;
        return c.f;
    }
    // if is nan
    else if (is_nan(in))
    {
        // if is -nan return -nan
        if (sign)
        {
            c.u = 0b11111111100000000000000000000001;
            return c.f;
        }
        // if is +nan return +nan
        else
        {
            c.u = 0b01111111100000000000000000000001;
            return c.f;
        }
    }
    // if is zero
    else if (exp == 0 && frac == 0)
    {
        // if is -0 return -0
        if (sign)
        {
            c.u = 0b10000000000000000000000000000000;
            return c.f;
        }
        // if is +0 return +0
        else
        {
            c.u = 0b00000000000000000000000000000000;
            return c.f;
        }
    }

    if (sign)
    {
        // get fl_sign ready for assembly
        fl_sign = set_bit(fl_sign, 31, 1);
    }

    int E = (int)exp - BIAS;
    fl_exp = E + 127; // IEEE 754 has bias of 127
    fl_exp = fl_exp << 23; // get fl_exp ready for assembly

    fl_frac = frac << 11; // get fl_frac ready for assembly

    c.u |= fl_sign;
    c.u |= fl_exp;
    c.u |= fl_frac;

    return c.f;
}

bf20 bf20_add(bf20 a, bf20 b)
{
    bf20 result = 0;

    int res_E = 0;
    uint res_sign = 0, res_exp = 0, res_frac = 0;

    int a_E = 0, b_E = 0;
    uint a_sign = 0, a_exp = 0, a_frac = 0;
    uint b_sign = 0, b_exp = 0, b_frac = 0;

    if (is_nan(a) || is_nan(b))
    {
        return BF20_PNAN;
    }
    if (is_p_inf(a) && is_p_inf(b))
    {
        return BF20_PINF;
    }
    if (is_m_inf(a) && is_m_inf(b))
    {
        return BF20_MINF;
    }
    if ((is_p_inf(a) && is_m_inf(b)) || (is_p_inf(b) && is_m_inf(a)))
    {
        return BF20_PNAN;
    }
    if (is_p_inf(a) || is_p_inf(b))
    {
        return BF20_PINF;
    }
    if (is_m_inf(a) || is_m_inf(b))
    {
        return BF20_MINF;
    }

    a_sign = extract_sign(a);
    a_exp = extract_exp(a);
    a_E = a_exp == 0 ? 1 - BIAS : a_exp - BIAS;
    a_frac = extract_frac(a);

    b_sign = extract_sign(b);
    b_exp = extract_exp(b);
    b_E = b_exp == 0 ? 1 - BIAS : b_exp - BIAS;
    b_frac = extract_frac(b);

    if (a_exp <= b_exp)
    {
        swap(&a_sign, &b_sign);
        swap(&a_exp, &b_exp);
        swap(&a_E, &b_E);
        swap(&a_frac, &b_frac);
    }

    res_E = a_E;

    // implied 1 if exp is not all zeroes
    if (a_exp != 0)
    {
        a_frac = set_bit(a_frac, FRAC_BITS, 1);
    }
    if (b_exp != 0)
    {
        b_frac = set_bit(b_frac, FRAC_BITS, 1);
    }

    int exp_dif = a_E - b_E;
    // store the end bits "compressed" into index 0, in makes subtraction and addition act
    // as if the entire thing was present
    // so we essentially get a sticky bit and a round bit, but the round bit gets placed automatically
    // bu the exp_dif shift into index 1.
    // but we might lose the small small fractional part of the b_frac, so we "compress" it and store
    // it in the first bit that makes addition and subtraction act as if the entire number was present
    // we check the first exp_dif - 1 bits because the bit at exp_dif is the r bit
    uint stored_s = chk_first_n_bits(b_frac, exp_dif - 1);

    // shift a_frac to make the subtraction and addition act consistent
    uint a_frac_shifted = a_frac << 2;
    // shift b to match floating point calculations
    uint b_frac_shifted = shift_bit_pattern(b_frac, exp_dif - 2);

    // if we had at least one set bit in the first exp_dif - 1 bits
    // we set the 0th bit to 1
    if (stored_s)
    {
        b_frac_shifted = set_bit(b_frac_shifted, 0, 1);
    }

    uint frac_addition_result;

    // print_bits(a_frac);
    // print_bits(b_frac);
    // printf("shifted: (expdif=%d)\n", exp_dif);
    // print_bits(a_frac_shifted);
    // print_bits(b_frac_shifted);

    // if signs are equal, then add their aboslute values together, preserve sign
    if (a_sign == b_sign)
    {
        frac_addition_result = round_to_even(a_frac_shifted + b_frac_shifted, 2) >> 2;
        res_sign = a_sign;

        // printf("initial add:\n");
        // print_bits(frac_addition_result);

        // if bit at 13 is set, overflow occured
        while (chk_bit(frac_addition_result, 13))
        {
            frac_addition_result = round_to_even(frac_addition_result, 1);
            frac_addition_result = frac_addition_result >> 1;
            res_E++;
            if (res_E + BIAS >= 0b00000000000000000000000001111111)
            {
                return res_sign ? BF20_MINF : BF20_PINF;
            }
        }
    }
    // signs are different, subtract the abs value of the number with the lower absolute value
    // from the one with the higher absolute value. sign is the sign of the one with the higher
    // absolute value
    else
    {
        // do the subtraction, round at g = 2 (because of our save method), shift back to correct frac position
        if (a_frac_shifted > b_frac_shifted)
        {
            frac_addition_result = round_to_even(a_frac_shifted - b_frac_shifted, 2) >> 2;
            res_sign = a_sign;
        }
        else if (b_frac_shifted > a_frac_shifted)
        {
            frac_addition_result = round_to_even(b_frac_shifted - a_frac_shifted, 2) >> 2;
            res_sign = b_sign;
        }
        // if they are equal, return +0
        else
        {
            return BF20_PZERO;
        }

        // if bit at 12 is not set, underflow occured
        while (!chk_bit(frac_addition_result, 12))
        {
            frac_addition_result = frac_addition_result << 1;
            res_E--;
            if (res_E + BIAS < 0b00000000000000000000000000000001)
            {
                // exponent is set to 00...00
                // denormalized, set res_E to appropriate value
                // denormalized -> no need to shift anymore -> break
                res_E = 1 - BIAS;
                break;
            }
        }
    }

    // prepare sign for assembly
    res_sign = res_sign ? MASK_SIGN : 0;
    // calculate exp and prepare for assembly
    res_exp = (res_E + BIAS) << FRAC_BITS;
    // prepare frac for assembly by masking off implied 1 if it's there
    res_frac = frac_addition_result & MASK_FRAC;

    // write result
    result |= res_sign;
    result |= res_exp;
    result |= res_frac;

    return result;
}

bf20 bf20_mul(bf20 a, bf20 b)
{
    bf20 result = 0;

    int res_E = 0;
    uint res_sign = 0, res_exp = 0, res_frac = 0;

    int a_E = 0, b_E = 0;
    uint a_sign = 0, a_exp = 0, a_frac = 0;
    uint b_sign = 0, b_exp = 0, b_frac = 0;

    if (is_nan(a) || is_nan(b))
    {
        return BF20_PNAN;
    }
    if (is_p_inf(a) && is_p_inf(b))
    {
        return BF20_PINF;
    }
    if (is_m_inf(a) && is_m_inf(b))
    {
        return BF20_PINF;
    }
    if ((is_p_inf(a) && is_m_inf(b)) || (is_p_inf(b) && is_m_inf(a)))
    {
        return BF20_MINF;
    }

    a_sign = extract_sign(a);
    a_exp = extract_exp(a);
    a_E = a_exp == 0 ? 1 - BIAS : a_exp - BIAS;
    a_frac = extract_frac(a);

    b_sign = extract_sign(b);
    b_exp = extract_exp(b);
    b_E = b_exp == 0 ? 1 - BIAS : b_exp - BIAS;
    b_frac = extract_frac(b);

    if ((is_inf(a) && is_zero(b)) || (is_inf(b) && is_zero(a)))
    {
        return BF20_PNAN;
    }

    // inf and normal value cases
    // +inf
    if ((is_p_inf(a) && !b_sign) || (is_p_inf(b) && !a_sign))
    {
        return BF20_PINF;
    }
    if ((is_p_inf(a) && b_sign) || (is_p_inf(b) && a_sign))
    {
        return BF20_MINF;
    }
    // -inf
    if ((is_m_inf(a) && !b_sign) || (is_m_inf(b) && !a_sign))
    {
        return BF20_MINF;
    }
    if ((is_m_inf(a) && b_sign) || (is_m_inf(b) && a_sign))
    {
        return BF20_PINF;
    }
    // if any of the inputs is zero at this point that means that a normal value
    // is being multiplied by zero. return 0
    if (is_zero(a) || is_zero(b))
    {
        return BF20_PZERO;
    }

    res_sign = a_sign ^ b_sign;
    res_E = a_E + b_E;

    // implied 1 if exp is not all zeroes
    if (a_exp != 0)
    {
        a_frac = set_bit(a_frac, FRAC_BITS, 1);
    }
    if (b_exp != 0)
    {
        b_frac = set_bit(b_frac, FRAC_BITS, 1);
    }

    // printf("before mult:\n");
    // print_bits(a_frac);
    // print_bits(b_frac);
    res_frac = a_frac * b_frac;
    // printf("after mult:\n");
    // print_bits(res_frac);

    int highest_bit = L2R_MAX_INDX - __builtin_clz(res_frac);
    // printf("hb: %d\n", highest_bit);

    int g_indx = highest_bit - FRAC_BITS;
    // printf("g_indx: %d\n", g_indx);
    res_frac = round_to_even(res_frac, g_indx);

    // printf("after round:\n");
    // print_bits(res_frac);

    res_frac = shift_bit_pattern(res_frac, FRAC_BITS);
    // printf("after shift:\n");
    // print_bits(res_frac);

    // if bit at 13 is set, overflow occured
    while (chk_bit(res_frac, 13))
    {
        // printf("overflow!\n");
        // print_bits(res_frac);
        res_frac = round_to_even(res_frac, 1);
        res_frac = res_frac >> 1;
        res_E++;
        if (res_E + BIAS >= 0b00000000000000000000000001111111)
        {
            return res_sign ? BF20_MINF : BF20_PINF;
        }
    }

    // prepare sign for assembly
    res_sign = res_sign ? MASK_SIGN : 0;
    // calculate exp and prepare for assembly
    res_exp = (res_E + BIAS) << FRAC_BITS;
    // prepare frac for assembly by masking off implied 1 if it's there
    res_frac = res_frac & MASK_FRAC;

    // printf("sign:\n");
    // print_bits(res_sign);
    // printf("exp:\n");
    // print_bits(res_exp);
    // printf("frac:\n");
    // print_bits(res_frac);
    // printf("ans:\n");
    // print_bits(0xc867a);

    result |= res_sign;
    result |= res_exp;
    result |= res_frac;

    // print_bits(result);

    return result;
}

int bf20_compare(bf20 a, bf20 b)
{
    int a_E = 0, b_E = 0;
    uint a_sign = 0, a_exp = 0, a_frac = 0;
    uint b_sign = 0, b_exp = 0, b_frac = 0;

    // if nan exit right away
    if (is_nan(a) || is_nan(b))
    {
        return -2;
    }
    if (a == b)
    {
        return 0;
    }
    if (is_zero(a) && is_zero(b))
    {
        return 0;
    }

    a_sign = extract_sign(a);
    a_exp = extract_exp(a);
    a_E = a_exp == 0 ? 1 - BIAS : a_exp - BIAS;
    a_frac = extract_frac(a);

    b_sign = extract_sign(b);
    b_exp = extract_exp(b);
    b_E = b_exp == 0 ? 1 - BIAS : b_exp - BIAS;
    b_frac = extract_frac(b);

    // a is positive, b is negative
    if (!a_sign && b_sign)
    {
        return 1;
    }
    // a is negative, b is positive
    if (a_sign && !b_sign)
    {
        return -1;
    }
    // inf and normal values
    if (is_p_inf(a) && !is_inf(b))
    {
        return 1;
    }
    if (!is_inf(a) && is_p_inf(b))
    {
        return -1;
    }
    if (is_m_inf(a) && !is_inf(b))
    {
        return -1;
    }
    if (!is_inf(a) && is_m_inf(b))
    {
        return 1;
    }

    if (a_exp == b_exp)
    {
        if (a_frac > b_frac)
        {
            // if negative then bigger frac -> smaller number
            return a_sign ? -1 : 1;
        }
        // they cant be equal so a_frac is less than b_frac
        else
        {
            // same but flipped because b is bigger
            return a_sign ? 1 : -1;
        }
    }
    else if (a_exp > b_exp)
    {
        return a_sign ? -1 : 1;
    }
    // a_exp < b_exp
    else
    {
        return a_sign ? 1 : -1;
    }
}
