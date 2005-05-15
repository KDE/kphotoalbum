/*
   Raw Photo Parser
   Copyright 2004 by Dave Coffin, dcoffin a cybercom o net

   This program extracts thumbnail images (preferably JPEGs)
   from any raw digital camera formats that have them, and
   shows table contents.

   $Revision: 1.36 $
   $Date: 2005/05/10 21:43:10 $
 */

/* Hacked for thumbnail extraction in KDE by 
   Steffen Hansen <hansen@kde.org>

   Based on parse.c and parts of dcraw.c by Dave Coffin
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef WIN32
#include <winsock2.h>
typedef __int64 INT64;
#else
#include <netinet/in.h>
typedef long long INT64;
#endif

/*
   TIFF and CIFF data blocks can be quite large.
   Display only the first DLEN bytes.
 */
#ifndef DLEN
#define DLEN 768
#endif

typedef unsigned char uchar;
/*typedef unsigned short ushort;*/

FILE *ifp;
short order;
char *fname;
char make[128], model[128], model2[128], thumb_head[128];
int width, height, offset, length, bps, is_dng;
int thumb_offset, thumb_length, thumb_layers;
static int flip = 0;

struct decode {
  struct decode *branch[2];
  int leaf;
} first_decode[640], *free_decode;

/*
   Get a 2-byte integer, making no assumptions about CPU byte order.
   Nor should we assume that the compiler evaluates left-to-right.
 */
ushort get2()
{
  uchar a, b;

  a = fgetc(ifp);  b = fgetc(ifp);

  if (order == 0x4949)		/* "II" means little-endian */
    return a | b << 8;
  else				/* "MM" means big-endian */
    return a << 8 | b;
}

/*
   Same for a 4-byte integer.
 */
int get4()
{
  uchar a, b, c, d;

  a = fgetc(ifp);  b = fgetc(ifp);
  c = fgetc(ifp);  d = fgetc(ifp);

  if (order == 0x4949)
    return a | b << 8 | c << 16 | d << 24;
  else
    return a << 24 | b << 16 | c << 8 | d;
}

void tiff_dump(int base, int tag, int type, int count, int level)
{
  int save, j, num, den;
  uchar c;
  int size[] = { 1,1,1,2,4,8,1,1,2,4,8,4,8 };

  if (count * size[type < 13 ? type:0] > 4)
    fseek (ifp, get4()+base, SEEK_SET);
  save = ftell(ifp);
  fseek (ifp, save, SEEK_SET);
}

void nikon_decrypt (uchar ci, uchar cj, int tag, int i, int size, uchar *buf)
{
}

int parse_tiff_ifd (int base, int level);

