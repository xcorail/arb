/*
    Copyright (C) 2018 Fredrik Johansson

    This file is part of Arb.

    Arb is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "arb_mat.h"

static void
arb_approx_set(arb_t z, const arb_t x)
{
    arf_set(arb_midref(z), arb_midref(x));
}

static void
arb_approx_sub(arb_t z, const arb_t x, const arb_t y, slong prec)
{
    arf_sub(arb_midref(z),
               arb_midref(x), arb_midref(y), prec, ARF_RND_DOWN);
}

static void
arb_approx_addmul(arb_t z, const arb_t x, const arb_t y, slong prec)
{
    arf_addmul(arb_midref(z),
               arb_midref(x), arb_midref(y), prec, ARF_RND_DOWN);
}

static void
arb_approx_div(arb_t z, const arb_t x, const arb_t y, slong prec)
{
    arf_div(arb_midref(z), arb_midref(x), arb_midref(y), prec, ARB_RND);
}

void
arb_mat_approx_solve_tril_classical(arb_mat_t X,
        const arb_mat_t L, const arb_mat_t B, int unit, slong prec)
{
    slong i, j, k, n, m;
    arb_ptr tmp;
    arb_t s;

    n = L->r;
    m = B->c;

    arb_init(s);
    tmp = _arb_vec_init(n);

    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
            arb_approx_set(tmp + j, arb_mat_entry(X, j, i));

        for (j = 0; j < n; j++)
        {
            arb_zero(s);
            for (k = 0; k < j; k++)
                arb_approx_addmul(s, L->rows[j] + k, tmp + k, prec);
            arb_approx_sub(s, arb_mat_entry(B, j, i), s, prec);
            if (!unit)
                arb_approx_div(s, s, arb_mat_entry(L, j, j), prec);
            arb_approx_set(tmp + j, s);
        }

        for (j = 0; j < n; j++)
            arb_approx_set(arb_mat_entry(X, j, i), tmp + j);
    }

    _arb_vec_clear(tmp, n);
    arb_clear(s);
}

void
arb_mat_approx_solve_tril_recursive(arb_mat_t X,
        const arb_mat_t L, const arb_mat_t B, int unit, slong prec)
{
    arb_mat_t LA, LC, LD, XX, XY, BX, BY, T;
    slong r, n, m;

    n = L->r;
    m = B->c;
    r = n / 2;

    if (n == 0 || m == 0)
        return;

    /*
    Denoting inv(M) by M^, we have:

    [A 0]^ [X]  ==  [A^          0 ] [X]  ==  [A^ X]
    [C D]  [Y]  ==  [-D^ C A^    D^] [Y]  ==  [D^ (Y - C A^ X)]
    */
    arb_mat_window_init(LA, L, 0, 0, r, r);
    arb_mat_window_init(LC, L, r, 0, n, r);
    arb_mat_window_init(LD, L, r, r, n, n);
    arb_mat_window_init(BX, B, 0, 0, r, m);
    arb_mat_window_init(BY, B, r, 0, n, m);
    arb_mat_window_init(XX, X, 0, 0, r, m);
    arb_mat_window_init(XY, X, r, 0, n, m);

    arb_mat_approx_solve_tril(XX, LA, BX, unit, prec);

    /* arb_mat_submul(XY, BY, LC, XX); */
    arb_mat_init(T, LC->r, BX->c);
    arb_mat_mul(T, LC, XX, prec);
    arb_mat_get_mid(T, T);
    arb_mat_sub(XY, BY, T, prec);
    arb_mat_get_mid(XY, XY);
    arb_mat_clear(T);

    arb_mat_approx_solve_tril(XY, LD, XY, unit, prec);

    arb_mat_window_clear(LA);
    arb_mat_window_clear(LC);
    arb_mat_window_clear(LD);
    arb_mat_window_clear(BX);
    arb_mat_window_clear(BY);
    arb_mat_window_clear(XX);
    arb_mat_window_clear(XY);
}

void
arb_mat_approx_solve_tril(arb_mat_t X, const arb_mat_t L,
                                    const arb_mat_t B, int unit, slong prec)
{
    if (B->r < 8 || B->c < 8)
        arb_mat_approx_solve_tril_classical(X, L, B, unit, prec);
    else
        arb_mat_approx_solve_tril_recursive(X, L, B, unit, prec);
}
