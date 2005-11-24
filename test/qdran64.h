#include <stdint.h>

uint64_t qdran64low, qdran64high;

void qdran64_init(uint64_t seedlow, uint64_t seedhigh)
{
  qdran64low = seedlow;
  qdran64high = seedhigh;
}


void qdran64(uint64_t *ranlow, uint64_t *ranhigh)
{
  const uint64_t ia = 0x27bb2ee687b0b0fdULL;
  const uint64_t ic = 0x00000000b504f32dULL;
  
  qdran64low *= ia;
  qdran64low += ic;
  qdran64high *= ia;
  qdran64high += ic;
  
  (*ranlow) = qdran64low;
  (*ranhigh) = qdran64high;
}


#ifdef _STD_C99_COMPLEX
#include <complex.h>
void qdran64z(complex *ranz)
{
  union { double x; uint64_t i; } re, im;
  uint64_t * ranlow = &re.i;
  uint64_t * ranhigh = &im.i;

  re.x = creal(*ranz);
  im.x = cimag(*ranz);

  qdran64(ranlow, ranhigh);

  (*ranlow) &= 0x000fffffffffffffULL;
  (*ranlow) |= 0x3ff0000000000000ULL;
  (*ranhigh) &= 0x000fffffffffffffULL;
  (*ranhigh) |= 0x3ff0000000000000ULL;

  *ranz = re.x + I * im.x;
}
#endif

void qdran64_2d(double *rand1, double *rand2)
{
  union { double x; uint64_t i; } re, im;
  uint64_t * ranlow = &re.i;
  uint64_t * ranhigh = &im.i;

  re.x = (*rand1);
  im.x = (*rand2);

  qdran64(ranlow, ranhigh);

  (*ranlow) &= 0x000fffffffffffffULL;
  (*ranlow) |= 0x3ff0000000000000ULL;
  (*ranhigh) &= 0x000fffffffffffffULL;
  (*ranhigh) |= 0x3ff0000000000000ULL;

  *rand1 = re.x;
  *rand2 = im.x;
}

/* void qdran32d(double *rand) { */
/*   complex tmp; */
/*   qdran64z(&tmp); */
/*   *rand=tmp; */
/* } */