void nef_parse_makernote (base)
{
  int offset=0, entries, tag, type, count, val, save;
  unsigned serial=0, key=0;
  uchar buf91[630], buf97[608], buf98[31];
  short sorder;
  char buf[10];

/*
   The MakerNote might have its own TIFF header (possibly with
   its own byte-order!), or it might just be a table.
 */
  sorder = order;
  fread (buf, 1, 10, ifp);
  if (!strcmp (buf,"Nikon")) {	/* starts with "Nikon\0\2\0\0\0" ? */
    base = ftell(ifp);
    order = get2();		/* might differ from file-wide byteorder */
    val = get2();		/* should be 42 decimal */
    offset = get4();
    fseek (ifp, offset-8, SEEK_CUR);
  } else if (!strncmp (buf,"FUJIFILM",8) ||
	     !strcmp  (buf,"Panasonic")) {
    order = 0x4949;
    fseek (ifp,  2, SEEK_CUR);
  } else if (!strcmp (buf,"OLYMP") ||
	     !strcmp (buf,"LEICA") ||
	     !strcmp (buf,"EPSON"))
    fseek (ifp, -2, SEEK_CUR);
  else if (!strcmp (buf,"AOC"))
    fseek (ifp, -4, SEEK_CUR);
  else
    fseek (ifp, -10, SEEK_CUR);

  entries = get2();
  if (entries > 100) return;
  while (entries--) {
    save = ftell(ifp);
    tag  = get2();
    type = get2();
    count= get4();
    tiff_dump (base, tag, type, count, 2);
    if (tag == 0x1d)
      fscanf (ifp, "%d", &serial);
    if (tag == 0x91)
      fread (buf91, sizeof buf91, 1, ifp);
    if (tag == 0x97)
      fread (buf97, sizeof buf97, 1, ifp);
    if (tag == 0x98)
      fread (buf98, sizeof buf98, 1, ifp);
    if (tag == 0xa7)
      key = fgetc(ifp)^fgetc(ifp)^fgetc(ifp)^fgetc(ifp);

    if (tag == 0x100 && type == 7 && !strncmp(make,"OLYMPUS",7)) {
      thumb_offset = ftell(ifp);
      thumb_length = count;
    }
    if (tag == 0x280 && type == 1) {	/* EPSON */
      strcpy (thumb_head, "\xff");
      thumb_offset = ftell(ifp)+1;
      thumb_length = count-1;
    }
    if (strstr(make,"Minolta") || strstr(make,"MINOLTA")) {
      switch (tag) {
	case 0x81:
	  thumb_offset = ftell(ifp);
	  thumb_length = count;
	  break;
	case 0x88:
	  thumb_offset = get4() + base;
	  break;
	case 0x89:
	  thumb_length = get4();
      }
    }
    if (!strcmp (buf,"OLYMP") && tag >> 8 == 0x20)
      parse_tiff_ifd (base, 3);
    fseek (ifp, save+12, SEEK_SET);
  }
  nikon_decrypt (serial, key, 0x91,   4, sizeof buf91, buf91);
  nikon_decrypt (serial, key, 0x97, 284, sizeof buf97, buf97);
  nikon_decrypt (serial, key, 0x98,   4, sizeof buf98, buf98);
  order = sorder;
}

void nef_parse_exif(int base)
{
  int entries, tag, type, count, save;

  entries = get2();
  while (entries--) {
    save = ftell(ifp);
    tag  = get2();
    type = get2();
    count= get4();
    tiff_dump (base, tag, type, count, 1);
    if (tag == 0x927c)
      nef_parse_makernote (base);
    fseek (ifp, save+12, SEEK_SET);
  }
}

int parse_tiff_ifd (int base, int level)
{
  int entries, tag, type, count, slen, save, save2, val, i;
  int comp=0;
  static const int flip_map[] = { 0,1,3,2,4,6,7,5 };

  entries = get2();
  if (entries > 255) return 1;
  while (entries--) {
    save = ftell(ifp);
    tag  = get2();
    type = get2();
    count= get4();
    slen = count;
    if (slen > 128) slen = 128;

    tiff_dump (base, tag, type, count, level);

    save2 = ftell(ifp);
    if (type == 3)			/* short int */
      val = get2();
    else
      val = get4();
    fseek (ifp, save2, SEEK_SET);

    if (tag > 50700 && tag < 50800)
      is_dng = 1;

    if (level == 3) {			/* Olympus E-1 and E-300 */
      if (type == 4) {
	if (tag == 0x101)
	  thumb_offset = val;
	else if (tag == 0x102)
	  thumb_length = val;
      }
      goto cont;
    }
    switch (tag) {
      case 0x100:			/* ImageWidth */
	if (!width)  width = val;
	break;
      case 0x101:			/* ImageHeight */
	if (!height) height = val;
	break;
      case 0x102:			/* Bits per sample */
	if (bps) break;
	bps = val;
	if (count == 1)
	  thumb_layers = 1;
	break;
      case 0x103:			/* Compression */
	comp = val;
	break;
      case 0x10f:			/* Make tag */
	fgets (make, slen, ifp);
	break;
      case 0x110:			/* Model tag */
	fgets (model, slen, ifp);
	break;
      case 33405:			/* Model2 tag */
	fgets (model2, slen, ifp);
	break;
      case 0x111:			/* StripOffset */
	if (!offset || is_dng) offset = val;
	break;
	  case 0x112:           /* Orientation */
		flip = flip_map[(val-1) & 7];
		break;
      case 0x117:			/* StripByteCounts */
	if (!length || is_dng) length = val;
	if (offset > val && !strncmp(make,"KODAK",5) && !is_dng)
	  offset -= val;
	break;
      case 0x14a:			/* SubIFD tag */
	save2 = ftell(ifp);
	for (i=0; i < count; i++) {
	  fseek (ifp, save2 + i*4, SEEK_SET);
	  fseek (ifp, get4()+base, SEEK_SET);
	  parse_tiff_ifd (base, level+1);
	}
	break;
      case 0x201:
	if (strncmp(make,"OLYMPUS",7) || !thumb_offset)
	  thumb_offset = val;
	break;
      case 0x202:
	if (strncmp(make,"OLYMPUS",7) || !thumb_length)
	  thumb_length = val;
	break;
      case 34665:
	fseek (ifp, get4()+base, SEEK_SET);
	nef_parse_exif (base);
	break;
      case 50706:
	is_dng = 1;
    }
cont:
    fseek (ifp, save+12, SEEK_SET);
  }
  if ((comp == 6 && !strcmp(make,"Canon")) ||
      (comp == 7 && is_dng)) {
    thumb_offset = offset;
    thumb_length = length;
  }
  return 0;
}

