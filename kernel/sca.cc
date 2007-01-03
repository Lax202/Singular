/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/***************************************************************
 *  File:    sca.cc
 *  Purpose: supercommutative kernel procedures
 *  Author:  motsak (Oleksandr Motsak)
 *  Created: 2006/12/18
 *  Version: $Id: sca.cc,v 1.1 2007-01-03 00:04:00 motsak Exp $
 *******************************************************************/

// #define PDEBUG 2
#include "mod2.h"

// for
#define PLURAL_INTERNAL_DECLARATIONS
#include "sca.h"


#include "febase.h"

#include "p_polys.h"
#include "kutil.h"
#include "ideals.h"
#include "intvec.h"
#include "polys.h"

#include "ring.h"
#include "numbers.h"
#include "matpol.h"
#include "kbuckets.h"
#include "kstd1.h"
#include "sbuckets.h"
#include "prCopy.h"
#include "p_Mult_q.h"
#include "p_MemAdd.h"

#include "kutil.h"
#include "kstd1.h"

#include "weight.h"


//   scaFirstAltVar( r, 0 );
//   scaLastAltVar( r, 0 );



////////////////////////////////////////////////////////////////////////////////////////////////////
// Super Commutative Algabra extension by Motsak
////////////////////////////////////////////////////////////////////////////////////////////////////





// returns the sign of: lm(pMonomM) * lm(pMonomMM),
// preserves input, may return +/-1, 0
inline int sca_Sign_mm_Mult_mm( const poly pMonomM, const poly pMonomMM, const ring rRing )
{
#ifdef PDEBUG
    p_Test(pMonomM,  rRing);
    p_Test(pMonomMM, rRing);
#endif

    const unsigned int iFirstAltVar = scaFirstAltVar(rRing);
    const unsigned int iLastAltVar  = scaLastAltVar(rRing);

    unsigned int tpower = 0;
    unsigned int cpower = 0;

    for( unsigned int j = iLastAltVar; j >= iFirstAltVar; j-- )
    {
      const unsigned int iExpM  = p_GetExp(pMonomM,  j, rRing);
      const unsigned int iExpMM = p_GetExp(pMonomMM, j, rRing);

      if( iExpMM != 0 )
      {
        if( iExpM != 0 )
        {
          return 0; // lm(pMonomM) * lm(pMonomMM) == 0
        }
        tpower += cpower; // compute degree of (-1).
      }

      cpower += iExpM;
    }

    if( (tpower&1) != 0 ) // degree is odd => negate coeff.
      return -1;

    return(1);
}




// returns and changes pMonomM: lt(pMonomM) = lt(pMonomM) * lt(pMonomMM),
// preserves pMonomMM. may return NULL!
// if result != NULL => next(result) = next(pMonomM), lt(result) = lt(pMonomM) * lt(pMonomMM)
// if result == NULL => pMonomM MUST BE deleted manually!
inline poly sca_m_Mult_mm( poly pMonomM, const poly pMonomMM, const ring rRing )
{
#ifdef PDEBUG
    p_Test(pMonomM,  rRing);
    p_Test(pMonomMM, rRing);
#endif

    const unsigned int iFirstAltVar = scaFirstAltVar(rRing);
    const unsigned int iLastAltVar = scaLastAltVar(rRing);

    unsigned int tpower = 0;
    unsigned int cpower = 0;

    for( unsigned int j = iLastAltVar; j >= iFirstAltVar; j-- )
    {
      const unsigned int iExpM  = p_GetExp(pMonomM,  j, rRing);
      const unsigned int iExpMM = p_GetExp(pMonomMM, j, rRing);

      if( iExpMM != 0 )
      {
        if( iExpM != 0 ) // result is zero!
        {
          return NULL; // we do nothing with pMonomM in this case!
        }

        tpower += cpower; // compute degree of (-1).
      }

      cpower += iExpM;
    }

    p_ExpVectorAdd(pMonomM, pMonomMM, rRing); // "exponents" are additive!!!

    number nCoeffM = p_GetCoeff(pMonomM, rRing); // no new copy! should be deleted!

    if( (tpower&1) != 0 ) // degree is odd => negate coeff.
      nCoeffM = n_Neg(nCoeffM, rRing); // negate nCoeff (will destroy the original number)

    const number nCoeffMM = p_GetCoeff(pMonomMM, rRing); // no new copy!

    number nCoeff = n_Mult(nCoeffM, nCoeffMM, rRing); // new number!

    p_SetCoeff(pMonomM, nCoeff, rRing); // delete lc(pMonomM) and set lc(pMonomM) = nCoeff

#ifdef PDEBUG
    p_Test(pMonomM, rRing);
#endif

    return(pMonomM);
}

// returns and changes pMonomM: lt(pMonomM) = lt(pMonomMM) * lt(pMonomM),
// preserves pMonomMM. may return NULL!
// if result != NULL => next(result) = next(pMonomM), lt(result) = lt(pMonomMM) * lt(pMonomM)
// if result == NULL => pMonomM MUST BE deleted manually!
inline poly sca_mm_Mult_m( const poly pMonomMM, poly pMonomM, const ring rRing )
{
#ifdef PDEBUG
    p_Test(pMonomM,  rRing);
    p_Test(pMonomMM, rRing);
#endif

    const unsigned int iFirstAltVar = scaFirstAltVar(rRing);
    const unsigned int iLastAltVar = scaLastAltVar(rRing);

    unsigned int tpower = 0;
    unsigned int cpower = 0;

    for( unsigned int j = iLastAltVar; j >= iFirstAltVar; j-- )
    {
      const unsigned int iExpMM = p_GetExp(pMonomMM, j, rRing);
      const unsigned int iExpM  = p_GetExp(pMonomM,  j, rRing);

      if( iExpM != 0 )
      {
        if( iExpMM != 0 ) // result is zero!
        {
          return NULL; // we do nothing with pMonomM in this case!
        }

        tpower += cpower; // compute degree of (-1).
      }

      cpower += iExpMM;
    }

    p_ExpVectorAdd(pMonomM, pMonomMM, rRing); // "exponents" are additive!!!

    number nCoeffM = p_GetCoeff(pMonomM, rRing); // no new copy! should be deleted!

    if( (tpower&1) != 0 ) // degree is odd => negate coeff.
      nCoeffM = n_Neg(nCoeffM, rRing); // negate nCoeff (will destroy the original number), creates new number!

    const number nCoeffMM = p_GetCoeff(pMonomMM, rRing); // no new copy!

    number nCoeff = n_Mult(nCoeffM, nCoeffMM, rRing); // new number!

    p_SetCoeff(pMonomM, nCoeff, rRing); // delete lc(pMonomM) and set lc(pMonomM) = nCoeff

#ifdef PDEBUG
    p_Test(pMonomM, rRing);
#endif

    return(pMonomM);
}



// returns: result = lt(pMonom1) * lt(pMonom2),
// preserves pMonom1, pMonom2. may return NULL!
// if result != NULL => next(result) = NULL, lt(result) = lt(pMonom1) * lt(pMonom2)
inline poly sca_mm_Mult_mm( poly pMonom1, const poly pMonom2, const ring rRing )
{
#ifdef PDEBUG
    p_Test(pMonom1, rRing);
    p_Test(pMonom2, rRing);
#endif

    const unsigned int iFirstAltVar = scaFirstAltVar(rRing);
    const unsigned int iLastAltVar = scaLastAltVar(rRing);

    unsigned int tpower = 0;
    unsigned int cpower = 0;

    for( unsigned int j = iLastAltVar; j >= iFirstAltVar; j-- )
    {
      const unsigned int iExp1 = p_GetExp(pMonom1, j, rRing);
      const unsigned int iExp2 = p_GetExp(pMonom2, j, rRing);

      if( iExp2 != 0 )
      {
        if( iExp1 != 0 ) // result is zero!
        {
          return NULL;
        }

        tpower += cpower; // compute degree of (-1).
      }

      cpower += iExp1;
    }

    poly pResult;
    omTypeAllocBin(poly, pResult, rRing->PolyBin);
    p_SetRingOfLm(pResult, rRing);

    p_ExpVectorSum(pResult, pMonom1, pMonom2, rRing); // "exponents" are additive!!!

    pNext(pResult) = NULL;

    const number nCoeff1 = p_GetCoeff(pMonom1, rRing); // no new copy!
    const number nCoeff2 = p_GetCoeff(pMonom2, rRing); // no new copy!

    number nCoeff = n_Mult(nCoeff1, nCoeff2, rRing); // new number!

    if( (tpower&1) != 0 ) // degree is odd => negate coeff.
      nCoeff = n_Neg(nCoeff, rRing); // negate nCoeff (will destroy the original number)

    p_SetCoeff0(pResult, nCoeff, rRing); // set lc(pResult) = nCoeff, no destruction!

#ifdef PDEBUG
    p_Test(pResult, rRing);
#endif

    return(pResult);
}

