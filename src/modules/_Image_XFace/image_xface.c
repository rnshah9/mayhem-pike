/*
|| This file is part of Pike. For copyright information see COPYRIGHT.
|| Pike is distributed under GPL, LGPL and MPL. See the file COPYING
|| for more information.
*/

#include "global.h"
#include "module.h"
#include "config.h"

#include "pike_macros.h"
#include "object.h"
#include "constants.h"
#include "interpret.h"
#include "svalue.h"
#include "threads.h"
#include "array.h"
#include "mapping.h"
#include "pike_error.h"
#include "stralloc.h"
#include "buffer.h"
#include "operators.h"
#include "builtin_functions.h"
#include "pike_types.h"

/* Includes <gmp.h> */
#include "bignum.h"

#include "../Image/image.h"

#define sp Pike_sp

#ifdef DYNAMIC_MODULE
static struct program *image_program=NULL;
#else
extern struct program *image_program;
/* Image module is probably linked static too. */
#endif


/*
**! module Image
**! submodule XFace
**!
**! note
**!	This module uses <tt>libgmp</tt>.
*/


static const unsigned char tab[] = {
  0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0xc7, 0xfb, 0xa0, 0xe8, 0xa0, 0xf0,
  0x00, 0xd8, 0xf0, 0xfb, 0x00, 0x20, 0x00, 0x00, 0xb0, 0xf0, 0xc0, 0xfe,
  0x00, 0x00, 0x00, 0x80, 0x00, 0xb8, 0xa2, 0xf4, 0x00, 0x00, 0x00, 0xb0,
  0x00, 0x50, 0xff, 0xff, 0x00, 0x20, 0x00, 0xa0, 0x80, 0xfc, 0xf3, 0xff,
  0x08, 0x80, 0x01, 0x93, 0xf0, 0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
  0xd8, 0xf8, 0xff, 0xff, 0xf2, 0x2a, 0xe0, 0xf8, 0xea, 0xe2, 0xeb, 0xbc,
  0xff, 0xff, 0xfa, 0xf8, 0xfe, 0xff, 0xfe, 0xfe, 0xa0, 0xf0, 0x80, 0xf0,
  0xf0, 0xfa, 0xd9, 0xfb, 0xfe, 0xff, 0xfa, 0xb8, 0xfa, 0xff, 0xf0, 0xf8,
  0xf0, 0xfa, 0xc0, 0xf8, 0xf2, 0xfa, 0xef, 0xfe, 0xfe, 0xff, 0xb0, 0xf0,
  0xdf, 0xff, 0xef, 0xfd, 0xf0, 0xf2, 0xeb, 0xfc, 0xf2, 0xfe, 0xff, 0xff,
  0xe6, 0xfd, 0x6a, 0xa4, 0xf8, 0xfe, 0xf9, 0xff, 0x00, 0x00, 0x00, 0xa0,
  0xfa, 0xfe, 0x80, 0xfb, 0x28, 0x00, 0xa0, 0xf0, 0xe0, 0x45, 0x90, 0xf0,
  0x00, 0x00, 0x00, 0x00, 0xf0, 0xfa, 0x18, 0xeb, 0x29, 0x8e, 0x00, 0xa0,
  0xf8, 0xed, 0x30, 0xe0, 0xf0, 0xf0, 0x00, 0xf0, 0xf0, 0xf8, 0x21, 0xf1,
  0xa0, 0xa8, 0xa0, 0xf0, 0xf2, 0xff, 0xe1, 0xfb, 0xa0, 0x80, 0x08, 0x00,
  0xf0, 0xf0, 0x00, 0x10, 0xa0, 0x20, 0x20, 0x80, 0xf2, 0xff, 0xf9, 0xf1,
  0x52, 0x02, 0xfa, 0xfa, 0xff, 0x7f, 0xfb, 0xff, 0xfe, 0xef, 0xff, 0xfe,
  0xff, 0xff, 0xde, 0xff, 0xf0, 0xbf, 0xeb, 0xfa, 0xf2, 0xfe, 0xfe, 0xfb,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xee, 0xfb, 0xfe, 0xf2, 0xf7, 0xff, 0xff,
  0xee, 0xff, 0xff, 0xff, 0xf6, 0xff, 0xf0, 0xf2, 0xff, 0xff, 0xb9, 0xff,
  0xf0, 0xf7, 0xff, 0xfb, 0xf6, 0xff, 0xff, 0xff, 0xf2, 0xff, 0xb3, 0xf0,
  0xf2, 0xff, 0xff, 0xfb, 0x00, 0x00, 0x00, 0xd0, 0xa0, 0x40, 0x40, 0xf0,
  0x20, 0x00, 0x00, 0x30, 0x80, 0x60, 0x00, 0xf0, 0x04, 0xc0, 0x00, 0x00,
  0xa0, 0xf0, 0x02, 0x10, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x30, 0xf0,
  0x80, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x28, 0x80, 0xa0,
  0x80, 0xa8, 0xf5, 0xf0, 0x00, 0x80, 0x08, 0x00, 0x10, 0x00, 0x62, 0x30,
  0x04, 0x00, 0x11, 0x00, 0xf0, 0xa8, 0xff, 0xfb, 0x40, 0x00, 0x00, 0xf0,
  0xfe, 0xfa, 0xdb, 0xff, 0xf2, 0x7c, 0xa0, 0xf0, 0xfe, 0xef, 0xa9, 0xf2,
  0xb0, 0xf0, 0x80, 0xf0, 0xf2, 0xfa, 0xf9, 0xfb, 0xa4, 0x70, 0xb0, 0xb0,
  0xf2, 0xfe, 0xf1, 0xf0, 0xf0, 0x5f, 0x20, 0xf2, 0xf2, 0xff, 0xef, 0xee,
  0xe2, 0xb7, 0xa0, 0xf0, 0xff, 0xff, 0xfb, 0xff, 0xf2, 0xf6, 0x1b, 0xfa,
  0xf0, 0xfe, 0xfb, 0xfa, 0xe0, 0xf0, 0x29, 0xb0, 0xf8, 0xff, 0xff, 0xff,
  0x00, 0x40, 0x00, 0xc0, 0x62, 0xea, 0x80, 0xb0, 0x80, 0x10, 0x80, 0xf0,
  0xe2, 0x36, 0xb0, 0xf0, 0x40, 0x00, 0x00, 0x00, 0xd0, 0xf2, 0x00, 0x10,
  0xa0, 0x00, 0xa9, 0x80, 0xf0, 0xfe, 0x30, 0xf0, 0x80, 0x70, 0x00, 0x00,
  0xf0, 0x82, 0x00, 0x00, 0x20, 0x24, 0xb0, 0xf0, 0xf0, 0xfe, 0xf3, 0xfb,
  0x00, 0x00, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x60, 0x64, 0xf3, 0xa0,
  0xf3, 0xfe, 0xfb, 0xfb, 0x00, 0x00, 0xe8, 0xfa, 0xff, 0xbf, 0xff, 0xff,
  0x62, 0x90, 0xf2, 0xfa, 0xfe, 0xbf, 0xfb, 0xff, 0x50, 0x11, 0xe5, 0xfe,
  0xfe, 0xff, 0xff, 0xff, 0xf0, 0x20, 0xfb, 0xfe, 0xf2, 0xff, 0xf9, 0xff,
  0x70, 0x67, 0xfb, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xf0, 0x37, 0xf1, 0xf2,
  0xfe, 0xff, 0xfb, 0xff, 0xf0, 0xf3, 0xfb, 0xff, 0xf6, 0xfe, 0xff, 0xff,
  0xc0, 0x30, 0xb9, 0xf0, 0xfe, 0xff, 0xff, 0xff, 0xec, 0xce, 0x00, 0x98,
  0xea, 0xfe, 0xaf, 0xdf, 0x0e, 0xcc, 0x0f, 0x9f, 0xfe, 0xff, 0xff, 0xff,
  0x0a, 0x00, 0x00, 0x00, 0xa0, 0x00, 0xf0, 0xac, 0x1f, 0x02, 0x82, 0x9e,
  0x0f, 0x42, 0x0c, 0xfa, 0x0f, 0x00, 0x00, 0x00, 0x04, 0x0c, 0xcc, 0xbc,
  0x0e, 0x0a, 0xac, 0xcf, 0x8f, 0xce, 0xfc, 0xff, 0x0f, 0x10, 0x00, 0x04,
  0x82, 0x04, 0x80, 0xa8, 0x0e, 0x4a, 0x0a, 0x0a, 0xcc, 0xda, 0x9f, 0xff,
  0x0f, 0x6e, 0x4f, 0x20, 0x80, 0x0e, 0xf6, 0x75, 0x01, 0x08, 0x8e, 0x9f,
  0x8f, 0xff, 0xff, 0xff, 0x0f, 0x02, 0x00, 0x08, 0x28, 0x4c, 0xf7, 0xcf,
  0x8f, 0x88, 0x88, 0x88, 0xa8, 0x88, 0x88, 0x8c, 0x88, 0x88, 0x88, 0x8c,
  0x88, 0x88, 0xc8, 0x8c, 0x88, 0x8c, 0x88, 0x8c, 0x8c, 0x8c, 0xcc, 0xc8,
  0x88, 0x08, 0x88, 0x88, 0xe8, 0x88, 0x88, 0xe8, 0x0a, 0x00, 0x0f, 0x00,
  0x09, 0x00, 0x0b, 0x00, 0x0b, 0x00, 0x0f, 0x20, 0x77, 0x02, 0x8b, 0x00,
  0x0f, 0x00, 0x2f, 0x22, 0x0f, 0x80, 0x07, 0x0f, 0x5f, 0x57, 0x2f, 0xea,
  0x7f, 0xff, 0x0f, 0xff, 0x0f, 0x00, 0x09, 0x88, 0x08, 0x00, 0x88, 0xb3,
  0x0b, 0x80, 0x0a, 0xf0, 0x0f, 0x80, 0x00, 0xf0, 0x0b, 0x00, 0x2f, 0xaf,
  0xcf, 0xfb, 0xff, 0xff, 0x2f, 0xff, 0x8f, 0xf3, 0xbf, 0xff, 0x1f, 0xff,
  0x0f, 0x00, 0x0b, 0x00, 0x00, 0x80, 0x0a, 0x08, 0x0e, 0x00, 0x08, 0x84,
  0x0f, 0x88, 0x08, 0xea, 0x0f, 0x80, 0x2f, 0xa8, 0x8e, 0xff, 0x0f, 0xea,
  0x0e, 0xab, 0x2f, 0xfb, 0x2f, 0xff, 0x0f, 0xff, 0x0f, 0x00, 0x00, 0x04,
  0x0a, 0x80, 0x08, 0xea, 0x8b, 0x80, 0x4a, 0xff, 0x2f, 0xa0, 0x00, 0xfb,
  0x0b, 0x02, 0x0f, 0x8e, 0x0f, 0xee, 0x0f, 0xdf, 0x0f, 0xeb, 0x0f, 0xff,
  0x2f, 0xeb, 0x0b, 0xff, 0x0f, 0x88, 0x0c, 0xec, 0x8f, 0xae, 0xaa, 0xae,
  0xee, 0x0f, 0x08, 0x08, 0x88, 0x0f, 0x8c, 0xcf, 0xff, 0x2f, 0x44
};