/*
   Parse a TIFF file looking for camera model and decompress offsets.
 */
void parse_tiff (int base)
{
  int doff, spp=3, ifd=0;

  width = height = offset = length = bps = is_dng = 0;
  fseek (ifp, base, SEEK_SET);
  order = get2();
  if (order != 0x4949 && order != 0x4d4d) return;
  get2();
  while ((doff = get4())) {
    fseek (ifp, doff+base, SEEK_SET);
    printf ("IFD #%d:\n", ifd++);
    if (parse_tiff_ifd (base, 0)) break;
  }
  if (is_dng) return;

  if (strncmp(make,"KODAK",5))
    thumb_layers = 0;
  if (!strncmp(make,"Kodak",5)) {
    fseek (ifp, 12+base, SEEK_SET);
    puts ("\nSpecial Kodak image directory:");
    parse_tiff_ifd (base, 0);
  }
  if (!strncmp(model,"DCS460A",7)) {
    spp = 1;
    thumb_layers = 0;
  }
  if (!thumb_length && offset) {
    thumb_offset = offset;
    sprintf (thumb_head, "P%d %d %d %d\n",
	spp > 1 ? 6:5, width, height, (1 << bps) - 1);
    thumb_length = width * height * spp * ((bps+7)/8);
  }
}

void parse_minolta()
{
  int data_offset, save, tag, len;

  fseek (ifp, 4, SEEK_SET);
  data_offset = get4() + 8;
  while ((save=ftell(ifp)) < data_offset) {
    tag = get4();
    len = get4();
    printf ("Tag %c%c%c offset %06x length %06x\n",
	tag>>16, tag>>8, tag, save, len);
    switch (tag) {
      case 0x545457:				/* TTW */
	parse_tiff (ftell(ifp));
    }
    fseek (ifp, save+len+8, SEEK_SET);
  }
  strcpy (thumb_head, "\xff");
  thumb_offset++;
  thumb_length--;
}

/*
   Parse the CIFF structure.
 */