// returns: result =  x_i * lt(pMonom),
// preserves pMonom. may return NULL!
// if result != NULL => next(result) = NULL, lt(result) = x_i * lt(pMonom)
inline poly sca_xi_Mult_mm(unsigned int i, const poly pMonom, const ring rRing)
{
#ifdef PDEBUG
    p_Test(pMonom, rRing);
#endif

    assume( i <= scaLastAltVar(rRing));
    assume( scaFirstAltVar(rRing) <= i );

    if( p_GetExp(pMonom, i, rRing) ) // result is zero!
      return NULL;

    const unsigned int iFirstAltVar = scaFirstAltVar(rRing);

    unsigned int cpower = 0;

    for( unsigned int j = iFirstAltVar; j < i ; j++ )
      cpower += p_GetExp(pMonom, j, rRing);

    poly pResult = p_LmInit(pMonom, rRing);

    p_SetExp(pResult, i, 1, rRing); // pResult*=X_i &&
    p_Setm(pResult, rRing);         // addjust degree after previous step!

    number nCoeff = n_Copy(p_GetCoeff(pMonom, rRing), rRing); // new number!

    if( (cpower&1) != 0 ) // degree is odd => negate coeff.
      nCoeff = n_Neg(nCoeff, rRing); // negate nCoeff (will destroy the original number)

    p_SetCoeff0(pResult, nCoeff, rRing); // set lc(pResult) = nCoeff, no destruction!

#ifdef PDEBUG
    p_Test(pResult, rRing);
#endif

    return(pResult);
}

//-----------------------------------------------------------------------------------//

// return poly = pPoly * pMonom; preserve pMonom, destroy or reuse pPoly.
poly sca_p_Mult_mm(poly pPoly, const poly pMonom, const ring rRing)
{
  assume( rIsSCA(rRing) );

#ifdef PDEBUG
//  Print("sca_p_Mult_mm\n"); // !

  p_Test(pPoly, rRing);
  p_Test(pMonom, rRing);
#endif

  if( pPoly == NULL )
    return NULL;

  if( pMonom == NULL )
  {
    // pPoly != NULL =>
    p_Delete( &pPoly, rRing );

    return NULL;
  }

  const int iComponentMonomM = p_GetComp(pMonom, rRing);

  poly p = pPoly; poly* ppPrev = &pPoly;

  loop
  {
#ifdef PDEBUG
    p_Test(p, rRing);
#endif
    const int iComponent = p_GetComp(p, rRing);

    if( iComponent!=0 )
    {
      if( iComponentMonomM!=0 ) // TODO: make global if on iComponentMonomM =?= 0
      {
        // REPORT_ERROR
        Werror("sca_p_Mult_mm: exponent mismatch %d and %d\n", iComponent, iComponentMonomM);
        // what should we do further?!?

        p_Delete( &pPoly, rRing); // delete the result AND rest
        return NULL;
      }
#ifdef PDEBUG
      if(iComponentMonomM==0 )
      {
        Print("Multiplication in the left module from the right");
      }
#endif
    }

    // terms will be in the same order because of quasi-ordering!
    poly v = sca_m_Mult_mm(p, pMonom, rRing);

    if( v != NULL )
    {
      ppPrev = &pNext(p); // fixed!

    // *p is changed if v != NULL ( p == v )
      pIter(p);

      if( p == NULL )
        break;
    }
    else
    { // Upps! Zero!!! we must kill this term!

      //
      p = p_LmDeleteAndNext(p, rRing);

      *ppPrev = p;

      if( p == NULL )
        break;
    }


  }

#ifdef PDEBUG
  p_Test(pPoly,rRing);
#endif

  return(pPoly);
}

// return new poly = pPoly * pMonom; preserve pPoly and pMonom.
poly sca_pp_Mult_mm(const poly pPoly, const poly pMonom, const ring rRing, poly &)
{
  assume( rIsSCA(rRing) );

#ifdef PDEBUG
//  Print("sca_pp_Mult_mm\n"); // !

  p_Test(pPoly, rRing);
  p_Test(pMonom, rRing);
#endif

  if( ( pPoly == NULL ) || ( pMonom == NULL ) )
    return NULL;

  const int iComponentMonomM = p_GetComp(pMonom, rRing);

  poly pResult = NULL;
  poly* ppPrev = &pResult;

  for( poly p = pPoly; p!= NULL; pIter(p) )
  {
#ifdef PDEBUG
    p_Test(p, rRing);
#endif
    const int iComponent = p_GetComp(p, rRing);

    if( iComponent!=0 )
    {
      if( iComponentMonomM!=0 ) // TODO: make global if on iComponentMonomM =?= 0
      {
        // REPORT_ERROR
        Werror("sca_pp_Mult_mm: exponent mismatch %d and %d\n", iComponent, iComponentMonomM);
        // what should we do further?!?

        p_Delete( &pResult, rRing); // delete the result
        return NULL;
      }

#ifdef PDEBUG
      if(iComponentMonomM==0 )
      {
        Print("Multiplication in the left module from the right");
      }
#endif
    }

    // terms will be in the same order because of quasi-ordering!
    poly v = sca_mm_Mult_mm(p, pMonom, rRing);

    if( v != NULL )
    {
      *ppPrev = v;
      ppPrev = &pNext(v);
    }

  }

#ifdef PDEBUG
  p_Test(pResult,rRing);
#endif

  return(pResult);
}

//-----------------------------------------------------------------------------------//

// return x_i * pPoly; preserve pPoly.
inline poly sca_xi_Mult_pp(unsigned int i, const poly pPoly, const ring rRing)
{
  assume( rIsSCA(rRing) );

#ifdef PDEBUG
  p_Test(pPoly, rRing);
#endif

  assume(i <= scaLastAltVar(rRing));
  assume(scaFirstAltVar(rRing) <= i);

  if( pPoly == NULL )
    return NULL;

  poly pResult = NULL;
  poly* ppPrev = &pResult;

  for( poly p = pPoly; p!= NULL; pIter(p) )
  {

    // terms will be in the same order because of quasi-ordering!
    poly v = sca_xi_Mult_mm(i, p, rRing);

#ifdef PDEBUG
    p_Test(v, rRing);
#endif

    if( v != NULL )
    {
      *ppPrev = v;
      ppPrev = &pNext(*ppPrev);
    }
  }


#ifdef PDEBUG
  p_Test(pResult, rRing);
#endif

  return(pResult);
}