/*
00   01   02   10   20   30   40   11   21   31   41   12   22   32   42
0000 4096 4224 4228 4740 4804 5060 6084 6116 6124 6156 6220 6222 6223 6227
*/

static const int taboffs[] = { 0, 4740, 4228, 5060, 4224, 6222, 6220, 6227,
			       4096, 6116, 6084, 6156 };

static void xform(unsigned char *i, unsigned char *o)
{
  int x, y, X, Y, p, n;

  for(y=0; y<48; y++)
    for(x=0; x<48; x++) {
      n=0;
      for(X=(x<3? 1:x-2); X<x+3; X++)
	for(Y=(y<3? 1:y-2); Y<=y; Y++)
	  if((Y<y || X<x) && X<=48)
	    n = (n<<1)|i[Y*48+X];

      if((p=x)==47)
	p = 3;
      else if(x>2)
	p = 0;
      if(y==1)
	p += 4;
      else if(y==2)
	p += 8;
      n += taboffs[p];
      *o++ ^= (tab[n>>3]>>(n&7))&1;
    }
}

static const unsigned int topprob[4][6] = {
  {1, 255, 251, 0, 4, 251},
  {1, 255, 200, 0, 55, 200},
  {33, 223, 159, 0, 64, 159},
  {131, 0, 0, 0, 125, 131}
};