void parse_ciff (int offset, int length, int level)
{
  int tboff, nrecs, i, j, type, len, dlen, roff, aoff=0, save;
  char c, name[256];

  fseek (ifp, offset+length-4, SEEK_SET);
  tboff = get4() + offset;
  fseek (ifp, tboff, SEEK_SET);
  nrecs = get2();
  if (nrecs > 100) return;
  for (i = 0; i < nrecs; i++) {
    save = ftell(ifp);
    type = get2();
    if (type & 0x4000) {
      len = 8;
      type &= 0x3fff;
    } else {
      len  = get4();
      roff = get4();
      aoff = offset + roff;
      fseek (ifp, aoff, SEEK_SET);
    }
    if (type == 0x0032)			/* display as words */
	type |= 0x1000;
    dlen = len < DLEN ? len:DLEN;
    switch (type >> 8) {
      case 0x28:
      case 0x30:
	parse_ciff (aoff, len, level+1);
	fseek (ifp, save+10, SEEK_SET);
    }
    fseek (ifp, save+10, SEEK_SET);
    if (type == 0x080a) {		/* Get the camera name */
      fseek (ifp, aoff, SEEK_SET);
      fread (name, 256, 1, ifp);
      strcpy (make, name);
      strcpy (model, name + strlen(make)+1);
    }
    if (type == 0x2007) {		/* Found the JPEG thumbnail */
      thumb_offset = aoff;
      thumb_length = len;
    }
	if (type == 0x1810) {               /* Get the rotation */
      fseek (ifp, aoff+12, SEEK_SET);
      flip = get4();
	}
  }
}

void parse_mos(int level)
{
  uchar data[256];
  int i, j, skip, save;
  char *cp;

  save = ftell(ifp);
  while (1) {
    fread (data, 1, 8, ifp);
    if (strcmp(data,"PKTS")) break;
    strcpy (model, "Valeo");
    fread (data, 1, 40, ifp);
    skip = get4();
    if (!strcmp(data,"icc_camera_to_tone_matrix")) {
      for (i=0; i < skip/4; i++) {
	j = get4();
      }
      continue;
    }
    if (!strcmp(data,"JPEG_preview_data")) {
      thumb_head[0] = 0;
      thumb_offset = ftell(ifp);
      thumb_length = skip;
    }
    fread (data, 1, sizeof data, ifp);
    fseek (ifp, -sizeof data, SEEK_CUR);
    data[sizeof data - 1] = 0;
    while ((cp=index(data,'\n')))
      *cp = ' ';
    parse_mos(level+2);
    fseek (ifp, skip, SEEK_CUR);
  }
  fseek (ifp, save, SEEK_SET);
}

void parse_rollei()
{
  char line[128], *val;

  fseek (ifp, 0, SEEK_SET);
  do {
    fgets (line, 128, ifp);
    fputs (line, stdout);
    if ((val = strchr(line,'=')))
      *val++ = 0;
    else
      val = line + strlen(line);
    if (!strcmp(line,"HDR"))
      thumb_offset = atoi(val);
    if (!strcmp(line,"TX "))
      width = atoi(val);
    if (!strcmp(line,"TY "))
      height = atoi(val);
  } while (strncmp(line,"EOHD",4));
  strcpy (make, "Rollei");
  strcpy (model, "d530flex");
  thumb_length = width*height*2;
}

void rollei_decode (FILE *tfp)
{
  ushort data;
  int row, col;

  fseek (ifp, thumb_offset, SEEK_SET);
  fprintf (tfp, "P6\n%d %d\n255\n", width, height);
  for (row=0; row < height; row++)
    for (col=0; col < width; col++) {
      fread (&data, 2, 1, ifp);
      data = ntohs(data);
      putc (data << 3, tfp);
      putc (data >> 5  << 2, tfp);
      putc (data >> 11 << 3, tfp);
    }
}

void get_utf8 (int offset, char *buf, int len)
{
  ushort c;
  char *cp;

  fseek (ifp, offset, SEEK_SET);
  for (cp=buf; (c = get2()) && cp+3 < buf+len; ) {
    if (c < 0x80)
      *cp++ = c;
    else if (c < 0x800) {
      *cp++ = 0xc0 + (c >> 6);
      *cp++ = 0x80 + (c & 0x3f);
    } else {
      *cp++ = 0xe0 + (c >> 12);
      *cp++ = 0x80 + (c >> 6 & 0x3f);
      *cp++ = 0x80 + (c & 0x3f);
    }
  }
  *cp = 0;
}