// return new poly = pMonom * pPoly; preserve pPoly and pMonom.
poly sca_mm_Mult_pp(const poly pMonom, const poly pPoly, const ring rRing)
{
  assume( rIsSCA(rRing) );

#ifdef PDEBUG
//  Print("sca_mm_Mult_pp\n"); // !

  p_Test(pPoly, rRing);
  p_Test(pMonom, rRing);
#endif

  if( ( pPoly == NULL ) || ( pMonom == NULL ) )
    return NULL;

  const int iComponentMonomM = p_GetComp(pMonom, rRing);

  poly pResult = NULL;
  poly* ppPrev = &pResult;

  for( poly p = pPoly; p!= NULL; pIter(p) )
  {
#ifdef PDEBUG
    p_Test(p, rRing);
#endif
    const int iComponent = p_GetComp(p, rRing);

    if( iComponentMonomM!=0 )
    {
      if( iComponent!=0 ) // TODO: make global if on iComponentMonomM =?= 0
      {
        // REPORT_ERROR
        Werror("sca_mm_Mult_pp: exponent mismatch %d and %d\n", iComponent, iComponentMonomM);
        // what should we do further?!?

        p_Delete( &pResult, rRing); // delete the result
        return NULL;
      }
#ifdef PDEBUG
      if(iComponent==0 )
      {
        Print("Multiplication in the left module from the right");
      }
#endif
    }

    // terms will be in the same order because of quasi-ordering!
    poly v = sca_mm_Mult_mm(pMonom, p, rRing);

    if( v != NULL )
    {
      *ppPrev = v;
      ppPrev = &pNext(*ppPrev); // nice line ;-)
    }
  }

#ifdef PDEBUG
  p_Test(pResult,rRing);
#endif

  return(pResult);
}


// return poly = pMonom * pPoly; preserve pMonom, destroy or reuse pPoly.
poly sca_mm_Mult_p(const poly pMonom, poly pPoly, const ring rRing) // !!!!! the MOST used procedure !!!!!
{
  assume( rIsSCA(rRing) );

#ifdef PDEBUG
  p_Test(pPoly, rRing);
  p_Test(pMonom, rRing);
#endif

  if( pPoly == NULL )
    return NULL;

  if( pMonom == NULL )
  {
    // pPoly != NULL =>
    p_Delete( &pPoly, rRing );
    return NULL;
  }

  const int iComponentMonomM = p_GetComp(pMonom, rRing);

  poly p = pPoly; poly* ppPrev = &pPoly;

  loop
  {
#ifdef PDEBUG
    if( !p_Test(p, rRing) )
    {
      Print("p is wrong!");
      p_Write(p,rRing);
    }
#endif

    const int iComponent = p_GetComp(p, rRing);

    if( iComponentMonomM!=0 )
    {
      if( iComponent!=0 ) // TODO: make global if on iComponentMonomM =?= 0
      {
        // REPORT_ERROR
        Werror("sca_mm_Mult_p: exponent mismatch %d and %d\n", iComponent, iComponentMonomM);
        // what should we do further?!?

        p_Delete( &pPoly, rRing); // delete the result
        return NULL;
      }
#ifdef PDEBUG
      if(iComponent==0 )
      {
        Print("Multiplication in the left module from the right");
      }
#endif
    }

    // terms will be in the same order because of quasi-ordering!
    poly v = sca_mm_Mult_m(pMonom, p, rRing);

    if( v != NULL )
    {
      ppPrev = &pNext(p);

    // *p is changed if v != NULL ( p == v )
      pIter(p);

      if( p == NULL )
        break;
    }
    else
    { // Upps! Zero!!! we must kill this term!
      p = p_LmDeleteAndNext(p, rRing);

      *ppPrev = p;

      if( p == NULL )
        break;
    }
  }

#ifdef PDEBUG
    if( !p_Test(pPoly, rRing) )
    {
      Print("pPoly is wrong!");
      p_Write(pPoly, rRing);
    }
#endif

  return(pPoly);
}

//-----------------------------------------------------------------------------------//

#ifdef PDEBUG
#endif




//-----------------------------------------------------------------------------------//

// GB computation routines:

/*2
* returns the LCM of the head terms of a and b
*/
inline poly p_Lcm(const poly a, const poly b, const long lCompM, const ring r)
{
  poly m = p_ISet(1, r);

  const int pVariables = r->N;

  for (int i = pVariables; i; i--)
  {
    const int lExpA = p_GetExp (a, i, r);
    const int lExpB = p_GetExp (b, i, r);

    p_SetExp (m, i, si_max(lExpA, lExpB), r);
  }

  p_SetComp (m, lCompM, r);

  p_Setm(m,r);

#ifdef PDEBUG
  p_Test(m,r);
#endif

  return(m);
}

/*4
* creates the S-polynomial of p1 and p2
* does not destroy p1 and p2
*/
poly sca_SPoly( const poly p1, const poly p2, const ring r )
{
  assume( rIsSCA(r) );

  const long lCompP1 = p_GetComp(p1,r);
  const long lCompP2 = p_GetComp(p2,r);

  if ((lCompP1!=lCompP2) && (lCompP1!=0) && (lCompP2!=0))
  {
#ifdef PDEBUG
    Print("sca_SPoly: different non-zero components!\n");
#endif
    return(NULL);
  }

  poly pL = p_Lcm(p1, p2, si_max(lCompP1, lCompP2), r);       // pL = lcm( lm(p1), lm(p2) )

  poly m1 = p_ISet(1, r);
  p_ExpVectorDiff(m1, pL, p1, r);                  // m1 = pL / lm(p1)
  //p_SetComp(m1,0,r);
  //p_Setm(m1,r);
#ifdef PDEBUG
  p_Test(m1,r);
#endif


  poly m2 = p_ISet(1, r);
  p_ExpVectorDiff (m2, pL, p2, r);                  // m2 = pL / lm(p2)

  //p_SetComp(m2,0,r);
  //p_Setm(m2,r);
#ifdef PDEBUG
  p_Test(m2,r);
#endif

  p_Delete(&pL,r);

  number C1  = n_Copy(p_GetCoeff(p1,r),r);      // C1 = lc(p1)
  number C2  = n_Copy(p_GetCoeff(p2,r),r);      // C2 = lc(p2)

  number C = nGcd(C1,C2,r);                     // C = gcd(C1, C2)

  if (!n_IsOne(C, r))                              // if C != 1
  {
    C1=n_Div(C1, C, r);                              // C1 = C1 / C
    C2=n_Div(C2, C, r);                              // C2 = C2 / C
  }

  n_Delete(&C,r); // destroy the number C

  const int iSignSum = sca_Sign_mm_Mult_mm (m1, p1, r) + sca_Sign_mm_Mult_mm (m2, p2, r);
  // zero if different signs

  assume( (iSignSum*iSignSum == 0) || (iSignSum*iSignSum == 4) );

  if( iSignSum != 0 ) // the same sign!
    C2=n_Neg (C2, r);

  p_SetCoeff(m1, C2, r);                           // lc(m1) = C2!!!
  p_SetCoeff(m2, C1, r);                           // lc(m2) = C1!!!

  poly tmp1 = mm_Mult_pp (m1, pNext(p1), r);    // tmp1 = m1 * tail(p1),
  p_Delete(&m1,r);  //  => n_Delete(&C1,r);

  poly tmp2 = mm_Mult_pp (m2, pNext(p2), r);    // tmp1 = m2 * tail(p2),
  p_Delete(&m2,r);  //  => n_Delete(&C1,r);

  poly spoly = p_Add_q (tmp1, tmp2, r); // spoly = spoly(lt(p1), lt(p2)) + m1 * tail(p1), delete tmp1,2

  if (spoly!=NULL) pCleardenom (spoly); // r?
//  if (spoly!=NULL) pContent (spoly); // r?

#ifdef PDEBUG
  p_Test (spoly, r);
#endif

  return(spoly);
}




