dnl dps_float_byte
AC_DEFUN([dps_float_byte],
    [AC_MSG_CHECKING(whether ${CXX} supports our byte<->float conversions)
        AC_CACHE_VAL(mdl_cv_have_float_byte_support,
            [ AC_TRY_RUN(
	changequote(<<<, >>>)dnl
	<<<
    typedef long double float_t;
    typedef long int32_t;
    typedef char uint8_t;
	
	//float to bits conversion utilities...
	union clvalue {
		int32_t     i;
		float		f; //must use a float type, else types dont match up
	};
	
	int32_t floatToIntBits(float_t value)
	{
		clvalue u;
		int32_t e, f;
		u.f = value;
		e = u.i & 0x7f800000;
		f = u.i & 0x007fffff;

		if (e == 0x7f800000 && f != 0)
		u.i = 0x7fc00000;
	    
		return u.i;
	}
	float_t intBitsToFloat(int32_t bits)
	{
		clvalue u;
		u.i = bits;
		return u.f;
	}
	
	float_t byteToFloat(uint8_t b) {
      if (b == 0)                                   // zero is a special case
         return 0.0f;
      int32_t mantissa = b & 7;
      int32_t exponent = (b >> 3) & 31;
      int32_t bits = ((exponent+(63-15)) << 24) | (mantissa << 21);
      return intBitsToFloat(bits);
   }

   uint8_t floatToByte(float_t f) {
      if (f < 0.0f)                                 // round negatives up to zero
         f = 0.0f;

      if (f == 0.0f)                                // zero is a special case
         return 0;

      int32_t bits = floatToIntBits(f);           // parse float_t into parts
      int32_t mantissa = (bits & 0xffffff) >> 21;
      int32_t exponent = (((bits >> 24) & 0x7f) - 63) + 15;

      if (exponent > 31) {                          // overflow: use max value
         exponent = 31;
         mantissa = 7;
      }

      if (exponent < 0) {                           // underflow: use min value
         exponent = 0;
         mantissa = 1;
      }

      return (uint8_t)((exponent << 3) | mantissa);    // pack into a uint8_t
   }
	
	int main(void)
	{
	    //well know conversion
	    if ( floatToByte(0.5f) != 120 )
	        return 1;
	    
	    //converting back works?
	    if ( floatToByte(byteToFloat(57)) != 57 )
	        return 1;

        return 0;
	}
>>>
	changequote([, ]), dps_float_byte_fail=0, dps_float_byte_fail=1,
	dps_float_byte_fail=2
    )]
)
	
    if test $dps_float_byte_fail -eq 0; then
      AC_MSG_RESULT([yes])
    elif test $dps_float_byte_fail -eq 1; then
      AC_MSG_RESULT([no])
      AC_DEFINE([HAVE_NO_FLOAT_BYTE], [], [Does not support new float byte<->float conversions])
    else
      AC_MSG_RESULT([unknown, assuming broken])
      AC_DEFINE([HAVE_NO_FLOAT_BYTE], [], [Does not support new float byte<->float conversions])
    fi

])