static const unsigned int botprob[32] = {
  0, 0, 38, 0, 38, 38, 13, 152,
  38, 76, 13, 165, 13, 178, 6, 230,
  38, 114, 13, 191, 13, 204, 6, 236,
  13, 217, 6, 242, 5, 248, 3, 253
};

static int pop(mpz_t val, const unsigned int *p)
{
  unsigned long int n;
  int r = 0;
  mpz_t dum;

  mpz_init(dum);
  n = mpz_fdiv_qr_ui(val, dum, val, 256);
  mpz_clear(dum);
  while(n<p[1] || n>=p[0]+p[1]) {
    r++;
    p += 2;
  }
  mpz_mul_ui(val, val, *p++);
  mpz_add_ui(val, val, n-*p);
  return r;
}

static void push(mpz_t val, const unsigned int *p, int r)
{
  unsigned long int n;
  mpz_t dum;

  p += r<<1;
  mpz_init(dum);
  n = mpz_fdiv_qr_ui(val, dum, val, p[0]);
  mpz_clear(dum);

  mpz_mul_ui(val, val, 256);
  mpz_add_ui(val, val, n+p[1]);
}

static void popg(mpz_t val, unsigned char *face, int s)
{
  if(s>=4) {
    s>>=1;
    popg(val, face, s);
    popg(val, face+s, s);
    popg(val, face+s*48, s);
    popg(val, face+s*49, s);
  } else {
    int p = pop(val, botprob);
    face[0]=p&1; p>>=1;
    face[1]=p&1; p>>=1;
    face[48]=p&1; p>>=1;
    face[49]=p&1;
  }
}