/*2
* reduction of p2 with p1
* do not destroy p1, but p2
* p1 divides p2 -> for use in NF algorithm
*/
poly sca_ReduceSpoly(const poly p1, poly p2, const ring r)
{
  assume( rIsSCA(r) );

  assume( p1 != NULL );

  const long lCompP1 = p_GetComp (p1, r);
  const long lCompP2 = p_GetComp (p2, r);

  if ((lCompP1!=lCompP2) && (lCompP1!=0) && (lCompP2!=0))
  {
#ifdef PDEBUG
    Print("sca_ReduceSpoly: different non-zero components!");
#endif
    return(NULL);
  }

  poly m = p_ISet (1, r);
  p_ExpVectorDiff (m, p2, p1, r);                      // m = lm(p2) / lm(p1)
  //p_Setm(m,r);
#ifdef PDEBUG
  p_Test (m,r);
#endif

  number C1 = n_Copy( p_GetCoeff(p1, r), r);
  number C2 = n_Copy( p_GetCoeff(p2, r), r);

  /* GCD stuff */
  number C = nGcd(C1, C2, r);

  if (!n_IsOne(C, r))
  {
    C1 = n_Div(C1, C, r);
    C2 = n_Div(C2, C, r);
  }
  n_Delete(&C,r);

  const int iSign = sca_Sign_mm_Mult_mm( m, p1, r );

  if(iSign == 1)
    C2 = n_Neg(C2,r);

  p_SetCoeff(m, C2, r);

#ifdef PDEBUG
  p_Test(m,r);
#endif

  p2 = p_LmDeleteAndNext( p2, r );

  p2 = p_Mult_nn(p2, C1, r); // p2 !!!
  n_Delete(&C1,r);

  poly T = mm_Mult_pp(m, pNext(p1), r);
  p_Delete(&m, r);

  p2 = p_Add_q(p2, T, r);

  if ( p2!=NULL ) pContent(p2); // r?

#ifdef PDEBUG
  p_Test(p2,r);
#endif

  return(p2);
}


void addLObject(LObject& h, kStrategy& strat)
{
  if(h.IsNull()) return;

  strat->initEcart(&h);
  h.sev=0; // pGetShortExpVector(h.p);

  // add h into S and L
  int pos=posInS(strat, strat->sl, h.p, h.ecart);

  if ( (pos <= strat->sl) && (pComparePolys(h.p, strat->S[pos])) )
  {
    if (TEST_OPT_PROT)
      PrintS("d\n");
  }
  else
  {
    if (TEST_OPT_INTSTRATEGY)
    {
      pCleardenom(h.p);
    }
    else
    {
      pNorm(h.p);
      pContent(h.p);
    }

    if ((strat->syzComp==0)||(!strat->homog))
    {
      h.p = redtailBba(h.p,pos-1,strat);

      if (TEST_OPT_INTSTRATEGY)
      {
//        pCleardenom(h.p);
        pContent(h.p);
      }
      else
      {
        pNorm(h.p);
      }
    }

    if(h.IsNull()) return;

    /* statistic */
    if (TEST_OPT_PROT)
    {
      PrintS("s\n");
    }

    if (TEST_OPT_DEBUG)
    {
      PrintS("new s:");
      wrp(h.p);
      PrintLn();
    }

    enterpairs(h.p, strat->sl, h.ecart, 0, strat);

    pos=0;

    if (strat->sl!=-1) pos = posInS(strat, strat->sl, h.p, h.ecart);

    strat->enterS(h, pos, strat, -1);

    if (h.lcm!=NULL) pLmFree(h.lcm);
  }


}


ideal sca_gr_bba(const ideal F, const ideal Q, const intvec *, const intvec *, kStrategy strat)
{
  assume(rIsSCA(currRing));

  BOOLEAN bIdHomog = id_IsYHomogeneous(F, currRing);

  strat->homog = strat->homog && bIdHomog;

#ifdef PDEBUG
  assume( strat->homog == bIdHomog );
#endif /*PDEBUG*/


#if 0
   PrintS("<sca::bba>\n");
  {
    Print("ideal F: \n");
    for (int i = 0; i < F->idelems(); i++)
    {
      Print("; F[%d] = ", i+1);
      p_Write(F->m[i], currRing);
    }
    Print(";\n");
    PrintLn();
  }
#endif


#ifdef PDEBUG
//   PrintS("sca_gr_bba\n");
#endif /*PDEBUG*/



  int srmax, lrmax;
  int olddeg, reduc;
  int red_result = 1;
  int hilbeledeg = 1, hilbcount = 0, minimcnt = 0;

  initBuchMoraCrit(strat); // set Gebauer, honey, sugarCrit

  gr_initBba(F,strat); // set enterS, red, initEcart, initEcartPair

  initBuchMoraPos(strat);


  // ?? set spSpolyShort, reduce ???

  initBuchMora(F, NULL, strat); // Q  = squares!!!!!!!

  strat->posInT=posInT110; // !!!

  srmax = strat->sl;
  reduc = olddeg = lrmax = 0;


  const unsigned int m_iFirstAltVar = scaFirstAltVar(currRing);
  const unsigned int m_iLastAltVar  = scaLastAltVar(currRing);




  /* compute------------------------------------------------------- */
  for(; strat->Ll >= 0;
#ifdef KDEBUG
    strat->P.lcm = NULL,
#endif
    kTest(strat)
    )
  {
    if (strat->Ll > lrmax) lrmax =strat->Ll;/*stat.*/

    if (TEST_OPT_DEBUG) messageSets(strat);

    if (strat->Ll== 0) strat->interpt=TRUE;

    if (TEST_OPT_DEGBOUND
    && ((strat->honey
    && (strat->L[strat->Ll].ecart+pFDeg(strat->L[strat->Ll].p,currRing)>Kstd1_deg))
       || ((!strat->honey) && (pFDeg(strat->L[strat->Ll].p,currRing)>Kstd1_deg))))
    {
      /*
      *stops computation if
      * 24 IN test and the degree +ecart of L[strat->Ll] is bigger then
      *a predefined number Kstd1_deg
      */
      while (strat->Ll >= 0) deleteInL(strat->L,&strat->Ll,strat->Ll,strat);
      break;
    }

    /* picks the last element from the lazyset L */
    strat->P = strat->L[strat->Ll];
    strat->Ll--;

    //kTest(strat);

    assume(pNext(strat->P.p) != strat->tail);

//     if( pNext(strat->P.p) == strat->tail )
//     {
//       // deletes the int spoly and computes SPoly
//       pLmFree(strat->P.p); // ???
//       strat->P.p = sca_SPoly(strat->P.p1, strat->P.p2, currRing);
//     }

    if(strat->P.IsNull()) continue;

//     poly save = NULL;
//
//     if(pNext(strat->P.p) != NULL)
//       save = p_Copy(strat->P.p, currRing);

    strat->initEcart(&strat->P); // remove it?

    if (TEST_OPT_PROT)
      message((strat->honey ? strat->P.ecart : 0) + strat->P.pFDeg(), &olddeg,&reduc,strat, red_result);

    // reduction of the element chosen from L wrt S
    strat->red(&strat->P,strat);

    if(strat->P.IsNull()) continue;

    addLObject(strat->P, strat);

    if (strat->sl > srmax) srmax = strat->sl;

    ////////////// children !!!!!!!!!!!!!

    const poly save = strat->P.p;

//     if( save != NULL )
    {

#ifdef PDEBUG
      p_Test(save, currRing);
#endif

      const poly pNext = pNext(save);

      for( unsigned int i = m_iFirstAltVar; i <= m_iLastAltVar; i++ )
      if( p_GetExp(save, i, currRing) )
      {
        const poly tt = xi_Mult_pp(i, pNext, currRing);

#ifdef PDEBUG
        p_Test(tt, currRing);
#endif

        if( tt == NULL) continue;

        LObject h(tt); // h = x_i * P

        if (TEST_OPT_INTSTRATEGY)
        {
//           h.pCleardenom(); // also does a pContent
          pContent(h.p);
        }
        else
        {
          h.pNorm();
        }

        strat->initEcart(&h);


//         if (pOrdSgn==-1)
//         {
//           cancelunit(&h);  // tries to cancel a unit
//           deleteHC(&h, strat);
//         }

//         if(h.IsNull()) continue;

//         if (TEST_OPT_PROT)
//           message((strat->honey ? h.ecart : 0) + h.pFDeg(), &olddeg, &reduc, strat, red_result);

//         strat->red(&h, strat); // wrt S
//         if(h.IsNull()) continue;


  //       poly save = p_Copy(h.p, currRing);

        int pos;

        if (strat->Ll==-1)
          pos =0;
        else
          pos = strat->posInL(strat->L,strat->Ll,&h,strat);

        h.sev = pGetShortExpVector(h.p);
        enterL(&strat->L,&strat->Ll,&strat->Lmax,h,pos);

  //       h.p = save;
  //       addLObject(h, strat);

//         if (strat->sl > srmax) srmax = strat->sl;
      }

      // p_Delete( &save, currRing );
    }


  } // for(;;)


  if (TEST_OPT_DEBUG) messageSets(strat);

  /* release temp data-------------------------------- */
  exitBuchMora(strat);

  if (TEST_OPT_WEIGHTM)
  {
    pFDeg=pFDegOld;
    pLDeg=pLDegOld;
    if (ecartWeights)
    {
      omFreeSize((ADDRESS)ecartWeights,(pVariables+1)*sizeof(int));
      ecartWeights=NULL;
    }
  }

  if (TEST_OPT_PROT) messageStat(srmax,lrmax,hilbcount,strat);

//  if (Q!=NULL) updateResult(strat->Shdl,Q,strat);


  /* complete reduction of the standard basis--------- */
  if (TEST_OPT_REDSB){
//     completeReduce(strat); // ???

    ideal I = strat->Shdl;
    ideal erg = kInterRed(I,NULL);
    assume(I!=erg);
    id_Delete(&I, currRing);
    strat->Shdl = erg;
  }


#if 0
  {
    Print("ideal S: \n");
    for (int i = 0; i < F->idelems(); i++)
    {
      Print("; S[%d] = ", 1+i);
      p_Write(F->m[i], currRing);
    }
    Print(";\n");
    PrintLn();
  }

   PrintS("</sca::bba>\n");
#endif

  return (strat->Shdl);
}


