    /*
           printf(format, ...)

       Prints formatted data to [[stdout]]. The format string (the first
       argument)
       controls how the following arguments are converted.

       ### Format of the format string ###

       The format string is a character string, beginning and  ending  in  its
       initial  shift state, if any.  The format string is composed of zero or
       more  directives:  ordinary  characters  (not  %),  which  are   copied
       unchanged  to the output stream; and conversion specifications, each of
       which results in fetching zero or more subsequent arguments.  Each
       conversion specification is introduced by the character %, and ends with a
       conversion specifier.  In between there may be (in this order) zero  or
       more  flags, an optional minimum field width, an optional precision and
       an optional length modifier.

       For some numeric conversions a radix  character  ('decimal  point')  or
       thousands'  grouping  character  is  used.   The  actual character used
       depends on the LC\_NUMERIC part of the locale.  The  POSIX  locale  uses
       '.' as radix character, and does not have a grouping character.  Thus,

               printf("%'.2f", 1234567.89);

       results  in  '1234567.89'  in  the POSIX locale, in '1234567,89' in the
       nl\_NL locale, and in '1.234.567,89' in the da\_DK locale.


       ### The flag characters ###

       The character % is followed by zero or more of the following flags:

       * **#** The value should be converted to an  "alternate  form".   For  o
         conversions,  the  first  character of the output string is made
         zero (by prefixing a 0 if it was not zero already).  For x and X
         conversions, a nonzero result has the string '0x' (or '0X' for X
         conversions) prepended to it.  For a, A, e, E, f, F,  g,  and  G
         conversions,  the  result  will  always contain a decimal point,
         even if no digits follow it (normally, a decimal  point  appears
         in  the  results  of those conversions only if a digit follows).
         For g and G conversions, trailing zeros are not removed from the
         result  as  they would otherwise be.  For other conversions, the
         result is undefined.
       * **0** The value should be zero padded.  For d, i, o, u, x, X, a, A, e,
         E,  f, F, g, and G conversions, the converted value is padded on
         the left with zeros rather than blanks.  If the 0  and  -  flags
         both  appear,  the  0  flag is ignored.  If a precision is given
         with a numeric conversion (d, i, o, u, x, and X), the 0 flag  is
         ignored.  For other conversions, the behavior is undefined.
       * **-** The  converted  value is to be left adjusted on the field
         boundary.  (The  default  is  right  justification.)   Except  for  n
         conversions,  the  converted  value  is padded on the right with
         blanks, rather than on the left with blanks or zeros.  A - over-
         rides a 0 if both are given.
       * **' '** (a  space)  A  blank should be left before a positive number (or
         empty string) produced by a signed conversion.
       * **+** A sign (+ or -) should always be placed before a number produced
         by a signed conversion.  By default a sign is used only for 
	 negative numbers.  A + overrides a space if both are used.

       The five flag characters above are defined  in  the  C  standard.   The
       SUSv2 specifies one further flag character.

       * **'** For decimal conversion (i, d, u, f, F, g, G) the output is to be
         grouped with thousands' grouping characters if the locale infor-
         mation  indicates any.  Note that many versions of gcc(1) cannot
         parse this option and will issue  a  warning.   SUSv2  does  not
         include %'F.

       glibc 2.2 adds one further flag character.

       * **I** For  decimal  integer  conversion  (i, d, u) the output uses the
         locale's alternative output digits, if any.  For example,  since
         glibc  2.2.3  this  will give Arabic-Indic digits in the Persian
         ('fa\_IR') locale.

       ### The field width ###

       An optional decimal digit string (with nonzero first digit)  specifying
       a  minimum  field  width.   If the converted value has fewer characters
       than the field width, it will be padded with spaces  on  the  left  (or
       right, if the left-adjustment flag has been given).  Instead of a decimal
       digit string one may write '*' or '*m$' (for some  decimal  integer
       m) to specify that the field width is given in the next argument, or in
       the m-th argument, respectively, which must be of type int.  A negative
       field  width is taken as a '-' flag followed by a positive field width.
       In no case does a nonexistent or small field width cause truncation  of
       a  field;  if the result of a conversion is wider than the field width,
       the field is expanded to contain the conversion result.

       ### The precision ###

       An optional precision, in the form of a period ('.')   followed  by  an
       optional  decimal  digit string.  Instead of a decimal digit string one
       may write '*' or '*m$' (for some decimal integer m) to specify that the
       precision  is  given  in  the  next  argument, or in the m-th argument,
       respectively, which must be of type int.  If the precision is given  as
       just  '.',  or  the precision is negative, the precision is taken to be
       zero.  This gives the minimum number of digits to appear for d,  i,  o,
       u, x, and X conversions, the number of digits to appear after the radix
       character for a, A, e, E, f, and F conversions, the maximum  number  of
       significant  digits  for  g and G conversions, or the maximum number of
       characters to be printed from a string for s and S conversions.

       ### The length modifier ###

       Here, 'integer conversion' stands for d, i, o, u, x, or X conversion.

       * **hh** A following integer conversion corresponds to a signed  char  or
         unsigned  char argument, or a following n conversion corresponds
         to a pointer to a signed char argument.
       * **h** A following integer conversion corresponds to  a  short  int  or
         unsigned  short int argument, or a following n conversion corre-
         sponds to a pointer to a short int argument.
       * **l** (ell) A following integer conversion corresponds to a  long  int
         or  unsigned long int argument, or a following n conversion cor-
         responds to a pointer to a long int argument, or a  following  c
         conversion  corresponds  to  a wint\_t argument, or a following s
         conversion corresponds to a pointer to wchar\_t argument.

       * **ll**     (ell-ell).  A following integer conversion corresponds to a long
         long  int  or  unsigned long long int argument, or a following n
         conversion corresponds to a pointer to a long long int argument.
       * **L**      A  following a, A, e, E, f, F, g, or G conversion corresponds to
         a long double argument.  (C99 allows %LF, but SUSv2 does not.)

       * **q**      ('quad'. 4.4BSD and Linux libc5 only.  Don't use.)   This  is  a
         synonym for ll.
       * **j**      A  following  integer  conversion  corresponds to an intmax\_t or
         uintmax\_t argument.
       * **z**      A following  integer  conversion  corresponds  to  a  size\_t  or
         ssize\_t  argument.  (Linux libc5 has Z with this meaning.  Don't
         use it.)
       * **t**      A following integer conversion corresponds to a ptrdiff\_t  argu-
         ment.

       The  SUSv2  only knows about the length modifiers h (in hd, hi, ho, hx,
       hX, hn) and l (in ld, li, lo, lx, lX, ln, lc, ls) and L (in Le, LE, Lf,
       Lg, LG).

       ### The conversion specifier ###

       A  character  that specifies the type of conversion to be applied.  The
       conversion specifiers and their meanings are:

       * **d,i** The int argument is converted to signed decimal  notation.   The
         precision,  if any, gives the minimum number of digits that must
         appear; if the converted value  requires  fewer  digits,  it  is
         padded  on  the  left  with  zeros.  The default precision is 1.
         When 0 is printed with an explicit precision 0,  the  output  is
         empty.
       * **o,u,x,X**
         The  unsigned  int  argument is converted to unsigned octal (o),
         unsigned decimal (u), or unsigned hexadecimal (x  and  X)  notation.
         The  letters abcdef are used for x conversions; the letters
         ABCDEF are used for X conversions.  The precision, if  any,
         gives the minimum number of digits that must appear; if the converted
         value requires fewer digits, it is  padded  on  the  left
         with zeros.  The default precision is 1.  When 0 is printed with
         an explicit precision 0, the output is empty.
       * **e,E**    The double argument  is  rounded  and  converted  in  the  style
         [-]d.dddeÂdd  where  there is one digit before the decimal-point
         character and the number of digits after it is equal to the precision;
         if  the  precision is missing, it is taken as 6; if the
         precision is zero, no decimal-point  character  appears.   An  E
         conversion  uses  the  letter E (rather than e) to introduce the
         exponent.  The exponent always contains at least two digits;  if
         the value is zero, the exponent is 00.
       * **f,F**    The double argument is rounded and converted to decimal notation
         in the style [-]ddd.ddd, where the number of  digits  after  the
         decimal-point character is equal to the precision specification.
         If the precision is missing, it is taken as 6; if the  precision
         is  explicitly  zero,  no decimal-point character appears.  If a
         decimal point appears, at least one digit appears before it.

         (The SUSv2 does not know about F and says that character  string
         representations for infinity and NaN may be made available.  The
         C99 standard specifies '[-]inf' or '[-]infinity'  for  infinity,
         and  a string starting with 'nan' for NaN, in the case of f
         conversion, and '[-]INF' or '[-]INFINITY' or 'NAN*' in the case  of
         F conversion.)
       * **g,G**  The  double argument is converted in style f or e (or F or E for
         G conversions).  The precision specifies the number of 
	 significant  digits.   If the precision is missing, 6 digits are given;
	 if the precision is zero, it is treated as 1.  Style e  is  used
	 if  the  exponent from its conversion is less than -4 or greater
	 than or equal to the precision.  Trailing zeros are removed from
	 the  fractional part of the result; a decimal point appears only
	 if it is followed by at least one digit.
       * **a,A**    (C99; not in SUSv2) For a conversion,  the  double  argument  is
         converted  to hexadecimal notation (using the letters abcdef) in
	 the style [-]0xh.hhhhp±d; for A conversion the  prefix  0X,  the
	 letters  ABCDEF, and the exponent separator P is used.  There is
	 one hexadecimal digit before the decimal point, and  the  number
	 of  digits after it is equal to the precision.  The default
	 precision suffices for an exact representation of the value  if  an
	 exact  representation  in  base 2 exists and otherwise is
	 sufficiently large to distinguish values of type double.   The  digit
	 before  the decimal point is unspecified for non-normalized numbers,
	 and nonzero but otherwise unspecified for normalized  numbers.
       * **c**      If no l modifier is present, the int argument is converted to an
         unsigned char, and the resulting character is written.  If an  l
	 modifier  is  present,  the  wint\_t (wide character) argument is
	 converted to a multibyte sequence by a call  to  the  wcrtomb(3)
	 function, with a conversion state starting in the initial state,
	 and the resulting multibyte string is written.
       * **s**      If no l modifier is  present:  The  const  char  *  argument  is
         expected  to be a pointer to an array of character type (pointer
	 to a string).  Characters from the array are written up to  (but
	 not including) a terminating null byte ('\0'); if a precision is
	 specified, no more than the number specified are written.  If  a
	 precision  is given, no null byte need be present; if the preci‐
	 sion is not specified, or is greater than the size of the array,
	 the array must contain a terminating null byte.
	 
	 If  an  l  modifier  is present: The const wchar\_t * argument is
	 expected to be a pointer to an array of wide  characters.   Wide
	 characters  from the array are converted to multibyte characters
	 (each by a call to the wcrtomb(3) function,  with  a  conversion
	 state  starting in the initial state before the first wide char‐
	 acter), up to and including a terminating null  wide  character.
	 The  resulting  multibyte  characters are written up to (but not
	 including) the terminating null byte.  If a precision is  speci‐
	 fied,  no  more bytes than the number specified are written, but
	 no partial multibyte characters are written.  Note that the pre‐
	 cision determines the number of bytes written, not the number of
	 wide characters or screen positions.  The array must  contain  a
	 terminating null wide character, unless a precision is given and
	 it is so small that the  number  of  bytes  written  exceeds  it
	 before the end of the array is reached.
       * **C**      (Not in C99, but in SUSv2.)  Synonym for lc.  Don't use.
       * **S**      (Not in C99, but in SUSv2.)  Synonym for ls.  Don't use.
       * **p**      The  void * pointer argument is printed in hexadecimal (as if by
              %#x or %#lx).
       * **n**      The number of characters written so far is stored into the integer  indicated  by  the int * (or variant) pointer argument.  No
              argument is converted.
       * **m**      (Glibc extension.)  Print output of strerror(errno).   No  argument is required.
       * **%**      A  '%' is written.  No argument is converted.  The complete conversion specification is '%%'.


      
     */

  
  function (format) {
      var str=sprintf.apply(null, arguments);
      print(str);
      return str.length;
  }

