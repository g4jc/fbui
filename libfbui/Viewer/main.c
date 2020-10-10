
/*=========================================================================
 *
 * fbview, an image viewer for FBUI (in-kernel framebuffer UI)
 * Copyright (C) 2004-2005 Zachary Smith, fbui@comcast.net
 *
 * This module is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This module is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this module; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * (See the file COPYING in the main directory of this archive for
 * more details.)
 *
 *=======================================================================*/

/*------------------------------------------------------------------------
 * Changes
 *
 * Sep 26, 2004: updated for grayscale images.
 * Dec 18, 2004: updated for TIFFs.
 * Jan 11, 2005: fixed memory allocation bugs.
 * Jan 19, 2005: added feature to let user delete images.
 * Jul 25, 2005: moved image-manipulation to libfbuiimage.
 * Jul 25, 2005: moved dialog-draw routine to libfbui.
 * Jul 25, 2005: ensured that image rotate is working.
 * Sep 07, 2005: added icon.
 * Sep 23, 2005: updated for FBUI_*CHANGED flag.
 *----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "libfbui.h"
#include "libfbuifont.h"
#include "libfbuiimage.h"
#include "libfbuidialog.h"


typedef struct item {
	struct item *next;
	char *str;
} Item;


/* Globals */

static Item *file_list = NULL;

static short max_width, max_height;