// is this an exterior algebra or a commuunative polynomial ring \otimes exterior algebra?
// we should check whether qr->qideal is of the form: y_i^2, y_{i+1}^2, \ldots, y_j^2 (j > i)
//.if yes, setup qr->nc->type, etc.
bool SetupSCA(ring& rGR, const ring rG)
{
//   return false; // test Plural

  //////////////////////////////////////////////////////////////////////////
  // checks...
  //////////////////////////////////////////////////////////////////////////
  assume(rGR != NULL);
  assume(rG  != NULL);

  assume(rIsPluralRing(rG));

  const int N = rG->N;
  if(N < 2)
    return false;

  // skew-commutative + constant skew? => no tensor products at the moment!!! Only exterior algebras are supported!

//   if( rG->nc->D != NULL )
//     return false;

  if(ncRingType(rG) != nc_skew)
    return false;

  if(rG->nc->IsSkewConstant != 1)
    return false;

  if(!n_IsMOne(p_GetCoeff(MATELEM(rG->nc->COM,1,2),rG->nc->basering), rG->nc->basering)) // COM live in basering!
    return false;

  assume(rGR->qideal != NULL);

  // sort qr->qideal:

  // for sanity
  ring rSaveRing = currRing;

  bool bWeChangeRing = false;
  if(currRing != rG)
  {
    rChangeCurrRing(rG);
    bWeChangeRing = true;
  }

  const ideal idQuotient = rGR->qideal;
  const int   iQSize = idQuotient->idelems();

  intvec *iv = idSort(idQuotient, FALSE); // by lex, currRing dependent!!! :(
  assume(iv != NULL);

  // check for y_{iAltVarStart}^2, y_{iAltVarStart+1}^2, \ldots, y_{iAltVarEnd}^2 (iAltVarEnd > iAltVarStart)
  bool bSCA = true;

  int iAltVarStart = -1;
  int iAltVarEnd   = -1;

  for(int i = 0; i < iQSize; i++)
  {
    const poly t = idQuotient->m[(*iv)[i]-1];

    if( pNext(t) != NULL )
    {
      bSCA = false;
      break;
    }

    const int iComp = p_GetComp(t, rG);

    if( iComp > 0 )
    {
      bSCA = false;
      break;
    }

    int iV = -1;

    for (int j = N; j; j--)
    {
      const int e = p_GetExp (t, j, rGR);

      if( e > 0 )
      {
        if( (e != 2) || (iV != -1) )
        {
          bSCA = false;
          break;
        }

        iV = j;
      }
    }

    if(!bSCA) break;

    if(iV == -1)
    {
      bSCA = false;
      break;
    }

    if(iAltVarStart == -1)
    {
      iAltVarStart = iV;
      iAltVarEnd   = iV;
    } else
    {
      if((iAltVarStart-1) == iV)
      {
        iAltVarStart = iV;
      } else
      if((iAltVarEnd+1) == iV)
      {
        iAltVarEnd = iV;
      } else
      {
        bSCA = false;
        break;
      }
    }
  }

  // cleanup:
  delete iv;

  if (bWeChangeRing)
  {
    rChangeCurrRing(rSaveRing);
  }


  if(!bSCA) return false;

  assume( 1            <= iAltVarStart );
  assume( iAltVarStart <= iAltVarEnd   );
  assume( iAltVarEnd   <= N            );


/*
  // Anti-commutative (c_{ij} = -1)?
  for( int i = iAltVarStart; i < iAltVarEnd; i++ )
    for( int j = i + 1; i <= iAltVarEnd; j++ )
      if(!n_IsMOne(p_GetCoeff(MATELEM(rG->nc->COM,i,j),rG->nc->basering), rG->nc->basering)) // COM live in basering!
        return false;


  for( int i = 1; i < iAltVarStart-1; i++ )
    for( int j = i + 1; i < iAltVarStart; j++ )
      if(!n_IsOne(p_GetCoeff(MATELEM(rG->nc->COM,i,j),rG->nc->basering), rG->nc->basering)) // COM live in basering!
        return false;


  for( int i = iAltVarEnd+1; i < N; i++ )
    for( int j = i + 1; i <= N; j++ )
      if(!n_IsOne(p_GetCoeff(MATELEM(rG->nc->COM,i,j),rG->nc->basering), rG->nc->basering)) // COM live in basering!
        return false;
*/

  //////////////////////////////////////////////////////////////////////////
  // ok... let's setup it!!!
  //////////////////////////////////////////////////////////////////////////

  ncRingType( rGR, nc_exterior );

  scaFirstAltVar( rGR, iAltVarStart );
  scaLastAltVar( rGR, iAltVarEnd );

  // ????????????????????????????????????????????????????????????????????????????????
  // check ordering!!! alternating variables must be bigger than all other variables?

  SetProcsSCA(rGR, rGR->p_Procs);

  return true;
}

// return x_i * pPoly; preserve pPoly.
poly xi_Mult_pp(unsigned int i, const poly pPoly, const ring rRing)
{
  assume(1 <= i);
  assume(i <= rRing->N);

  if(rIsSCA(rRing))
    return sca_xi_Mult_pp(i, pPoly, rRing);



  poly xi =  p_ISet(1, rRing);
  p_SetExp(xi, i, 1, rRing);
  p_Setm(xi, rRing);

  poly pResult = pp_Mult_qq(xi, pPoly, rRing);

  p_Delete( &xi, rRing);

  return pResult;

}


///////////////////////////////////////////////////////////////////////////////////////
//************* SCA BBA *************************************************************//
///////////////////////////////////////////////////////////////////////////////////////

