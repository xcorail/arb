/*
    Copyright (C) 2018 Fredrik Johansson

    This file is part of Arb.

    Arb is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "arb_mat.h"

void
arb_mat_solve_triu_classical(arb_mat_t X, const arb_mat_t U,
    const arb_mat_t B, int unit, slong prec)
{
    slong i, j, k, n, m;
    arb_ptr tmp;
    arb_t s;

    n = U->r;
    m = B->c;

    tmp = _arb_vec_init(n);
    arb_init(s);

    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
            arb_set(tmp + j, arb_mat_entry(X, j, i));

        for (j = n - 1; j >= 0; j--)
        {
            arb_zero(s);
            for (k = 0; k < n - j - 1; k++)
                arb_addmul(s, U->rows[j] + j + 1 + k, tmp + j + 1 + k, prec);

            arb_sub(s, arb_mat_entry(B, j, i), s, prec);
            if (!unit)
                arb_div(s, s, arb_mat_entry(U, j, j), prec);

            arb_set(tmp + j, s);
        }

        for (j = 0; j < n; j++)
            arb_set(arb_mat_entry(X, j, i), tmp + j);
    }

    _arb_vec_clear(tmp, n);
    arb_clear(s);
}

void
arb_mat_solve_triu_recursive(arb_mat_t X,
        const arb_mat_t U, const arb_mat_t B, int unit, slong prec)
{
    arb_mat_t UA, UB, UD, XX, XY, BX, BY, T;
    slong r, n, m;

    n = U->r;
    m = B->c;
    r = n / 2;

    if (n == 0 || m == 0)
        return;

    /*
    Denoting inv(M) by M^, we have:
    [A B]^ [X]  ==  [A^ (X - B D^ Y)]
    [0 D]  [Y]  ==  [    D^ Y       ]
    */

    arb_mat_window_init(UA, U, 0, 0, r, r);
    arb_mat_window_init(UB, U, 0, r, r, n);
    arb_mat_window_init(UD, U, r, r, n, n);
    arb_mat_window_init(BX, B, 0, 0, r, m);
    arb_mat_window_init(BY, B, r, 0, n, m);
    arb_mat_window_init(XX, X, 0, 0, r, m);
    arb_mat_window_init(XY, X, r, 0, n, m);

    arb_mat_solve_triu(XY, UD, BY, unit, prec);

    /* arb_mat_submul(XX, BX, UB, XY); */
    arb_mat_init(T, UB->r, XY->c);
    arb_mat_mul(T, UB, XY, prec);
    arb_mat_sub(XX, BX, T, prec);
    arb_mat_clear(T);

    arb_mat_solve_triu(XX, UA, XX, unit, prec);

    arb_mat_window_clear(UA);
    arb_mat_window_clear(UB);
    arb_mat_window_clear(UD);
    arb_mat_window_clear(BX);
    arb_mat_window_clear(BY);
    arb_mat_window_clear(XX);
    arb_mat_window_clear(XY);
}

void
arb_mat_solve_triu(arb_mat_t X, const arb_mat_t U,
                                    const arb_mat_t B, int unit, slong prec)
{
    if (B->r < 8 || B->c < 8)
        arb_mat_solve_triu_classical(X, U, B, unit, prec);
    else
        arb_mat_solve_triu_recursive(X, U, B, unit, prec);
}