/* XPM */
static char * icon_xpm[] = {
"24 32 469 2",
"  	c #06FF00",
". 	c #07FF02",
"+ 	c #0CFF06",
"@ 	c #10FF0A",
"# 	c #14FF0F",
"$ 	c #18FF13",
"% 	c #1CFF17",
"& 	c #21FF1B",
"* 	c #25FF20",
"= 	c #29FF24",
"- 	c #2DFF29",
"; 	c #31FF2D",
"> 	c #35FF30",
", 	c #39FF34",
"' 	c #3EFF39",
") 	c #42FF3D",
"! 	c #46FF42",
"~ 	c #4BFF46",
"{ 	c #4EFF4A",
"] 	c #53FF4F",
"^ 	c #57FF53",
"/ 	c #5BFF57",
"( 	c #09FF04",
"_ 	c #0DFF08",
": 	c #12FF0C",
"< 	c #16FF10",
"[ 	c #1AFF14",
"} 	c #1EFF18",
"| 	c #22FF1D",
"1 	c #26FF21",
"2 	c #2BFF26",
"3 	c #2FFF2A",
"4 	c #33FF2F",
"5 	c #37FF32",
"6 	c #3BFF37",
"7 	c #40FF3B",
"8 	c #44FF3F",
"9 	c #48FF43",
"0 	c #4CFF48",
"a 	c #50FF4C",
"b 	c #54FF50",
"c 	c #59FF54",
"d 	c #5DFF59",
"e 	c #61FF5D",
"f 	c #07FF01",
"g 	c #0BFF05",
"h 	c #0FFF09",
"i 	c #14FF0E",
"j 	c #17FF12",
"k 	c #1BFF16",
"l 	c #1FFF1A",
"m 	c #24FF1E",
"n 	c #28FF23",
"o 	c #2CFF27",
"p 	c #31FF2C",
"q 	c #34FF30",
"r 	c #3DFF38",
"s 	c #41FF3C",
"t 	c #45FF41",
"u 	c #4AFF45",
"v 	c #4EFF49",
"w 	c #52FF4E",
"x 	c #56FF52",
"y 	c #5AFF57",
"z 	c #5FFF5A",
"A 	c #62FF5E",
"B 	c #67FF63",
"C 	c #0DFF07",
"D 	c #11FF0B",
"E 	c #15FF10",
"F 	c #19FF13",
"G 	c #1DFF17",
"H 	c #22FF1C",
"I 	c #29FF25",
"J 	c #2EFF29",
"K 	c #32FF2D",
"L 	c #36FF32",
"M 	c #3BFF35",
"N 	c #3FFF3A",
"O 	c #43FF3E",
"P 	c #47FF42",
"Q 	c #4CFF47",
"R 	c #4FFF4B",
"S 	c #58FF54",
"T 	c #5CFF58",
"U 	c #5FFF5C",
"V 	c #64FF61",
"W 	c #68FF64",
"X 	c #6CFF69",
"Y 	c #13FF0D",
"Z 	c #16FF11",
"` 	c #1BFF15",
" .	c #23FF1E",
"..	c #27FF22",
"+.	c #30FF2A",
"@.	c #34FF2F",
"#.	c #38FF33",
"$.	c #3CFF37",
"%.	c #45FF40",
"&.	c #49FF45",
"*.	c #51FF4D",
"=.	c #55FF51",
"-.	c #59FF56",
";.	c #5EFF59",
">.	c #66FF62",
",.	c #6AFF67",
"'.	c #6EFF6B",
").	c #72FF6F",
"!.	c #20FF1C",
"~.	c #2EFF28",
"{.	c #35FF31",
"].	c #3AFF34",
"^.	c #42FF3E",
"/.	c #53FF4E",
"(.	c #5FFF5B",
"_.	c #63FF60",
":.	c #67FF64",
"<.	c #6CFF68",
"[.	c #70FF6D",
"}.	c #74FF70",
"|.	c #78FF75",
"1.	c #1EFF19",
"2.	c #2AFF25",
"3.	c #2FFF29",
"4.	c #33FF2E",
"5.	c #37FF33",
"6.	c #3FFF3B",
"7.	c #65FF62",
"8.	c #6AFF66",
"9.	c #6DFF6A",
"0.	c #71FF6E",
"a.	c #76FF72",
"b.	c #7AFF77",
"c.	c #7EFF7B",
"d.	c #25FF1F",
"e.	c #2DFF28",
"f.	c #30FF2C",
"g.	c #22861F",
"h.	c #207C1E",
"i.	c #44DB40",
"j.	c #5AFF56",
"k.	c #2B9128",
"l.	c #278725",
"m.	c #7BFF78",
"n.	c #80FF7D",
"o.	c #84FF81",
"p.	c #39F935",
"q.	c #2AA827",
"r.	c #3ECE3A",
"s.	c #51F84D",
"t.	c #57FE54",
"u.	c #5BFF58",
"v.	c #60FF5C",
"w.	c #68FF65",
"x.	c #5CE359",
"y.	c #238021",
"z.	c #60DA5D",
"A.	c #7BFB77",
"B.	c #82FF7E",
"C.	c #86FF82",
"D.	c #8AFF87",
"E.	c #3DFF37",
"F.	c #40FF3C",
"G.	c #36CC33",
"H.	c #45D842",
"I.	c #5AFF55",
"J.	c #5EFF5A",
"K.	c #43B340",
"L.	c #54C752",
"M.	c #7FFF7C",
"N.	c #82FF80",
"O.	c #87FF84",
"P.	c #8BFF89",
"Q.	c #8FFF8D",
"R.	c #3AFF35",
"S.	c #3FDC3B",
"T.	c #44CB40",
"U.	c #6FFF6D",
"V.	c #6BF269",
"W.	c #228020",
"X.	c #339831",
"Y.	c #7FFD7C",
"Z.	c #89FF86",
"`.	c #8DFF8B",
" +	c #91FF8F",
".+	c #96FF93",
"++	c #43FF3F",
"@+	c #49ED46",
"#+	c #40BD3D",
"$+	c #65FF61",
"%+	c #69FF65",
"&+	c #75FF72",
"*+	c #46B343",
"=+	c #207D1E",
"-+	c #6EE46B",
";+	c #86FF83",
">+	c #8FFF8C",
",+	c #93FF90",
"'+	c #97FF95",
")+	c #9BFF98",
"!+	c #51FF4E",
"~+	c #55FC51",
"{+	c #3CB03A",
"]+	c #6FFF6B",
"^+	c #73FF70",
"/+	c #77FF74",
"(+	c #71EF6D",
"_+	c #227F20",
":+	c #45AD43",
"<+	c #88FF85",
"[+	c #8CFF89",
"}+	c #90FF8D",
"|+	c #94FF92",
"1+	c #99FF97",
"2+	c #9DFF9B",
"3+	c #A1FF9F",
"4+	c #47FF43",
"5+	c #4BFF47",
"6+	c #50FF4B",
"7+	c #58FF53",
"8+	c #278A25",
"9+	c #37A334",
"0+	c #71FF6D",
"a+	c #74FF72",
"b+	c #79FF76",
"c+	c #7DFF7A",
"d+	c #46AF43",
"e+	c #248222",
"f+	c #7FF27C",
"g+	c #8EFF8B",
"h+	c #92FF8F",
"i+	c #9AFF98",
"j+	c #9FFF9C",
"k+	c #A2FFA0",
"l+	c #A7FFA4",
"m+	c #4CFF49",
"n+	c #59FF55",
"o+	c #2F992D",
"p+	c #30962E",
"q+	c #77FF73",
"r+	c #7FFF7B",
"s+	c #74EB71",
"t+	c #217E1F",
"u+	c #57BF54",
"v+	c #98FF96",
"w+	c #9CFF9A",
"x+	c #A0FF9E",
"y+	c #A5FFA2",
"z+	c #A8FFA6",
"A+	c #ACFFAB",
"B+	c #39A837",
"C+	c #288826",
"D+	c #7DFF79",
"E+	c #81FF7D",
"F+	c #85FF82",
"G+	c #43A841",
"H+	c #2C8A29",
"I+	c #8DFA8A",
"J+	c #95FF93",
"K+	c #9EFF9B",
"L+	c #A6FFA4",
"M+	c #AAFFA8",
"N+	c #AFFFAD",
"O+	c #B3FFB1",
"P+	c #60FF5D",
"Q+	c #66FF61",
"R+	c #6EFF69",
"S+	c #44B641",
"T+	c #80FC7D",
"U+	c #75E472",
"V+	c #68CF66",
"W+	c #9FFF9D",
"X+	c #A4FFA1",
"Y+	c #A7FFA6",
"Z+	c #ACFFAA",
"`+	c #B0FFAE",
" @	c #B5FFB2",
".@	c #B8FFB7",
"+@	c #6BFF67",
"@@	c #73FF6F",
"#@	c #50C44D",
"$@	c #7BEF78",
"%@	c #8DFE8A",
"&@	c #3D9D3B",
"*@	c #369534",
"=@	c #98FE95",
"-@	c #9DFF9A",
";@	c #A5FFA3",
">@	c #AEFFAB",
",@	c #B1FFB0",
"'@	c #B6FFB4",
")@	c #BAFFB8",
"!@	c #BEFFBC",
"~@	c #65FF60",
"{@	c #6DFF69",
"]@	c #75FF71",
"^@	c #5ED25B",
"/@	c #74E071",
"(@	c #72DA6F",
"_@	c #7BDD78",
":@	c #9EFF9D",
"<@	c #A2FFA1",
"[@	c #A7FFA5",
"}@	c #ABFFA9",
"|@	c #B7FFB6",
"1@	c #BBFFBA",
"2@	c #C0FFBE",
"3@	c #C4FFC3",
"4@	c #6EFF6A",
"5@	c #6CE069",
"6@	c #6AD168",
"7@	c #359333",
"8@	c #44A342",
"9@	c #A8FFA7",
"0@	c #B1FFAF",
"a@	c #B4FFB3",
"b@	c #B9FFB7",
"c@	c #BDFFBC",
"d@	c #C1FFC0",
"e@	c #C6FFC5",
"f@	c #CAFFC8",
"g@	c #74FF71",
"h@	c #7CFF79",
"i@	c #7BEE79",
"j@	c #8DE98B",
"k@	c #ABFFA8",
"l@	c #AEFFAC",
"m@	c #B3FFB0",
"n@	c #B6FFB5",
"o@	c #BBFFB9",
"p@	c #BFFFBD",
"q@	c #C3FFC2",
"r@	c #C8FFC6",
"s@	c #CBFFCA",
"t@	c #D0FFCE",
"u@	c #75FF73",
"v@	c #7AFF76",
"w@	c #86FF84",
"x@	c #8BFF88",
"y@	c #8BFB89",
"z@	c #55B154",
"A@	c #B4FFB2",
"B@	c #B8FFB6",
"C@	c #BCFFBB",
"D@	c #C1FFBF",
"E@	c #C5FFC3",
"F@	c #C9FFC7",
"G@	c #CDFFCC",
"H@	c #D1FFD0",
"I@	c #D6FFD4",
"J@	c #88FF86",
"K@	c #8CFF8A",
"L@	c #2A8728",
"M@	c #268224",
"N@	c #9EF39B",
"O@	c #B2FFB0",
"P@	c #C2FFC1",
"Q@	c #C7FFC5",
"R@	c #CFFFCD",
"S@	c #D3FFD2",
"T@	c #D7FFD6",
"U@	c #DBFFDB",
"V@	c #81FF7E",
"W@	c #8DFF8C",
"X@	c #92FF90",
"Y@	c #389536",
"Z@	c #68C066",
"`@	c #C0FFBF",
" #	c #C4FFC2",
".#	c #C8FFC7",
"+#	c #D5FFD4",
"@#	c #D9FFD8",
"##	c #DDFFDC",
"$#	c #E1FFE0",
"%#	c #94FF91",
"&#	c #98FF95",
"*#	c #9CFF99",
"=#	c #A4FFA3",
"-#	c #A9FFA6",
";#	c #ADFFAB",
">#	c #B5FFB3",
",#	c #BAFFB7",
"'#	c #C6FFC4",
")#	c #CAFFC9",
"!#	c #CEFFCD",
"~#	c #D2FFD2",
"{#	c #D6FFD5",
"]#	c #DBFFDA",
"^#	c #DFFFDE",
"/#	c #E3FFE2",
"(#	c #E7FFE6",
"_#	c #8DFF8A",
":#	c #91FF8E",
"<#	c #95FF92",
"[#	c #A1FFA0",
"}#	c #D0FFCF",
"|#	c #D4FFD3",
"1#	c #D8FFD7",
"2#	c #DCFFDB",
"3#	c #E4FFE4",
"4#	c #E9FFE8",
"5#	c #EDFFED",
"6#	c #93FF91",
"7#	c #9BFF99",
"8#	c #A0FF9D",
"9#	c #C9FFC8",
"0#	c #CEFFCC",
"a#	c #DAFFD9",
"b#	c #DEFFDD",
"c#	c #E2FFE1",
"d#	c #E6FFE6",
"e#	c #EAFFEA",
"f#	c #EEFFEE",
"g#	c #F3FFF3",
"h#	c #99FF96",
"i#	c #A6FFA3",
"j#	c #AAFFA7",
"k#	c #ADFFAC",
"l#	c #BEFFBD",
"m#	c #C3FFC1",
"n#	c #CBFFC9",
"o#	c #CFFFCE",
"p#	c #E3FFE3",
"q#	c #E8FFE7",
"r#	c #ECFFEB",
"s#	c #F0FFEF",
"t#	c #F4FFF4",
"u#	c #F8FFF8",
"v#	c #A3FFA1",
"w#	c #A6FFA5",
"x#	c #BCFFBA",
"y#	c #CCFFCB",
"z#	c #D5FFD3",
"A#	c #E1FFE1",
"B#	c #E5FFE5",
"C#	c #EAFFE9",
"D#	c #F2FFF2",
"E#	c #F6FFF5",
"F#	c #FAFFFB",
"G#	c #FFFFFE",
"H#	c #A4FFA2",
"I#	c #A9FFA7",
"J#	c #B5FFB4",
"K#	c #B9FFB8",
"L#	c #C2FFC0",
"M#	c #D2FFD1",
"N#	c #DEFFDE",
"O#	c #EBFFEB",
"P#	c #EFFFEF",
"Q#	c #F7FFF8",
"R#	c #FCFFFC",
"S#	c #FFFFFF",
"T#	c #B7FFB5",
"U#	c #C7FFC6",
"V#	c #CCFFCA",
"W#	c #E0FFE0",
"X#	c #E5FFE4",
"Y#	c #F1FFF0",
"Z#	c #F5FFF5",
"`#	c #FAFFFA",
" $	c #FEFFFD",
".$	c #B1FFAE",
"+$	c #C4FFC4",
"@$	c #D2FFD0",
"#$	c #DDFFDD",
"$$	c #E6FFE5",
"%$	c #F3FFF2",
"&$	c #F6FFF7",
"*$	c #FBFFFB",
"      . + @ # $ % & * = - ; > , ' ) ! ~ { ] ^ / ",
"    ( _ : < [ } | 1 2 3 4 5 6 7 8 9 0 a b c d e ",
"f g h i j k l m n o p q , r s t u v w x y z A B ",
"C D E F G H * I J K L M N O P Q R ] S T U V W X ",
"Y Z ` l  ...2 +.@.#.$.7 %.&.0 *.=.-.;.A >.,.'.).",
"F % !.* = ~.K {.].' ^.! ~ { /.^ / (._.:.<.[.}.|.",
"1.| 1 2.3.4.5.6 6.8 9 0 a b c d e 7.8.9.0.a.b.c.",
"d.n e.f.g.h.h.h.h.h.h.i.x j.z k.h.h.h.h.l.m.n.o.",
"I J K L p.q.h.h.h.r.s.t.u.v.V w.x.h.y.z.A.B.C.D.",
"3 @.#.E.F.G.h.h.h.H.I.J.A >.8.'.K.h.L.M.N.O.P.Q.",
"{.R.' ) ! S.h.h.h.T.U _.W <.U.V.W.X.Y.o.Z.`. +.+",
"6 6.++9 0 @+h.h.h.#+$+%+9.0.&+*+=+-+;+D.>+,+'+)+",
"s t &.{ !+~+=+h.h.{+,.]+^+/+(+_+:+<+[+}+|+1+2+3+",
"4+5+6+b 7+T 8+h.h.9+0+a+b+c+d+e+f+g+h+.+i+j+k+l+",
"m+*.=.n+J.A o+h.h.p+q+b.r+s+t+u+}+|+v+w+x+y+z+A+",
"] ^ / (._.W B+h.h.C+D+E+F+G+H+I+J+1+K+k+L+M+N+O+",
"S d P+Q+%+R+S+h.h.h.T+O.U+h.V+'+)+W+X+Y+Z+`+ @.@",
"z A B +@]+@@#@h.h.h.$@%@&@*@=@-@x+;@M+>@,@'@)@!@",
"~@w.{@0.]@b+^@h.h.h./@(@h._@:@<@[@}@N+O+|@1@2@3@",
",.4@@@q+m.M.5@h.h.h.6@7@8@x+y+9@Z+0@a@b@c@d@e@f@",
"[.g@|.h@n.F+i@h.h.h.7@t+j@L+k@l@m@n@o@p@q@r@s@t@",
"u@v@c.N.w@x@y@h.h.h.h.z@z+}@`+A@B@C@D@E@F@G@H@I@",
"h@n.o.J@K@}+|+L@h.h.M@N@>@O@'@)@!@P@Q@s@R@S@T@U@",
"V@C.D.W@X@.+i+Y@h.h.Z@N+A@B@1@`@ #.#G@H@+#@###$#",
"<+P.}+%#&#*#x+=#-#;#0@>#,#c@d@'#)#!#~#{#]#^#/#(#",
"_#:#<#1+K+[#L+M+l@O+n@o@p@ #r@s@}#|#1#2#$#3#4#5#",
"6#'+7#8#X+z+Z+`+A@.@C@D@3@9#0#H@+#a#b#c#d#e#f#g#",
"h#-@3+i#j#k#O@'@)@l#m#e@n#o#S@T@2#^#p#q#r#s#t#u#",
":@v#w#}@N+O+B@x#2@3@.#y#H@z#@###A#B#C#5#D#E#F#G#",
"H#I#Z+0@J#K#!@L#'#f@!#M#T@a#N#/#(#O#P#g#Q#R#S#S#",
"M+l@O+T#1@2@q@U#V#}#|#1#2#W#X#4#5#Y#Z#`# $S#S#S#",
".$a@b@C@D@+$9#G@@${#a##$c#$$e#f#%$&$*$S#S#S#S#S#"};