// Under development!!!
ideal sca_bba (const ideal F, const ideal Q, const intvec *w, const intvec *hilb, kStrategy strat)
{
  assume(rIsSCA(currRing));

  BOOLEAN bIdHomog = id_IsYHomogeneous(F, currRing);

  strat->homog = strat->homog && bIdHomog;

#ifdef PDEBUG
  assume( strat->homog == bIdHomog );
#endif /*PDEBUG*/


//    PrintS("<sca>\n");

  om_Opts.MinTrack = 5; // ???

  int   srmax, lrmax, red_result = 1;
  int   olddeg, reduc;
  int hilbeledeg=1, hilbcount=0, minimcnt=0;

  BOOLEAN withT = FALSE;

  initBuchMoraCrit(strat); // sets Gebauer, honey, sugarCrit // sca - ok???
  initBuchMoraPos(strat); // sets strat->posInL, strat->posInT // check!! (Plural's: )

//   initHilbCrit(F, Q, &hilb, strat);

//  gr_initBba(F,strat);
  initBba(F, strat); // set enterS, red, initEcart, initEcartPair

  /*set enterS, spSpolyShort, reduce, red, initEcart, initEcartPair*/
  // ?? set spSpolyShort, reduce ???
  initBuchMora(F, NULL, strat); // Q = squares!!!

//   if (strat->minim>0) strat->M = idInit(IDELEMS(F),F->rank);

  srmax = strat->sl;
  reduc = olddeg = lrmax = 0;

#define NO_BUCKETS

#ifndef NO_BUCKETS
  if (!TEST_OPT_NOT_BUCKETS)
    strat->use_buckets = 1;
#endif

  // redtailBBa against T for inhomogenous input
  if (!K_TEST_OPT_OLDSTD)
    withT = ! strat->homog;

  // strat->posInT = posInT_pLength;
  kTest_TS(strat);

#undef HAVE_TAIL_RING

#ifdef HAVE_TAIL_RING
  kStratInitChangeTailRing(strat);
#endif

  ///////////////////////////////////////////////////////////////
  // SCA:
  const unsigned short m_iFirstAltVar = scaFirstAltVar(currRing);
  const unsigned short m_iLastAltVar  = scaLastAltVar(currRing);



  /* compute------------------------------------------------------- */
  while (strat->Ll >= 0)
  {
    if (strat->Ll > lrmax) lrmax =strat->Ll;/*stat.*/

#ifdef KDEBUG
//     loop_count++;
    if (TEST_OPT_DEBUG) messageSets(strat);
#endif

    if (strat->Ll== 0) strat->interpt=TRUE;

    if (TEST_OPT_DEGBOUND
        && ((strat->honey && (strat->L[strat->Ll].ecart+pFDeg(strat->L[strat->Ll].p,currRing)>Kstd1_deg))
            || ((!strat->honey) && (pFDeg(strat->L[strat->Ll].p,currRing)>Kstd1_deg))))
    {
      /*
       *stops computation if
       * 24 IN test and the degree +ecart of L[strat->Ll] is bigger then
       *a predefined number Kstd1_deg
       */
      while ((strat->Ll >= 0)
  && (strat->L[strat->Ll].p1!=NULL) && (strat->L[strat->Ll].p2!=NULL)
        && ((strat->honey && (strat->L[strat->Ll].ecart+pFDeg(strat->L[strat->Ll].p,currRing)>Kstd1_deg))
            || ((!strat->honey) && (pFDeg(strat->L[strat->Ll].p,currRing)>Kstd1_deg)))
  )
        deleteInL(strat->L,&strat->Ll,strat->Ll,strat);
      if (strat->Ll<0) break;
      else strat->noClearS=TRUE;
    }

    /* picks the last element from the lazyset L */
    strat->P = strat->L[strat->Ll];
    strat->Ll--;


    assume(pNext(strat->P.p) != strat->tail);

/*    if (pNext(strat->P.p) == strat->tail)
    {
      // deletes the short spoly
      pLmFree(strat->P.p);
      strat->P.p = NULL;
      poly m1 = NULL, m2 = NULL;

      // check that spoly creation is ok
      while (strat->tailRing != currRing &&
             !kCheckSpolyCreation(&(strat->P), strat, m1, m2))
      {
        assume(m1 == NULL && m2 == NULL);
        // if not, change to a ring where exponents are at least
        // large enough
        kStratChangeTailRing(strat);
      }

#ifdef PDEBUG
      Print("ksCreateSpoly!#?");
#endif

      // create the real one
      ksCreateSpoly(&(strat->P), NULL, strat->use_buckets,
                    strat->tailRing, m1, m2, strat->R); //?????????
    }
    else */

    if (strat->P.p1 == NULL)
    {
//       if (strat->minim > 0)
//         strat->P.p2=p_Copy(strat->P.p, currRing, strat->tailRing);


      // for input polys, prepare reduction
      strat->P.PrepareRed(strat->use_buckets);
    }

    if (TEST_OPT_PROT)
      message((strat->honey ? strat->P.ecart : 0) + strat->P.pFDeg(),
              &olddeg,&reduc,strat, red_result);

    /* reduction of the element choosen from L */
    red_result = strat->red(&strat->P,strat);


    // reduction to non-zero new poly
    if (red_result == 1)
    {
      /* statistic */
      if (TEST_OPT_PROT) PrintS("s");

      // get the polynomial (canonicalize bucket, make sure P.p is set)
      strat->P.GetP(strat->lmBin);

      int pos = posInS(strat,strat->sl,strat->P.p,strat->P.ecart);

      // reduce the tail and normalize poly
      if (TEST_OPT_INTSTRATEGY)
      {
        strat->P.pCleardenom();
        if ((TEST_OPT_REDSB)||(TEST_OPT_REDTAIL))
        {
          strat->P.p = redtailBba(&(strat->P),pos-1,strat, withT); // !!!
          strat->P.pCleardenom();

        }
      }
      else
      {

        strat->P.pNorm();
        if ((TEST_OPT_REDSB)||(TEST_OPT_REDTAIL))
          strat->P.p = redtailBba(&(strat->P),pos-1,strat, withT);
      }


#ifdef KDEBUG
      if (TEST_OPT_DEBUG){PrintS(" ns:");strat->P.wrp();PrintLn();}
#endif

//       // min_std stuff
//       if ((strat->P.p1==NULL) && (strat->minim>0))
//       {
//         if (strat->minim==1)
//         {
//           strat->M->m[minimcnt]=p_Copy(strat->P.p,currRing,strat->tailRing);
//           p_Delete(&strat->P.p2, currRing, strat->tailRing);
//         }
//         else
//         {
//           strat->M->m[minimcnt]=strat->P.p2;
//           strat->P.p2=NULL;
//         }
//         if (strat->tailRing!=currRing && pNext(strat->M->m[minimcnt])!=NULL)
//           pNext(strat->M->m[minimcnt])
//             = strat->p_shallow_copy_delete(pNext(strat->M->m[minimcnt]),
//                                            strat->tailRing, currRing,
//                                            currRing->PolyBin);
//         minimcnt++;
//       }

      // enter into S, L, and T
      if(withT)
        enterT(strat->P, strat);

      // L
      enterpairs(strat->P.p,strat->sl,strat->P.ecart,pos,strat, strat->tl);

      // posInS only depends on the leading term
      strat->enterS(strat->P, pos, strat, strat->tl);

//       if (hilb!=NULL) khCheck(Q,w,hilb,hilbeledeg,hilbcount,strat);

//      Print("[%d]",hilbeledeg);
      if (strat->P.lcm!=NULL) pLmFree(strat->P.lcm);

      if (strat->sl>srmax) srmax = strat->sl;

      // //////////////////////////////////////////////////////////
      // SCA:
      const poly pSave = strat->P.p;
      const poly pNext = pNext(pSave);

//       if(0)
      for( unsigned short i = m_iFirstAltVar; i <= m_iLastAltVar; i++ )
      if( p_GetExp(pSave, i, currRing) )
      {
        const poly pNew = xi_Mult_pp(i, pNext, currRing);

#ifdef PDEBUG
        p_Test(pNew, currRing);
#endif

        if( pNew == NULL) continue;

        LObject h(pNew); // h = x_i * strat->P

        if (TEST_OPT_INTSTRATEGY)
        {
//           h.pCleardenom(); // also does a pContent
          pContent(h.p);
        }
        else
        {
          h.pNorm();
        }

        strat->initEcart(&h);
        h.sev = pGetShortExpVector(h.p);

        int pos;

        if (strat->Ll==-1)
          pos =0;
        else
          pos = strat->posInL(strat->L,strat->Ll,&h,strat);

        enterL(&strat->L,&strat->Ll,&strat->Lmax,h,pos);


/*
        h.sev = pGetShortExpVector(h.p);
        strat->initEcart(&h);

        h.PrepareRed(strat->use_buckets);

        // reduction of the element choosen from L(?)
        red_result = strat->red(&h,strat);

        // reduction to non-zero new poly
        if (red_result != 1) continue;


        int pos = posInS(strat,strat->sl,h.p,h.ecart);

        // reduce the tail and normalize poly
        if (TEST_OPT_INTSTRATEGY)
        {
          h.pCleardenom();
          if ((TEST_OPT_REDSB)||(TEST_OPT_REDTAIL))
          {
            h.p = redtailBba(&(h),pos-1,strat, withT); // !!!
            h.pCleardenom();
          }
        }
        else
        {

          h.pNorm();
          if ((TEST_OPT_REDSB)||(TEST_OPT_REDTAIL))
            h.p = redtailBba(&(h),pos-1,strat, withT);
        }


  #ifdef KDEBUG
        if (TEST_OPT_DEBUG){PrintS(" N:");h.wrp();PrintLn();}
  #endif

//         h.PrepareRed(strat->use_buckets); // ???

        h.sev = pGetShortExpVector(h.p);
        strat->initEcart(&h);

        if (strat->Ll==-1)
          pos = 0;
        else
          pos = strat->posInL(strat->L,strat->Ll,&h,strat);

         enterL(&strat->L,&strat->Ll,&strat->Lmax,h,pos);*/

      } // for all x_i \in Ann(lm(P))





    } // if red(P) != NULL

//     else if (strat->P.p1 == NULL && strat->minim > 0)
//     {
//       p_Delete(&strat->P.p2, currRing, strat->tailRing);
//     }

// #ifdef KDEBUG
    memset(&(strat->P), 0, sizeof(strat->P));
// #endif

    kTest_TS(strat);

//     Print("\n$\n");

  }

#ifdef KDEBUG
  if (TEST_OPT_DEBUG) messageSets(strat);
#endif

  /* complete reduction of the standard basis--------- */
  if (TEST_OPT_REDSB) completeReduce(strat);

  /* release temp data-------------------------------- */

  exitBuchMora(strat);

  if (TEST_OPT_WEIGHTM)
  {
    pRestoreDegProcs(pFDegOld, pLDegOld);
    if (ecartWeights)
    {
      omFreeSize((ADDRESS)ecartWeights,(pVariables+1)*sizeof(short));
      ecartWeights=NULL;
    }
  }

  if (TEST_OPT_PROT) messageStat(srmax,lrmax,hilbcount,strat);

//   if (Q!=NULL) updateResult(strat->Shdl,Q,strat);

//    PrintS("</sca>\n");

  return (strat->Shdl);
}



