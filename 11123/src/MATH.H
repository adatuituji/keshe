/*	math.h
1 三角函数
double sin (double);
double cos (double);
double tan (double);
 
2 反三角函数
double asin (double);   结果介于[-PI/2, PI/2]
double acos (double);   结果介于[0, PI]
double atan (double);   反正切(主值), 结果介于[-PI/2, PI/2]
double atan2 (double, double);  反正切(整圆值), 结果介于[-PI/2, PI/2]
 
3 双曲三角函数
double sinh (double);
double cosh (double);
double tanh (double);
 
4 指数与对数
double exp (double);
double pow (double, double);
double sqrt (double);
double log (double);   以e为底的对数
double log10 (double);
 
5 取整
double ceil (double);   取上整
double floor (double);   取下整
 
6 绝对值
double fabs (double);
 
7 标准化浮点数
double frexp (double f, int *p); 标准化浮点数, f = x * 2^p, 已知f求x, p ( x介于[0.5, 1] )
double ldexp (double x, int p); 与frexp相反, 已知x, p求f
 
8 取整与取余
double modf (double, double*);  将参数的整数部分通过指针回传, 返回小数部分
double fmod (double, double);  返回两参数相除的余数
*/
#if __STDC__
#define _Cdecl
#else
#define _Cdecl	cdecl
#endif

#ifndef  _MATH_H
#define  _MATH_H 1

#define EDOM	33		/* Math argument */
#define ERANGE	34		/* Result too large */

#define HUGE_VAL	_huge_dble
extern double _Cdecl _huge_dble;

int	_Cdecl abs   (int x);
double	_Cdecl acos  (double x);
double	_Cdecl asin  (double x);
double	_Cdecl atan  (double x);
double	_Cdecl atan2 (double y, double x);
double	_Cdecl atof  (const char *s);
double	_Cdecl ceil  (double x);
double	_Cdecl cos   (double x);
double	_Cdecl cosh  (double x);
double	_Cdecl exp   (double x);
double	_Cdecl fabs  (double x);
double	_Cdecl floor (double x);
double	_Cdecl fmod  (double x, double y);
double	_Cdecl frexp (double x, int *exponent);
long	_Cdecl labs  (long x);
double	_Cdecl ldexp (double x, int exponent);
double	_Cdecl log   (double x);
double	_Cdecl log10 (double x);
double	_Cdecl modf  (double x, double *ipart);
double	_Cdecl pow   (double x, double y);
double	_Cdecl sin   (double x);
double	_Cdecl sinh  (double x);
double	_Cdecl sqrt  (double x);
double	_Cdecl tan   (double x);
double	_Cdecl tanh  (double x);

struct	exception 
{
	int	type;
	char   *name;
	double	arg1, arg2, retval;
};

int _Cdecl matherr (struct exception *e);

#if !__STDC__
double	cdecl hypot (double x, double y);
double	cdecl poly  (double x, int degree, double coeffs []);
double	cdecl pow10 (int p);

struct complex	    /* as used by "cabs" function */
{
    double  x, y;
};

#define cabs(z)     (hypot ((z).x, (z).y))

/*  The customary matherr() exception handler for maths functions is
    not compatible with the x3j11 draft standard for C.  _matherr() is
    provided as a compromise.
*/

typedef enum
{
    DOMAIN = 1,    /* argument domain error -- log (-1)        */
    SING,	   /* argument singularity  -- pow (0,-2))     */
    OVERFLOW,	   /* overflow range error  -- exp (1000)      */
    UNDERFLOW,	   /* underflow range error -- exp (-1000)     */
    TLOSS,	   /* total loss of significance -- sin(10e70) */
    PLOSS,	   /* partial loss of signif. -- not used      */
}   _mexcep;

double _Cdecl _matherr (_mexcep why, char *fun, double  *arg1p,
			double  *arg2p, double  retval);

/* Constants rounded for 21 decimals. */
#define M_E		2.71828182845904523536
#define M_LOG2E		1.44269504088896340736
#define M_LOG10E	0.434294481903251827651
#define M_LN2		0.693147180559945309417
#define M_LN10		2.30258509299404568402
#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923
#define M_PI_4		0.785398163397448309116
#define M_1_PI		0.318309886183790671538
#define M_2_PI		0.636619772367581343076
#define M_1_SQRTPI	0.564189583547756286948
#define M_2_SQRTPI	1.12837916709551257390
#define M_SQRT2		1.41421356237309504880
#define M_SQRT_2	0.707106781186547524401

#endif	/* __STDC__ */

#endif