static void pushg(mpz_t val, unsigned char *face, int s)
{
  if(s>=4) {
    s>>=1;
    pushg(val, face+s*49, s);
    pushg(val, face+s*48, s);
    pushg(val, face+s, s);
    pushg(val, face, s);
  } else
    push(val, botprob,
	 (face[0])|((face[1])<<1)|((face[48])<<2)|((face[49])<<3));
}

static void uncomp(mpz_t val, unsigned char *face, int s, int l)
{
  switch(pop(val, topprob[l])) {
  case 0:
    popg(val, face, s);
    break;
  case 1:
    s>>=1;
    l++;
    uncomp(val, face, s, l);
    uncomp(val, face+s, s, l);
    uncomp(val, face+s*48, s, l);
    uncomp(val, face+s*49, s, l);
    break;
  }
}

static int all_white(unsigned char *face, int s)
{
  int i, j;
  for(i=0; i<s; i++) {
    for(j=0; j<s; j++)
      if(face[j])
	return 0;
    face += 48;
  }
  return 1;
}

static int all_black(unsigned char *face, int s)
{
  if(s>=4) {
    s>>=1;
    return all_black(face, s) && all_black(face+s, s) &&
      all_black(face+s*48, s) && all_black(face+s*49, s);
  } else
    return face[0] || face[1] || face[48] || face[49];
}

static void comp(mpz_t val, unsigned char *face, int s, int l)
{
  if(all_white(face, s))
    push(val, topprob[l], 2);
  else if(all_black(face, s)) {
    pushg(val, face, s);
    push(val, topprob[l], 0);
  } else {
    s>>=1;
    l++;
    comp(val, face+s*49, s, l);
    comp(val, face+s*48, s, l);
    comp(val, face+s, s, l);
    comp(val, face, s, l);
    push(val, topprob[l-1], 1);
  }
}

static void decodeface(char *data, INT32 len, rgb_group *out)
{
  unsigned char face[48][48];
  int i, j;
  mpz_t val;

  mpz_init(val);
  mpz_set_ui(val, 0);
  while(len--)
    if(*data>='!' && *data<='~') {
      mpz_mul_ui(val, val, 94);
      mpz_add_ui(val, val, *data++-'!');
    } else
      data++;
  memset(face, 0, sizeof(face));
  for(i=0; i<3; i++)
    for(j=0; j<3; j++)
      uncomp(val, &face[i*16][j*16], 16, 0);
  mpz_clear(val);

  xform((unsigned char *)face, (unsigned char *)face);
  for(i=0; i<48; i++)
    for(j=0; j<48; j++) {
      if(face[i][j])
	out->r = out->g = out->b = 0;
      else
	out->r = out->g = out->b = 0xff;
      out++;
    }
}

static struct pike_string *encodeface(rgb_group *in)
{
  unsigned char face[48][48], newface[48][48];
  int i, j;
  unsigned long int n;
  mpz_t val, dum;
  struct byte_buffer buf;

  for(i=0; i<48; i++)
    for(j=0; j<48; j++) {
      if(in->r || in->g || in->b)
	face[i][j] = 0;
      else
	face[i][j] = 1;
      in++;
    }
  memcpy(newface, face, sizeof(face));
  xform((unsigned char *)face, (unsigned char *)newface);
  mpz_init(val);
  mpz_set_ui(val, 0);
  for(i=2; i>=0; --i)
    for(j=2; j>=0; --j)
      comp(val, &newface[i*16][j*16], 16, 0);
  buffer_init( &buf );
  mpz_init(dum);
  i = 0;
  while(mpz_cmp_ui(val, 0)) {
    n = mpz_fdiv_qr_ui(val, dum, val, 94);
    buffer_add_char( &buf ,  (char)(n+'!'));
    i++;
  }
  if (i==0)
    buffer_add_char( &buf ,  '!');
  mpz_clear(dum);
  mpz_clear(val);
  return buffer_finish_pike_string( &buf );
}