void append_item (char *str)
{
	Item *nu = (Item*)malloc(sizeof(Item));
	memset ((void*)nu,0,sizeof(Item));
	nu->str = strdup (str);
	
	Item *last=NULL;
	last=file_list;
	while(last) {
		if (last->next)
			last = last->next;
		else
			break;
	}
	if (!last)
		file_list = nu;
	else
		last->next = nu;
}

char *nth_item (int n)
{
	Item *item= file_list;

	while (n--) {
		if (item)
			item = item->next;
		if (!item)
			break;
	}

	return item? item->str : NULL;
}


Font *pcf;
int target_w, target_h;
int available_height, text_height;


Image *image;
Image *shrunken_image;

static int grayscale=0;

static int fileNum = 0;
static char *path = NULL;

static short win_w, win_h;
static char do_shrink=0;


void redraw (Display *dpy, Window *win,
	     char use_clip, short x0,short y0,short x1,short y1)
{
	/* If we reach this point, we are doing an exposure.
	 */
	Image *im = shrunken_image ? shrunken_image : image;

	// Draw text
	char expr [100];
	char *tmp = path;
	char *tmp2;
	while ((tmp2 = strchr(tmp, '/'))) {
		tmp = tmp2 + 1;
	}
	sprintf (expr, "%s (%d x %d, depth %d)", tmp, 
		im->width, im->height, 
		im->bpp);

	short w,a,d;
	short y = im->height + 5;
	Font_string_dims (pcf, expr, &w,&a,&d);
	fbui_clear_area (dpy, win, 0, y, w, y + text_height);
	fbui_draw_string (dpy, win, pcf, 0, y, expr, RGB_WHITE);

	if (!use_clip)
		Image_draw (im, dpy, win, 0, 0);
	else {
		Image_draw_partial (im, dpy, win, 0, 0,
			x0,y0,x1,y1);
	}
}

