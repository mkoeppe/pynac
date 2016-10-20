/** @file useries.cpp
 *
 *  Functions for extended truncated univariate power series. */

/*
 *  Copyright (C) 2016  Ralf Stephan <ralf@ark.in-berlin.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __PYNAC_USERIES_CPP__
#define __PYNAC_USERIES_CPP__

#include "useries.h"
#include "useries-flint.h"
#include "add.h"
#include "mul.h"
#include "power.h"
#include "symbol.h"
#include "function.h"
#include "relational.h"
#include "inifcns.h"
#include "utils.h"

#include <unordered_map>

namespace GiNaC {

static bool first_symbol = true;
static symbol symb;

static void check_poly_ccoeff_zero(flint_series_t& fp)
{
        if (fp.offset > 0) {
                fmpq_poly_shift_left(fp.ft, fp.ft, fp.offset);
                fp.offset = 0;
                return;
        }
        else if (fp.offset < 0)
                throw flint_error();
        fmpq_t c;
        fmpq_init(c);
        fmpq_poly_get_coeff_fmpq(c, fp.ft, 0);
        if (not fmpq_is_zero(c))
                throw flint_error();
        fmpq_clear(c);
}

static void check_poly_ccoeff_one(const flint_series_t& fp)
{
        if (fp.offset != 0)
                throw flint_error();
        fmpq_t c;
        fmpq_init(c);
        fmpq_poly_get_coeff_fmpq(c, fp.ft, 0);
        if (not fmpq_is_one(c))
                throw flint_error();
        fmpq_clear(c);
}

long fmpq_poly_ldegree(fmpq_poly_t fp)
{
        long len = fmpq_poly_length(fp);
        for (slong n=0; n<=len; n++) {
                fmpq_t c;
                fmpq_init(c);
                fmpq_poly_get_coeff_fmpq(c, fp, n);
                if (not fmpq_is_zero(c)) {
                        return n;
                }
                fmpq_clear(c);
        }
        return 0;
}

static void exp_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_exp_series(fp.ft, arg.ft, order);
}

static void log_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_one(arg);
        fmpq_poly_log_series(fp.ft, arg.ft, order);
}

static void sin_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_sin_series(fp.ft, arg.ft, order);
}

static void cos_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_cos_series(fp.ft, arg.ft, order);
}

static void tan_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_tan_series(fp.ft, arg.ft, order);
}

static void cot_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_tan_series(fp.ft, arg.ft, order);
        long ldeg = fmpq_poly_ldegree(fp.ft);
        fmpq_poly_shift_right(fp.ft, fp.ft, ldeg);
        fmpq_poly_inv_series(fp.ft, fp.ft, order-ldeg);
        fp.offset = -ldeg;
}

static void sec_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_cos_series(fp.ft, arg.ft, order);
        long ldeg = fmpq_poly_ldegree(fp.ft);
        fmpq_poly_shift_right(fp.ft, fp.ft, ldeg);
        fmpq_poly_inv_series(fp.ft, fp.ft, order-ldeg);
        fp.offset = -ldeg;
}

static void csc_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_sin_series(fp.ft, arg.ft, order);
        long ldeg = fmpq_poly_ldegree(fp.ft);
        fmpq_poly_shift_right(fp.ft, fp.ft, ldeg);
        fmpq_poly_inv_series(fp.ft, fp.ft, order-ldeg);
        fp.offset = -ldeg;
}

static void asin_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_asin_series(fp.ft, arg.ft, order);
}

static void atan_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_atan_series(fp.ft, arg.ft, order);
}

static void sinh_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_sinh_series(fp.ft, arg.ft, order);
}

static void cosh_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_cosh_series(fp.ft, arg.ft, order);
}

static void tanh_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_tanh_series(fp.ft, arg.ft, order);
}

static void coth_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_tanh_series(fp.ft, arg.ft, order);
        long ldeg = fmpq_poly_ldegree(fp.ft);
        fmpq_poly_shift_right(fp.ft, fp.ft, ldeg);
        fmpq_poly_inv_series(fp.ft, fp.ft, order-ldeg);
        fp.offset = -ldeg;
}

static void sech_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_cosh_series(fp.ft, arg.ft, order);
        long ldeg = fmpq_poly_ldegree(fp.ft);
        fmpq_poly_shift_right(fp.ft, fp.ft, ldeg);
        fmpq_poly_inv_series(fp.ft, fp.ft, order-ldeg);
        fp.offset = -ldeg;
}

static void csch_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_sinh_series(fp.ft, arg.ft, order);
        long ldeg = fmpq_poly_ldegree(fp.ft);
        fmpq_poly_shift_right(fp.ft, fp.ft, ldeg);
        fmpq_poly_inv_series(fp.ft, fp.ft, order-ldeg);
        fp.offset = -ldeg;
}

static void asinh_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_asinh_series(fp.ft, arg.ft, order);
}

static void atanh_useries(flint_series_t& fp, flint_series_t& arg, int order)
{
        check_poly_ccoeff_zero(arg);
        fmpq_poly_atanh_series(fp.ft, arg.ft, order);
}

using usfun_t = decltype(exp_useries);
static std::unordered_map<unsigned int,usfun_t*> funcmap {{
        {exp_SERIAL::serial, &exp_useries},
        {log_SERIAL::serial, &log_useries},
        {sin_SERIAL::serial, &sin_useries},
        {cos_SERIAL::serial, &cos_useries},
        {tan_SERIAL::serial, &tan_useries},
        {cot_SERIAL::serial, &cot_useries},
        {sec_SERIAL::serial, &sec_useries},
        {csc_SERIAL::serial, &csc_useries},
        {asin_SERIAL::serial, &asin_useries},
        {atan_SERIAL::serial, &atan_useries},
        {sinh_SERIAL::serial, &sinh_useries},
        {cosh_SERIAL::serial, &cosh_useries},
        {tanh_SERIAL::serial, &tanh_useries},
        {coth_SERIAL::serial, &coth_useries},
        {sech_SERIAL::serial, &sech_useries},
        {csch_SERIAL::serial, &csch_useries},
        {asinh_SERIAL::serial, &asinh_useries},
        {atanh_SERIAL::serial, &atanh_useries},
}};

// Fast heuristic that rejects/accepts expressions for the fast
// expansion via Flint. It can give false positives that must be
// caught before Flint raises SIGABRT, because we want to use the
// older ::series() methods in case. Details:
//
// Does the expression have inexact values, constants, or such?
// It should practically consist of one symbol appearing in
// polynomials from QQ[], and only functions from a supported set.
// The helper uses recurrence to check that all numerics are from QQ,
// that there is not more than one symbol, no constants, and all
// function serial numbers are in the funcmap keys.
static bool unhandled_elements_in(ex the_ex) {

        if (is_exactly_a<constant>(the_ex))
                return true;
        if (is_exactly_a<numeric>(the_ex))
                return not (ex_to<numeric>(the_ex).is_mpz()
                                or ex_to<numeric>(the_ex).is_mpq());
        if (is_exactly_a<symbol>(the_ex)) {
                if (not first_symbol)
                        return (not ex_to<symbol>(the_ex).is_equal(symb));
                first_symbol = false;
                symb = ex_to<symbol>(the_ex);
                return false;
        }
        if (is_exactly_a<function>(the_ex)) {
                function f = ex_to<function>(the_ex);
                if (funcmap.find(f.get_serial()) == funcmap.end())
                        return true;
                for (unsigned int i=0; i<f.nops(); i++)
                        if (unhandled_elements_in(f.op(i)))
                                return true;
                return false;
        }
        if (is_exactly_a<power>(the_ex)) {
                power pow = ex_to<power>(the_ex);
                return (unhandled_elements_in(pow.op(0))
                     or unhandled_elements_in(pow.op(1)));
        }
        if (is_a<expairseq>(the_ex)) {
                const expairseq& epseq = ex_to<expairseq>(the_ex);
                for (unsigned int i=0; i<epseq.nops(); i++) {
                        if (unhandled_elements_in(epseq.op(i)))
                                return true;
                }
                if (unhandled_elements_in(epseq.op(epseq.nops())))
                        return true;
                return false;
        }
        return true;
}

bool useries_can_handle(ex the_ex) {

        return (not unhandled_elements_in(the_ex));
}

ex useries(ex the_ex, const relational & r, int order, unsigned options)
{
        if (order == 0)
                // send residues to the old code
                throw flint_error(); 
        symbol x = ex_to<symbol>(r.lhs());
        flint_series_t fp;
        fmpq_poly_set_ui(fp.ft, 0);
        the_ex.useries(fp, order);
        epvector epv;

        for (slong n=0; n<order; n++) {
                fmpq_t c;
                fmpq_init(c);
                fmpq_poly_get_coeff_fmpq(c, fp.ft, n);
                if (not fmpq_is_zero(c)) {
                        mpq_t gc;
                        mpq_init(gc);
                        fmpq_get_mpq(gc, c);
                        numeric nc(gc); // numeric clears gc
                        epv.push_back(expair(nc, numeric(n + fp.offset)));
                }
                fmpq_clear(c);
        }
        epv.push_back(expair(Order(_ex1), order + fp.offset));
        return pseries(r, epv);
}

void symbol::useries(flint_series_t& fp, int order) const
{
        fmpq_poly_set_str(fp.ft, "1  1");
        fp.offset = 1;
}

void add::useries(flint_series_t& fp, int order) const
{
        fmpq_poly_set_ui(fp.ft, 0);
        for (const auto & elem : seq) {
		const ex& t = recombine_pair_to_ex(elem);
                flint_series_t fp1;
                t.useries(fp1, order);
                if (fp.offset < fp1.offset) {
                        fmpq_poly_shift_left(fp1.ft, fp1.ft, fp1.offset-fp.offset);
                        fp1.offset = fp.offset;
                }
                else if (fp.offset > fp1.offset) {
                        fmpq_poly_shift_left(fp.ft, fp.ft, fp.offset-fp1.offset);
                        fp.offset = fp1.offset;
                }
                fmpq_poly_add(fp.ft, fp.ft, fp1.ft);
        }
        ex ovcoeff = op(nops());
        if (not is_exactly_a<numeric>(ovcoeff))
                throw std::runtime_error("non-numeric oc encountered");
        numeric oc = ex_to<numeric>(ovcoeff);
        if (oc.is_zero())
                return;

        flint_series_t fp1;
        if (oc.is_mpz())
                fmpq_poly_set_mpz(fp1.ft, oc.as_mpz());
        else
                fmpq_poly_set_mpq(fp1.ft, oc.as_mpq());
        fmpq_poly_add(fp.ft, fp.ft, fp1.ft);
}

void mul::useries(flint_series_t& fp, int order) const
{
        fmpq_poly_set_ui(fp.ft, 1);
        for (const auto & elem : seq) {
		const ex& t = recombine_pair_to_ex(elem);
                flint_series_t fp1;
                t.useries(fp1, order);
                long newoff = fp1.offset + fp.offset;
                fmpq_poly_mullow(fp.ft, fp.ft, fp1.ft, order+2);
                fp.offset = newoff;
        }
        ex ovcoeff = op(nops());
        if (not is_exactly_a<numeric>(ovcoeff))
                throw std::runtime_error("non-numeric oc encountered");
        numeric oc = ex_to<numeric>(ovcoeff);
        if (oc.is_equal(*_num1_p))
                return;

        if (oc.is_mpz())
                fmpq_poly_scalar_mul_mpz(fp.ft, fp.ft, oc.as_mpz());
        else
                fmpq_poly_scalar_mul_mpq(fp.ft, fp.ft, oc.as_mpq());
}

void power::useries(flint_series_t& fp, int order) const
{
        flint_series_t fp1;
        basis.useries(fp1, order);
        if (not is_exactly_a<numeric>(exponent)) {
                fmpq_poly_log_series(fp1.ft, fp1.ft, order);
                exponent.useries(fp, order);
                fmpq_poly_mullow(fp.ft, fp.ft, fp1.ft, order+2);
                fmpq_poly_exp_series(fp.ft, fp.ft, order);
                return;
        }
        numeric nexp = ex_to<numeric>(exponent);
        if (nexp.is_mpq()) {
                int num = nexp.numer().to_int();
                int den = nexp.denom().to_int();
                if (den == 2) { // exponent of form n/2
                        fmpq_t c;
                        fmpq_init(c);
                        fmpq_poly_get_coeff_fmpq(c, fp1.ft, 0);
                        mpz_t cnum, cden;
                        mpz_init(cnum);
                        mpz_init(cden);
                        fmpq_get_mpz_frac(cnum, cden, c);
                        if (not mpz_perfect_square_p(cnum)
                            or not mpz_perfect_square_p(cden))
                                throw flint_error();
                        mpz_sqrt(cnum, cnum);
                        mpz_sqrt(cden, cden);
                        fmpq_t cc;
                        fmpq_init_set_mpz_frac_readonly(cc, cnum, cden);
                        mpz_clear(cnum);
                        mpz_clear(cden);

                        fmpq_poly_scalar_div_fmpq(fp1.ft, fp1.ft, c);
                        fmpq_poly_sqrt_series(fp1.ft, fp1.ft, order);
                        fmpq_poly_scalar_mul_fmpq(fp1.ft, fp1.ft, cc);
                        if (num > 0)
                                fmpq_poly_pow(fp.ft, fp1.ft, num);
                        else {
                                fmpq_poly_inv_series(fp1.ft, fp1.ft, order);
                                fmpq_poly_pow(fp.ft, fp1.ft, -num);
                        }
                        fmpq_clear(c);
                        return;
                }
                fmpq_poly_log_series(fp1.ft, fp1.ft, order);
                fmpq_poly_scalar_mul_mpq(fp1.ft, fp1.ft, nexp.as_mpq());
                fmpq_poly_exp_series(fp.ft, fp1.ft, order);
                return;
        }
        // integer exponent
        int expint = nexp.to_int();
        long ldeg = fmpq_poly_ldegree(fp1.ft);
        if (expint > 0) {
                fmpq_poly_pow(fp.ft, fp1.ft, expint);
                fp.offset = fp1.offset * expint;
                return;
        }
        else if (expint < 0) {
                if (ldeg) {
                        fmpq_poly_shift_right(fp1.ft, fp1.ft, ldeg);
                        fp1.offset = ldeg;
                }
                fmpq_poly_inv_series(fp1.ft, fp1.ft, order - fp1.offset*expint);
                fmpq_poly_pow(fp.ft, fp1.ft, -expint);
                fp.offset = fp1.offset * expint;
                return;
        }
        fmpq_poly_set_str(fp.ft, "1 1");
}

void function::useries(flint_series_t& fp, int order) const
{
        auto search = funcmap.find(serial);
        if (search == funcmap.end())
                throw std::runtime_error("can't happen in function::useries");
        flint_series_t fp1;
        seq[0].useries(fp1, order);
        (*search->second)(fp, fp1, order);
}

void numeric::useries(flint_series_t& fp, int order) const
{
        if (is_mpz())
                fmpq_poly_set_mpz(fp.ft, as_mpz());
        else
                fmpq_poly_set_mpq(fp.ft, as_mpq());
}

} // namespace GiNaC

#endif // undef __PYNAC_USERIES_CPP__