/*
**! method object decode(string data)
**! method object decode(string data, mapping options)
**! 	Decodes an X-Face image.
**!
**!     The <tt>options</tt> argument may be a mapping
**!	containing zero options.
**!
*/

static void image_xface_decode(INT32 args)
{
  struct object *o;
  struct image *img;

  if(args<1 || TYPEOF(sp[-args]) != T_STRING)
    Pike_error("Illegal arguments\n");

  o=clone_object(image_program,0);
  img=get_storage(o,image_program);
  if (!img) Pike_error("image no image? foo?\n"); /* should never happen */
  img->img=malloc(sizeof(rgb_group)*48*48);
  if (!img->img) {
    free_object(o);
    Pike_error("Out of memory\n");
  }
  img->xsize=48;
  img->ysize=48;
  decodeface(sp[-args].u.string->str, sp[-args].u.string->len, img->img);
  pop_n_elems(args);
  push_object(o);
}


/*
**! method string encode(object img)
**! method string encode(object img, mapping options)
**! 	Encodes an X-Face image.
**!
**!     The <tt>img</tt> argument must be an image of the dimensions
**!     48 by 48 pixels.  All non-black pixels will be considered white.
**!
**!     The <tt>options</tt> argument may be a mapping
**!	containing zero options.
**!
*/

static void image_xface_encode(INT32 args)
{
  struct image *img=NULL;
  struct pike_string *res;

  if (args<1
      || TYPEOF(sp[-args]) != T_OBJECT
      || !(img=get_storage(sp[-args].u.object,image_program))
      || (args>1 && TYPEOF(sp[1-args]) != T_MAPPING))
    Pike_error("Illegal arguments\n");

  if (!img->img)
    Pike_error("Given image is empty.\n");

  if (img->xsize != 48 || img->ysize != 48)
    Pike_error("Wrong image dimensions (must be 48 by 48).\n");

  res = encodeface(img->img);

  pop_n_elems(args);
  if (res == NULL)
    push_int(0);
  else {
    push_string(res);
    f_reverse(1);
  }
}

/*
**! method object decode_header(string data)
**! method object decode_header(string data, mapping options)
**! 	Decodes an X-Face image header.
**!
**!	<pre>
**!	    "xsize":int
**!	    "ysize":int
**!		size of image
**!	    "type":"image/x-xface"
**!		file type information
**!	</pre>
**!
**!     The <tt>options</tt> argument may be a mapping
**!	containing zero options.
**!
**! note
**!	There aint no such thing as a X-Face image header.
**!	This stuff tells the characteristics of an X-Face image.
**!
*/

static void image_xface_decode_header(INT32 args)
{
  if(args<1 || TYPEOF(sp[-args]) != T_STRING)
    Pike_error("Illegal arguments\n");

  pop_n_elems(args);

  ref_push_string(literal_type_string);
  push_static_text("image/x-xface");

  push_static_text("xsize");
  push_int(48);

  push_static_text("ysize");
  push_int(48);

  f_aggregate_mapping(6);
}


/*** module init & exit & stuff *****************************************/

PIKE_MODULE_EXIT
{
}

PIKE_MODULE_INIT
{
#ifdef DYNAMIC_MODULE
   push_static_text("Image.Image");
   SAFE_APPLY_MASTER("resolv",1);
   if (TYPEOF(sp[-1]) == T_PROGRAM)
      image_program=program_from_svalue(sp-1);
   pop_stack();
#endif /* DYNAMIC_MODULE */

   if (image_program)
   {
      /* function(string,void|mapping(string:int):object) */
  ADD_FUNCTION("decode",image_xface_decode,tFunc(tStr tOr(tVoid,tMap(tStr,tInt)),tObj),0);
      /* function(string,void|mapping(string:int):object) */
  ADD_FUNCTION("decode_header",image_xface_decode_header,tFunc(tStr tOr(tVoid,tMap(tStr,tInt)),tObj),0);
      /* function(object,void|mapping(string:int):string) */
  ADD_FUNCTION("encode",image_xface_encode,tFunc(tObj tOr(tVoid,tMap(tStr,tInt)),tStr),0);
   }
}