ushort sget2 (uchar *s)
{
  return s[0] + (s[1]<<8);
}

int sget4 (uchar *s)
{
  return s[0] + (s[1]<<8) + (s[2]<<16) + (s[3]<<24);
}

void parse_foveon()
{
  int entries, img=0, off, len, tag, save, i, j, k, pent, poff[256][2];
  char name[128], value[128], camf[0x20000], *pos, *cp, *dp;
  unsigned val, key, type, num, ndim, dim[3];

  order = 0x4949;			/* Little-endian */
  fseek (ifp, -4, SEEK_END);
  fseek (ifp, get4(), SEEK_SET);
  if (get4() != 0x64434553) {	/* SECd */
    printf ("Bad Section identifier at %6x\n", (int)ftell(ifp)-4);
    return;
  }
  get4();
  entries = get4();
  while (entries--) {
    off = get4();
    len = get4();
    tag = get4();
    save = ftell(ifp);
    fseek (ifp, off, SEEK_SET);
    if (get4() != (0x20434553 | (tag << 24))) {
      printf ("Bad Section identifier at %6x\n", off);
      goto next;
    }
    val = get4();
    switch (tag) {
      case 0x32414d49:			/* IMA2 */
      case 0x47414d49:			/* IMAG */
	if (++img == 2) {		/* second image */
	  thumb_offset = off;
	  thumb_length = 1;
	}
	printf ("type %d, "	, get4());
	printf ("format %2d, "	, get4());
	printf ("columns %4d, "	, get4());
	printf ("rows %4d, "	, get4());
	printf ("rowsize %d\n"	, get4());
	break;
      case 0x464d4143:			/* CAMF */
	printf ("type %d, ", get4());
	get4();
	for (i=0; i < 4; i++)
	  putchar(fgetc(ifp));
	val = get4();
	printf (" version %d.%d:\n",val >> 16, val & 0xffff);
	key = get4();
	if ((len -= 28) > 0x20000)
	  len = 0x20000;
	fread (camf, 1, len, ifp);
	for (i=0; i < len; i++) {
	  key = (key * 1597 + 51749) % 244944;
	  val = key * (INT64) 301593171 >> 24;
	  camf[i] ^= ((((key << 8) - val) >> 1) + val) >> 17;
	}
	for (pos=camf; (unsigned) (pos-camf) < len; pos += sget4(pos+8)) {
	  if (strncmp (pos, "CMb", 3)) {
	    printf("Bad CAMF tag \"%.4s\"\n", pos);
	    break;
	  }
	  val = sget4(pos+4);
	  printf ("  %4.4s version %d.%d: ", pos, val >> 16, val & 0xffff);
	  switch (pos[3]) {
	    case 'M':
	      cp = pos + sget4(pos+16);
	      type = sget4(cp);
	      ndim = sget4(cp+4);
	      dim[0] = dim[1] = dim[2] = 1;
	      printf ("%d-dimensonal array %s of type %d:\n    Key: (",
		ndim, pos+sget4(pos+12), sget4(cp));
	      dp = pos + sget4(cp+8);
	      for (i=ndim; i--; ) {
		cp += 12;
		dim[i] = sget4(cp);
		printf ("%s %d%s", pos+sget4(cp+4), dim[i], i ? ", ":")\n");
	      }
	      for (i=0; i < dim[2]; i++) {
		for (j=0; j < dim[1]; j++) {
		  printf ("    ");
		  for (k=0; k < dim[0]; k++)
		    switch (type) {
		      case 0:
		      case 6:
			printf ("%7d", sget2(dp));
			dp += 2;
			break;
		      case 1:
		      case 2:
			printf (" %d", sget4(dp));
			dp += 4;
			break;
		      case 3:
			val = sget4(dp);
			printf (" %9f", *(float *)(&val));
			dp += 4;
		    }
		  printf ("\n");
		}
		printf ("\n");
	      }
	      break;
	    case 'P':
	      val = sget4(pos+16);
	      num = sget4(pos+val);
	      printf ("%s, %d parameters:\n", pos+sget4(pos+12), num);
	      cp = pos+val+8 + num*8;
	      for (i=0; i < num; i++) {
		val += 8;
		printf ("    %s = %s\n", cp+sget4(pos+val), cp+sget4(pos+val+4));
	      }
	      break;
	    case 'T':
	      cp = pos + sget4(pos+16);
	      printf ("%s = %.*s\n", pos+sget4(pos+12), sget4(cp), cp+4);
	      break;
	    default:
	      printf ("\n");
	  }
	}
	break;
      case 0x504f5250:			/* PROP */
	printf ("entries %d, ", pent=get4());
	printf ("charset %d, ", get4());
	get4();
	printf ("nchars %d\n", get4());
	off += pent*8 + 24;
	if (pent > 256) pent=256;
	for (i=0; i < pent*2; i++)
	  poff[0][i] = off + get4()*2;
	for (i=0; i < pent; i++) {
	  get_utf8 (poff[i][0], name, 128);
	  get_utf8 (poff[i][1], value, 128);
	  printf ("  %s = %s\n", name, value);
	  if (!strcmp (name,"CAMMANUF"))
	    strcpy (make, value);
	  if (!strcmp (name,"CAMMODEL"))
	    strcpy (model, value);
	}
    }
next:
    fseek (ifp, save, SEEK_SET);
  }
}