// y-Degree
inline long p_yDegree(const poly p, const ring r)
{
  assume(rIsSCA(r));

  const unsigned short m_iFirstAltVar = scaFirstAltVar(r);
  const unsigned short m_iLastAltVar  = scaLastAltVar(r);

  long sum = 0;

  for(int i = m_iFirstAltVar; i <= m_iLastAltVar; i++)
    sum += p_GetExp(p, i, r);

  return sum;
}




/**2
 * tests whether p is sca-homogeneous without respect to the actual weigths(=>all ones)
 */
BOOLEAN p_IsYHomogeneous(const poly p, const ring r)
{
  assume(rIsSCA(r));

  if( p == NULL ) return TRUE;

  poly q = p;

  const long o = p_yDegree(q, r);
  pIter(q);

  while( q != NULL )
  {
    if (p_yDegree(q,r) != o) return FALSE;
    pIter(q);
  }

  return TRUE;
}


/*2
*returns true if id is y-homogenous without respect to the aktual weights(=> all ones)
*/
BOOLEAN id_IsYHomogeneous(const ideal id, const ring r)
{
  if (id == NULL) return TRUE;

  const int iSize = IDELEMS(id);

  if (iSize == 0) return TRUE;

  BOOLEAN b = TRUE;

  for(int i = iSize - 1; (i >= 0 ) && b; i--)
    b = p_IsYHomogeneous(id->m[i], r);

  return b;
}

// //////////////////////////////////////////////////////////////////////////////
// sca mora...

// returns TRUE if mora should use buckets, false otherwise
static BOOLEAN kMoraUseBucket(kStrategy strat)
{
#ifdef MORA_USE_BUCKETS
  if (TEST_OPT_NOT_BUCKETS)
    return FALSE;
  if (strat->red == redFirst)
  {
#ifdef NO_LDEG
    if (!strat->syzComp)
      return TRUE;
#else
    if ((strat->homog || strat->honey) && !strat->syzComp)
      return TRUE;
#endif
  }
  else
  {
    assume(strat->red == redEcart);
    if (strat->honey && !strat->syzComp)
      return TRUE;
  }
#endif
  return FALSE;
}


#ifdef HAVE_ASSUME
static int sca_mora_count = 0;
static int sca_mora_loop_count;
#endif