void
rescale()
{
	if (!image) {
		FATAL("no primary image");
	}

	do_shrink= false;
	short target_w = image->width;
	short target_h = image->height > available_height ? available_height : image->height;

	if (win_w < image->width || available_height < image->height) 
	{
		do_shrink = true;

		double ratio = 0;
		if (win_w < image->width) {
			ratio = win_w / (double) image->width;
		}
		if (available_height < image->height) {
			double r = available_height / (double) image->height;
			if (ratio == 0 || r < ratio)
				ratio = r;
		}
		target_w = image->width * ratio;
		target_h = image->height * ratio;
	}

	if (do_shrink) {
		if (shrunken_image)
			Image_delete (shrunken_image);

		shrunken_image = Image_resize (image, target_w, target_h);
		do_shrink = false;
	}
}

int
readnext (Display *dpy, Window *win)
{
	path=NULL;
	int result=0;

	while (!result) {
		if (image) {
			Image_delete (image);
			image = NULL;
		}
		if (shrunken_image) {
			Image_delete (shrunken_image);
			shrunken_image = NULL;
		}

		path = nth_item (fileNum++);
		if (!path)
			break;

		char *extension = path;
		char *s = NULL;
		while ((s = strchr (extension, '.'))) {
			extension = s;
			if (strchr (s+1, '.')) {
				extension = s+1;
			} else 
				break;
		}
		if (extension == path) {
			fprintf (stderr, "fbview: invalid filename %s\n", path);
			continue;
		}

		if (!strcmp (extension, ".jpg") ||
		    !strcmp (extension, ".JPG") ||
		    !strcmp (extension, ".jpeg") ||
		    !strcmp (extension, ".JPEG"))
		{
			image = Image_read_jpeg (path);
			result = 0;
			if (image) {
				printf ("JPEG information: %s, width %d height %d depth %d\n",
					path,image->width,image->height,image->bpp);
				result=1;
			}

		}
		else
		if (!strcmp (extension, ".tif") ||
		    !strcmp (extension, ".TIF") ||
		    !strcmp (extension, ".tiff") ||
		    !strcmp (extension, ".TIFF")) 
		{
			image = Image_read_tiff (path);
			result = 0;
			if (image) {
				printf ("TIFF information: %s, width %d height %d depth %d components %u\n",
					path,image->width,image->height,image->bpp, image->components);
				result=1;
			}
		}
		else
		if (!strcmp (extension, ".nef") ||
		    !strcmp (extension, ".NEF")) 
		{
			fprintf (stderr, "fbview: not supporting raw images yet: %s\n", path);
			continue;
		}
		else
		{
			fprintf (stderr, "fbview: unsupported image extension in %s\n", path);
			continue;
		}
	}

	if (!path)
		return 0;

	if (result) {
		fbui_set_subtitle (dpy, win, path);
		rescale();
	}

	return result;
}