void foveon_tree (unsigned huff[1024], unsigned code)
{
  struct decode *cur;
  int i, len;

  cur = free_decode++;
  if (code) {
    for (i=0; i < 1024; i++)
      if (huff[i] == code) {
	cur->leaf = i;
	return;
      }
  }
  if ((len = code >> 27) > 26) return;
  code = (len+1) << 27 | (code & 0x3ffffff) << 1;

  cur->branch[0] = free_decode;
  foveon_tree (huff, code);
  cur->branch[1] = free_decode;
  foveon_tree (huff, code+1);
}

void foveon_decode (FILE *tfp)
{
  int bwide, row, col, bit=-1, c, i;
  char *buf;
  struct decode *dindex;
  short pred[3];
  unsigned huff[1024], bitbuf=0;

  fseek (ifp, thumb_offset+16, SEEK_SET);
  width  = get4();
  height = get4();
  bwide  = get4();
  fprintf (tfp, "P6\n%d %d\n255\n", width, height);
  if (bwide > 0) {
    buf = malloc(bwide);
    for (row=0; row < height; row++) {
      fread  (buf, 1, bwide, ifp);
      fwrite (buf, 3, width, tfp);
    }
    free (buf);
    return;
  }
  for (i=0; i < 256; i++)
    huff[i] = get4();
  memset (first_decode, 0, sizeof first_decode);
  free_decode = first_decode;
  foveon_tree (huff, 0);

  for (row=0; row < height; row++) {
    memset (pred, 0, sizeof pred);
    if (!bit) get4();
    for (col=bit=0; col < width; col++) {
      for (c=0; c < 3; c++) {
	for (dindex=first_decode; dindex->branch[0]; ) {
	  if ((bit = (bit-1) & 31) == 31)
	    for (i=0; i < 4; i++)
	      bitbuf = (bitbuf << 8) + fgetc(ifp);
	  dindex = dindex->branch[bitbuf >> bit & 1];
	}
	pred[c] += dindex->leaf;
	fputc (pred[c], tfp);
      }
    }
  }
}