// ideal sca_mora (ideal F, ideal Q, intvec *w, intvec *, kStrategy strat)
ideal sca_mora(const ideal F, const ideal Q, const intvec *w, const intvec *, kStrategy strat)
{
  BOOLEAN bIdHomog = id_IsYHomogeneous(F, currRing);

  strat->homog = strat->homog && bIdHomog;

#ifdef PDEBUG
  assume( strat->homog == bIdHomog );
#endif /*PDEBUG*/


#ifdef HAVE_ASSUME
  sca_mora_count++;
  sca_mora_loop_count = 0;
#endif
#ifdef KDEBUG
  om_Opts.MinTrack = 5;
#endif
  int srmax;
  int lrmax = 0;
  int olddeg = 0;
  int reduc = 0;
  int red_result = 1;
  int hilbeledeg=1,hilbcount=0;

  strat->update = TRUE;
  /*- setting global variables ------------------- -*/
  initBuchMoraCrit(strat);
//   initHilbCrit(F,NULL,&hilb,strat); // no Q!
  initMora(F,strat);
  initBuchMoraPos(strat);
  /*Shdl=*/initBuchMora(F,NULL,strat); // no Q!
//   if (TEST_OPT_FASTHC) missingAxis(&strat->lastAxis,strat);
  /*updateS in initBuchMora has Hecketest
  * and could have put strat->kHEdgdeFound FALSE*/
#if 0
  if (ppNoether!=NULL)
  {
    strat->kHEdgeFound = TRUE;
  }
  if (strat->kHEdgeFound && strat->update)
  {
    firstUpdate(strat);
    updateLHC(strat);
    reorderL(strat);
  }
  if (TEST_OPT_FASTHC && (strat->lastAxis) && strat->posInLOldFlag)
  {
    strat->posInLOld = strat->posInL;
    strat->posInLOldFlag = FALSE;
    strat->posInL = posInL10;
    updateL(strat);
    reorderL(strat);
  }
#endif

  srmax = strat->sl;
  kTest_TS(strat);

  strat->use_buckets = kMoraUseBucket(strat);
  /*- compute-------------------------------------------*/

#undef HAVE_TAIL_RING

#ifdef HAVE_TAIL_RING
//  if (strat->homog && strat->red == redFirst)
//     kStratInitChangeTailRing(strat);
#endif


  // 4SCA:
  const unsigned short m_iFirstAltVar = scaFirstAltVar(currRing);
  const unsigned short m_iLastAltVar  = scaLastAltVar(currRing);

  while (strat->Ll >= 0)
  {
#ifdef HAVE_ASSUME
    sca_mora_loop_count++;
#endif
    if (lrmax< strat->Ll) lrmax=strat->Ll; /*stat*/
    //test_int_std(strat->kIdeal);
    if (TEST_OPT_DEBUG) messageSets(strat);
    if (TEST_OPT_DEGBOUND
    && (strat->L[strat->Ll].ecart+strat->L[strat->Ll].GetpFDeg()> Kstd1_deg))
    {
      /*
      * stops computation if
      * - 24 (degBound)
      *   && upper degree is bigger than Kstd1_deg
      */
      while ((strat->Ll >= 0)
        && (strat->L[strat->Ll].p1!=NULL) && (strat->L[strat->Ll].p2!=NULL)
        && (strat->L[strat->Ll].ecart+strat->L[strat->Ll].GetpFDeg()> Kstd1_deg)
      )
      {
        deleteInL(strat->L,&strat->Ll,strat->Ll,strat);
        //if (TEST_OPT_PROT)
        //{
        //   PrintS("D"); mflush();
        //}
      }
      if (strat->Ll<0) break;
      else strat->noClearS=TRUE;
    }
    strat->P = strat->L[strat->Ll];/*- picks the last element from the lazyset L -*/
    if (strat->Ll==0) strat->interpt=TRUE;
    strat->Ll--;

    // create the real Spoly
    assume(pNext(strat->P.p) != strat->tail);

    if (strat->P.p1 == NULL)
    {
      // for input polys, prepare reduction (buckets !)
      strat->P.SetLength(strat->length_pLength);
      strat->P.PrepareRed(strat->use_buckets);
    }

    if (!strat->P.IsNull())
    {
      // might be NULL from noether !!!
      if (TEST_OPT_PROT)
        message(strat->P.ecart+strat->P.GetpFDeg(),&olddeg,&reduc,strat, red_result);
      // reduce
      red_result = strat->red(&strat->P,strat);
    }

    if (! strat->P.IsNull())
    {
      strat->P.GetP();
      // statistics
      if (TEST_OPT_PROT) PrintS("s");
      // normalization
      if (!TEST_OPT_INTSTRATEGY)
        strat->P.pNorm();
      // tailreduction
      strat->P.p = redtail(&(strat->P),strat->sl,strat);
      // set ecart -- might have changed because of tail reductions
      if ((!strat->noTailReduction) && (!strat->honey))
        strat->initEcart(&strat->P);
      // cancel unit
      cancelunit(&strat->P);
      // for char 0, clear denominators
      if (TEST_OPT_INTSTRATEGY)
        strat->P.pCleardenom();

      // put in T
      enterT(strat->P,strat);
      // build new pairs
      enterpairs(strat->P.p,strat->sl,strat->P.ecart,0,strat, strat->tl);
      // put in S
      strat->enterS(strat->P,
                    posInS(strat,strat->sl,strat->P.p, strat->P.ecart),
                    strat, strat->tl);


      // clear strat->P
      if (strat->P.lcm!=NULL) pLmFree(strat->P.lcm);
      strat->P.lcm=NULL;

      if (strat->sl>srmax) srmax = strat->sl; /*stat.*/
      if (strat->Ll>lrmax) lrmax = strat->Ll;



      // //////////////////////////////////////////////////////////
      // SCA:
      const poly pSave = strat->P.p;
      const poly pNext = pNext(pSave);

//       if(0)
      for( unsigned short i = m_iFirstAltVar; i <= m_iLastAltVar; i++ )
      if( p_GetExp(pSave, i, currRing) )
      {

        assume(p_GetExp(pSave, i, currRing) == 1);

        const poly pNew = xi_Mult_pp(i, pNext, currRing);

#ifdef PDEBUG
        p_Test(pNew, currRing);
#endif

        if( pNew == NULL) continue;

        LObject h(pNew); // h = x_i * strat->P

        if (TEST_OPT_INTSTRATEGY)
           h.pCleardenom(); // also does a pContent
        else
          h.pNorm();

        strat->initEcart(&h);
        h.sev = pGetShortExpVector(h.p);

        int pos;

        if (strat->Ll==-1)
          pos = 0;
        else
          pos = strat->posInL(strat->L,strat->Ll,&h,strat);

        enterL(&strat->L,&strat->Ll,&strat->Lmax,h,pos);

        if (strat->Ll>lrmax) lrmax = strat->Ll;
      }

#ifdef KDEBUG
      // make sure kTest_TS does not complain about strat->P
      memset(&strat->P,0,sizeof(strat->P));
#endif
    }
#if 0
    if (strat->kHEdgeFound)
    {
      if ((BTEST1(27))
      || ((TEST_OPT_MULTBOUND) && (scMult0Int((strat->Shdl)) < mu)))
      {
        // obachman: is this still used ???
        /*
        * stops computation if strat->kHEdgeFound and
        * - 27 (finiteDeterminacyTest)
        * or
        * - 23
        *   (multBound)
        *   && multiplicity of the ideal is smaller then a predefined number mu
        */
        while (strat->Ll >= 0) deleteInL(strat->L,&strat->Ll,strat->Ll,strat);
      }
    }
#endif
    kTest_TS(strat);
  }
  /*- complete reduction of the standard basis------------------------ -*/
  if (TEST_OPT_REDSB) completeReduce(strat);
  /*- release temp data------------------------------- -*/
  exitBuchMora(strat);
  /*- polynomials used for HECKE: HC, noether -*/
  if (BTEST1(27))
  {
    if (strat->kHEdge!=NULL)
      Kstd1_mu=pFDeg(strat->kHEdge,currRing);
    else
      Kstd1_mu=-1;
  }
  pDelete(&strat->kHEdge);
  strat->update = TRUE; //???
  strat->lastAxis = 0; //???
  pDelete(&strat->kNoether);
  omFreeSize((ADDRESS)strat->NotUsedAxis,(pVariables+1)*sizeof(BOOLEAN));
  if (TEST_OPT_PROT) messageStat(srmax,lrmax,hilbcount,strat);
  if (TEST_OPT_WEIGHTM)
  {
    pRestoreDegProcs(pFDegOld, pLDegOld);
    if (ecartWeights)
    {
      omFreeSize((ADDRESS)ecartWeights,(pVariables+1)*sizeof(short));
      ecartWeights=NULL;
    }
  }
//   if (Q!=NULL) updateResult(strat->Shdl,Q,strat);
  idTest(strat->Shdl);
  return (strat->Shdl);
}






void SetProcsSCA(ring& rGR, p_Procs_s* p_Procs)
{

  // "commutative" procedures:
  rGR->p_Procs->p_Mult_mm     = sca_p_Mult_mm;
  rGR->p_Procs->pp_Mult_mm    = sca_pp_Mult_mm;

  p_Procs->p_Mult_mm          = sca_p_Mult_mm;
  p_Procs->pp_Mult_mm         = sca_pp_Mult_mm;

  // non-commutaitve
  rGR->nc->p_Procs.mm_Mult_p   = sca_mm_Mult_p;
  rGR->nc->p_Procs.mm_Mult_pp  = sca_mm_Mult_pp;


  if (pOrdSgn==-1)
    rGR->nc->p_Procs.GB          = sca_mora; // local ordering => Mora, otherwise - Buchberger!
  else
    rGR->nc->p_Procs.GB          = sca_gr_bba; // sca_bba?


//   rGR->nc->p_Procs.GlobalGB    = sca_gr_bba;
//   rGR->nc->p_Procs.LocalGB     = sca_mora;


//   rGR->nc->p_Procs.SPoly         = sca_SPoly;
//   rGR->nc->p_Procs.ReduceSPoly   = sca_ReduceSpoly;

#if 0

        // Multiplication procedures:

        p_Procs->p_Mult_mm   = sca_p_Mult_mm;
        _p_procs->p_Mult_mm  = sca_p_Mult_mm;

        p_Procs->pp_Mult_mm  = sca_pp_Mult_mm;
        _p_procs->pp_Mult_mm = sca_pp_Mult_mm;

        r->nc->mmMultP()     = sca_mm_Mult_p;
        r->nc->mmMultPP()    = sca_mm_Mult_pp;

        r->nc->GB()            = sca_gr_bba;
/*
        // ??????????????????????????????????????? coefficients swell...
        r->nc->SPoly()         = sca_SPoly;
        r->nc->ReduceSPoly()   = sca_ReduceSpoly;
*/
//         r->nc->BucketPolyRed() = gnc_kBucketPolyRed;
//         r->nc->BucketPolyRed_Z()= gnc_kBucketPolyRed_Z;

#endif
}