void printstatus (Display *dpy, Window *win, char *status)
{
	if (!dpy || !win || !status)
		return;

	fbui_draw_string (dpy, win, pcf, 10, 10, status, RGB_WHITE);
}



int
main(int argc, char** argv)
{
	int i;
	Display *dpy;
	Window *win;
	Font *dialog_font;
	char waiting_for_yn = 0;
	char yn_operation = 0;

	path=NULL;

	image = NULL;
	shrunken_image = NULL;

	if (argc==1)  {
		printf ("No images specified.\n");
		return 0;
	}

	dpy = fbui_display_open ();
	if (!dpy)
		FATAL ("cannot open display");

        pcf = Font_new ();
        dialog_font = Font_new ();

        if (!pcf_read (pcf, "timR12.pcf")) {
                Font_free (pcf);
		FATAL ("cannot load main font");
        }

        if (!pcf_read (dialog_font, "timR14.pcf")) {
                Font_free (dialog_font);
                Font_free (pcf);
		FATAL ("cannot load dialog box font");
        }

	text_height = pcf->ascent + pcf->descent;

/* XX
 * If only one image file, we should open
 * with dimensions of that file, plus room for text
 */
	long fg,bg;
	fg = RGB_NOCOLOR;
	bg = RGB_BLACK;

	/* get the maximal window */
	max_width=dpy->width;
	max_height=dpy->height;

	win = fbui_window_open (dpy, max_width, max_height,
		&win_w, &win_h,
		9999,9999, // max wid/ht
		0, 0, 
		&fg, &bg, 
		"fbview", "", 
		FBUI_PROGTYPE_APP, 
		false, // not requesting control
		false, // therefore not autoplacing anything
		-1,
		true, // need keys
		false, // not all motion
		false, // not hidden
		NULL, /* no mask */
		argc,argv);
	if (!win)
		FATAL ("cannot create window");

	if (!fbui_xpm_to_icon (dpy, win, icon_xpm)) {
		printf ("xpm to icon conversion failed\n");
	}

	/* Parse file list */
	i=1;
	while (i<argc) {
		char ch = argv[i] ? *argv[i] : 0;
		/* params used by FBUI lib will be truncated to zero length */
		if (ch && ch != '-')
			append_item (argv[i]);
	
		i++;
	}

	fileNum = 0;

	available_height = win_h - 5 - text_height;

	printstatus (dpy, win, "Loading...");

	int result = readnext (dpy,win);

	if (!result) {
		fbui_display_close (dpy);
		exit(0);
	}

	fbui_clear (dpy, win);
	fbui_flush (dpy, win);

	/* Event loop */
	char done=0;
	while(!done) {
		Event ev;
		int err;
		if ((err = fbui_wait_event (dpy, &ev, FBUI_EVENTMASK_ALL))) {
			fbui_print_error (err);
			continue;
		}
// printf ("%s got event %s\n", argv[0], fbui_get_event_name (ev.type));

		int num = ev.type;

		if (ev.win != win)
			FATAL ("got event for another window");

		switch (num) {
		case FBUI_EVENT_MOVERESIZE:
			if (win_w == ev.width && win_h && ev.height) {
				// No resize
				if (ev.key & FBUI_CONTENTS_CHANGED)
					break;
				continue;
			}
			win_w = ev.width;
			win_h = ev.height;
			available_height = win_h - 5 - text_height;
			if (image)
				rescale ();
			break;
		
		case FBUI_EVENT_MOTION: {
			short x, y;
			
			x = ev.x;
			y = ev.y;

			/* not used */
			continue;
		}
		
		case FBUI_EVENT_KEY: {
			short ch = fbui_convert_key (dpy, ev.key);

			ch = tolower(ch);

			switch (ch) {
			case 'y':
			case 'n':
				if (waiting_for_yn) {
					waiting_for_yn = false;

					switch (yn_operation) {
					case 1:
						if (ch == 'y') {
							printf ("Deleting file %s\n", path);
							unlink (path);
							goto next_image;
						}
						else
							continue;
					}
				}
				else
					continue;
			
			case 'r': {
				Image *nu = Image_rotate (image, 90);
				if (nu) {
					fbui_clear (dpy, win);
					Image_delete (image);
					image = nu;
					rescale ();
				}
				break;
			}
			 
			case 'q':
				done=true;
				continue;
			 
			case 'd':
				fbui_draw_dialog (dpy, win, dialog_font, "Delete file? (y/n)");
				waiting_for_yn = true;
				yn_operation = 1;
				continue;
			
			case 'g':
				waiting_for_yn = false;
				grayscale = !grayscale;
				break;
			
			case ' ':
next_image:
				waiting_for_yn = false;

				if (shrunken_image) {
					Image_delete (shrunken_image);
					shrunken_image = NULL;
				}

				fbui_clear (dpy, win);
				printstatus (dpy, win, "Loading...");

				int result= readnext (dpy,win);

				if (!result) {
					fbui_window_close (dpy, win);
					exit(0);
				}
				break;
			
			default:
				continue;
			} /* switch */
		 }
		
		case FBUI_EVENT_EXPOSE:
			if (ev.has_rects) {
				struct fbui_rects *r = &ev.rects;
				short min_x = 10000;
				short min_y = 10000;
				short max_x = -10000;
				short max_y = -10000;
				int ix = 0;
				int lim = r->total << 2;
				short x0,y0,x1,y1;
				while (ix < lim) {
					x0 = r->c[ix++];
					y0 = r->c[ix++];
					x1 = r->c[ix++];
					y1 = r->c[ix++];
					if (x0 < min_x)
						min_x = x0;
					if (y0 < min_y)
						min_y = y0;
					if (x1 > max_x)
						max_x = x1;
					if (y1 > max_y)
						max_y = y1;
					//redraw (true, x0,y0,x1,y1);
				}
				redraw (dpy,win,true, min_x, min_y, max_x, max_y);
				continue;
			}
			break;
			
		default:
			continue;
		}

		redraw (dpy,win,false, 0,0,0,0);

	} /* while */

	fbui_flush (dpy, win);
	fbui_window_close (dpy, win);
	fbui_display_close (dpy);
	
	return 0;
}