void kodak_yuv_decode (FILE *tfp)
{
  uchar c, blen[384];
  unsigned row, col, len, bits=0;
  INT64 bitbuf=0;
  int i, li=0, si, diff, six[6], y[4], cb=0, cr=0, rgb[3];
  ushort *out, *op;

  fseek (ifp, thumb_offset, SEEK_SET);
  width = (width+1) & -2;
  height = (height+1) & -2;
  fprintf (tfp, "P6\n%d %d\n65535\n", width, height);
  out = malloc (width * 12);
  if (!out) {
    fprintf (stderr, "kodak_yuv_decode() malloc failed!\n");
    exit(1);
  }

  for (row=0; row < height; row+=2) {
    for (col=0; col < width; col+=2) {
      if ((col & 127) == 0) {
	len = (width - col + 1) * 3 & -4;
	if (len > 384) len = 384;
	for (i=0; i < len; ) {
	  c = fgetc(ifp);
	  blen[i++] = c & 15;
	  blen[i++] = c >> 4;
	}
	li = bitbuf = bits = y[1] = y[3] = cb = cr = 0;
	if (len % 8 == 4) {
	  bitbuf  = fgetc(ifp) << 8;
	  bitbuf += fgetc(ifp);
	  bits = 16;
	}
      }
      for (si=0; si < 6; si++) {
	len = blen[li++];
	if (bits < len) {
	  for (i=0; i < 32; i+=8)
	    bitbuf += (INT64) fgetc(ifp) << (bits+(i^8));
	  bits += 32;
	}
	diff = bitbuf & (0xffff >> (16-len));
	bitbuf >>= len;
	bits -= len;
	if ((diff & (1 << (len-1))) == 0)
	  diff -= (1 << len) - 1;
	six[si] = diff;
      }
      y[0] = six[0] + y[1];
      y[1] = six[1] + y[0];
      y[2] = six[2] + y[3];
      y[3] = six[3] + y[2];
      cb  += six[4];
      cr  += six[5];
      for (i=0; i < 4; i++) {
	op = out + ((i >> 1)*width + col+(i & 1)) * 3;
	rgb[0] = y[i] + 1.40200/2 * cr;
	rgb[1] = y[i] - 0.34414/2 * cb - 0.71414/2 * cr;
	rgb[2] = y[i] + 1.77200/2 * cb;
	for (c=0; c < 3; c++)
	  if (rgb[c] > 0) op[c] = htons(rgb[c]);
      }
    }
    fwrite (out, sizeof *out, width*6, tfp);
  }
  free(out);
}

void parse_phase_one (int base)
{
  unsigned entries, tag, type, len, data, save;
  char str[256];

  fseek (ifp, base + 8, SEEK_SET);
  fseek (ifp, base + get4(), SEEK_SET);
  entries = get4();
  get4();
  while (entries--) {
    tag  = get4();
    type = get4();
    len  = get4();
    data = get4();
    save = ftell(ifp);
    printf ("Phase One tag=0x%x, type=%d, len=%2d, data = 0x%x\n",
		tag, type, len, data);
    if (type == 1 && len < 256) {
      fseek (ifp, base + data, SEEK_SET);
      fread (str, 256, 1, ifp);
      puts (str);
    }
    if (tag == 0x110) {
      thumb_offset = data + base;
      thumb_length = len;
    }
    fseek (ifp, save, SEEK_SET);
  }
  strcpy (make, "Phase One");
  strcpy (model, "unknown");
}

void parse_jpeg (int offset)
{
  int len, save, hlen;

  fseek (ifp, offset, SEEK_SET);
  if (fgetc(ifp) != 0xff || fgetc(ifp) != 0xd8) return;

  while (fgetc(ifp) == 0xff && fgetc(ifp) >> 4 != 0xd) {
    order = 0x4d4d;
    len   = get2() - 2;
    save  = ftell(ifp);
    order = get2();
    hlen  = get4();
    if (get4() == 0x48454150)		/* "HEAP" */
      parse_ciff (save+hlen, len-hlen, 0);
    parse_tiff (save+6);
    fseek (ifp, save+len, SEEK_SET);
  }
}

char *memmem (char *haystack, size_t haystacklen,
              char *needle, size_t needlelen)
{
  char *c;
  for (c = haystack; c <= haystack + haystacklen - needlelen; c++)
    if (!memcmp (c, needle, needlelen))
      return c;
  return NULL;
}

/*
   Identify which camera created this file, and set global variables
   accordingly.	 
   Return nonzero if the file cannot be decoded or no thumbnail is found
 */
