LIB "tst.lib"; tst_init();
//******* Part 1 *******//
LIB "freegb.lib";
ring r = 0,(x,y,z),dp; // the ordering on the free algebra will be degree right lex
ring R = freeAlgebra(r, 5);  // 5 the is degree (length) bound;
ideal I = x*y + y*z, x*x + x*y - z; // define an ideal via the set of polynomials
ideal J = twostd(I);
J; // as we see, with respect to the current ordering this Groebner basis
// tends to be infinite. Increasing the bound and recomputing helps to check it.
poly p = reduce(x*y*z*y,J);
p; // since p!=0, x*y*z*y is not contained in J up to length 5
// however this does not imply a definite answer on whether p is in J
poly q = x*(y+1)*z*y-x*y*z^2;
reduce(q, J); // 0, thus q is in J
//******* Part 2 *******//
qring Q = J; // J is a Groebner basis, computed above
poly p = reduce(x*x, twostd(0)); // the canonical representative of x*x in Q
p;
rightstd(ideal(p)); // right Groebner basis of the right ideal, generated by p in Q
//******* Part 3 *******//
setring r;
ring R8 = freeAlgebra(r, 5, 8);  // 5 the is length bound; 8 is the rank of the free bimodule
ideal J = imap(R, J); // we map J identically from R (of rank 1)
J = twostd(J);
poly q = imap(R, q);
NF(q, J); // NF is an alias to reduce, we have rechecked that q is in J
matrix L = lift(J, q); // creates the presentation for q in terms of J
// since J is a Groebner basis, this is a Groebner presentation of q
print(transpose(matrix(L))); // J has 8 generators and these are the needed coefficients
// here, the generators of the free bimodule are ncgen(1)*gen(1), ... , ncgen(8)*gen(8)
// the output means, that substituting ncgen(i) by the i-th generator of J, we get q
J[1]*z*y - J[1]*z*z  - J[5] - q; // 0, so this is the seeked expression of q
testLift(J,L); // recovers q from the lift matrix
// Let us compare now this nice Groebner presentation with the one
// obtained from the original set of generators
ideal I = imap(R,I); // note: I is not a Groebner basis of itself
matrix M = lift(I, q); // creates the presentation for q in terms of I
M; // presentation is longer and more complicated than the one in L
testLift(I,M); // a routine test to ensure that indeed we recover q
//******* Part 4 *******//
// Let us compute the module of bisyzygies of J and analyze it
module S = syz(J); size(S); // 18
S[6]; // consider, for example, this element
// plugging the i-th generator of J instead of ncgen(i), we obtain a bisyzygy:
J[1]*z*y - J[1]*z*z - x*J[3] - J[5]; //0
module S2 = S[6..8]; // pick just three generators
testSyz(J,S2); // tests the bisyzygy property for the generators
//******* Part 5 *******//
option(redSB); option(redTail); // to compute minimal and tail-reduced bases
module GS = twostd(S); size(GS); // 30
// let us construct a vector, belonging to GS:
vector v = GS[11]*y - x*GS[7] + z*GS[3]*z;
print(v);
NF(v, GS); // 0, by the construction
// now we wish to compute the expression of v via GS
ring r3 = 0,(x,y,z),(c,dp);
ring R30 = freeAlgebra(r3,5,30);
module GS = imap(R8,GS);
vector v = imap(R8,v);
matrix L = lift(GS,v); // via printing we see only three components involved:
L[3,1]; // =z*ncgen(3)*z, as well as
L[7,1]; // =-x*ncgen(7) and
L[11,1]; // =ncgen(11)*y
//******* Part 6 *******//
// Notice, that the module ordering is (c,dp): it is a position-over-term ordering
// which eliminates module components in an descending way.
GS = GS[1..5]; // consider just first five syzygies,  for a smaller example
GS = twostd(GS); // a nice finite Groebner basis
print(matrix(GS)); // shows the structure
// As we can see, intersections of the subbimodule GS with the free bimodules
// generated by all but first resp. all but first two bimodule generators
// are not empty and given by vectors having zero in the first resp.
// in the first two components.
tst_status(1);$