int identify(FILE* tfp)
{
  char head[32], *thumb, *rgb, *cp;
  unsigned hlen, fsize, toff, tlen, lsize, i;

  make[0] = model[0] = model2[0] = is_dng = 0;
  thumb_head[0] = thumb_offset = thumb_length = thumb_layers = 0;
  order = get2();
  hlen = get4();
  fseek (ifp, 0, SEEK_SET);
  fread (head, 1, 32, ifp);
  fseek (ifp, 0, SEEK_END);
  fsize = ftell(ifp);
  if ((cp = memmem (head, 32, "MMMMRawT", 8)) ||
      (cp = memmem (head, 32, "IIIITwaR", 8)))
    parse_phase_one (cp - head);
  else if (order == 0x4949 || order == 0x4d4d) {
    if (!memcmp(head+6,"HEAPCCDR",8)) {
      parse_ciff (hlen, fsize - hlen, 0);
      fseek (ifp, hlen, SEEK_SET);
    } else
      parse_tiff (0);
  } else if (!memcmp (head, "\0MRM", 4))
    parse_minolta();
    else if (!memcmp (head, "\xff\xd8\xff\xe1", 4) &&
	     !memcmp (head+6, "Exif", 4)) {
    parse_tiff (12);
    thumb_length = 0;
  } else if (!memcmp (head, "FUJIFILM", 8)) {
    fseek (ifp, 84, SEEK_SET);
    toff = get4();
    tlen = get4();
    thumb_offset = toff;
    thumb_length = tlen;
  } else if (!memcmp (head, "DSC-Image", 9))
    parse_rollei();
  else if (!memcmp (head, "FOVb", 4))
    parse_foveon();
  fseek (ifp, 8, SEEK_SET);
  parse_mos(0);
  fseek (ifp, 3472, SEEK_SET);
  parse_mos(0);
  parse_jpeg(0);

  if (!thumb_length) {
    fprintf (stderr, "Thumbnail image not found\n");
    return -1;
  }

  if (is_dng) goto dng_skip;
  if (!strncmp(model,"DCS Pro",7)) {
    kodak_yuv_decode (tfp);
    goto done;
  }
  if (!strcmp(make,"Rollei")) {
    rollei_decode (tfp);
    goto done;
  }
  if (!strcmp(make,"SIGMA")) {
    foveon_decode (tfp);
    goto done;
  }
dng_skip:
  thumb = (char *) malloc(thumb_length);
  if (!thumb) {
    fprintf (stderr, "Cannot allocate %d bytes!!\n", thumb_length);
    exit(1);
  }
  fseek (ifp, thumb_offset, SEEK_SET);
  fread (thumb, 1, thumb_length, ifp);
  if (thumb_layers && !is_dng) {
    rgb = (char *) malloc(thumb_length);
    if (!rgb) {
      fprintf (stderr, "Cannot allocate %d bytes!!\n", thumb_length);
      return -1;
    }
    lsize = thumb_length/3;
    for (i=0; i < thumb_length; i++)
      rgb[(i%lsize)*3 + i/lsize] = thumb[i];
    free(thumb);
    thumb = rgb;
  }
  fputs (thumb_head, tfp);
  fwrite(thumb, 1, thumb_length, tfp);
  free (thumb);
done:
  fprintf (stderr, "Thumbnail image written\n");
  return 0;
}

int extract_thumbnail( FILE* input, FILE* output, int* orientation )
{
  /* Coffin's code has different meaning for orientation
	 values than TIFF, so we map them to TIFF values */
  static const int flip_map[] = { 0,1,3,2,4,7,5,6 };
  int rc;
  ifp = input;
  rc = identify(output);
  switch ((flip+3600) % 360) {
  case 270:  flip = 5;  break;
  case 180:  flip = 3;  break;
  case  90:  flip = 6;
  }
  if( orientation ) *orientation = flip_map[flip%7];
  return rc;
}
